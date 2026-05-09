import asyncio
import subprocess
import struct
import os
import json
import threading
from contextlib import asynccontextmanager
from fastapi import FastAPI, WebSocket
from fastapi.staticfiles import StaticFiles
from fastapi.middleware.cors import CORSMiddleware
import uvicorn

# ----------------------------------------------------------------
# Shared state
# ----------------------------------------------------------------
cpp_process = None
console_clients: set[WebSocket] = set()
telemetry_clients: set[WebSocket] = set()
log_buffer: list[str] = []
latest_frame: str = ""
buildings_data: list = []  # Cached building list from C++

# ----------------------------------------------------------------
# Thread: Read C++ stdout line by line (blocking I/O)
# ----------------------------------------------------------------
def _reader_thread():
    global cpp_process
    while True:
        proc = cpp_process
        if proc and proc.stdout:
            try:
                for line in iter(proc.stdout.readline, ""):
                    if line:
                        log_buffer.append(line)
                        if len(log_buffer) > 500:
                            log_buffer.pop(0)
                    # If a new process was started, stop reading from the old one
                    if cpp_process is not proc:
                        break
            except (ValueError, OSError):
                # Pipe was closed by kill_engine, this is expected
                pass
            print("[Reader] Finished reading from process pipe.")
        import time; time.sleep(0.2)

threading.Thread(target=_reader_thread, daemon=True).start()

# ----------------------------------------------------------------
# Thread: Connect to C++ TCP bridge (port 9998) and read drone positions
# ----------------------------------------------------------------
def _tcp_bridge_thread():
    """Reads binary drone positions and building data from C++ bridge."""
    global latest_frame, buildings_data
    import socket, time

    while True:
        sock = None
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect(("127.0.0.1", 9998))
            print("[Python Bridge] Connected to C++ visualizer on port 9998")

            while True:
                # Read header (uint32)
                header = _recv_exact(sock, 4)
                if header is None:
                    break
                marker = struct.unpack('<I', header)[0]

                # Check if this is building data (marker = 0xFFFFFFFF)
                if marker == 0xFFFFFFFF:
                    # Read building count
                    count_bytes = _recv_exact(sock, 4)
                    if count_bytes is None:
                        break
                    count = struct.unpack('<I', count_bytes)[0]

                    # Read building data: count * 5 floats (x, y, w, h, type)
                    body = _recv_exact(sock, count * 5 * 4)
                    if body is None:
                        break
                    floats = struct.unpack(f'<{count * 5}f', body)

                    bldgs = []
                    for i in range(count):
                        bldgs.append({
                            "x": round(floats[i*5], 1),
                            "y": round(floats[i*5+1], 1),
                            "w": round(floats[i*5+2], 1),
                            "h": round(floats[i*5+3], 1),
                            "type": int(floats[i*5+4])
                        })
                    buildings_data = bldgs
                    print(f"[Python Bridge] Received {count} buildings")
                    continue

                # Normal drone frame
                num_drones = marker
                if num_drones == 0 or num_drones > 50000:
                    continue

                body_size = num_drones * 2 * 4
                body = _recv_exact(sock, body_size)
                if body is None:
                    break

                floats = struct.unpack(f'<{num_drones * 2}f', body)
                positions = []
                for i in range(num_drones):
                    positions.append([round(floats[i*2], 1), round(floats[i*2+1], 1)])

                latest_frame = json.dumps({
                    "type": "frame",
                    "n": num_drones,
                    "pos": positions
                })

        except ConnectionRefusedError:
            pass  # Engine not running yet, retry silently
        except Exception as e:
            print(f"[Python Bridge] TCP error: {e}")
        finally:
            if sock:
                try: sock.close()
                except: pass
            latest_frame = ""
            time.sleep(1)  # Wait before reconnecting

def _recv_exact(sock, num_bytes):
    """Helper: receive exactly num_bytes from a socket."""
    data = b""
    while len(data) < num_bytes:
        chunk = sock.recv(num_bytes - len(data))
        if not chunk:
            return None
        data += chunk
    return data

threading.Thread(target=_tcp_bridge_thread, daemon=True).start()

# ----------------------------------------------------------------
# Async tasks: broadcast logs and telemetry to WebSocket clients
# ----------------------------------------------------------------
async def _broadcast_logs():
    sent_index = 0
    while True:
        # Detect buffer reset (log_buffer.clear() was called on new boot)
        if sent_index > len(log_buffer):
            sent_index = 0
        
        if sent_index < len(log_buffer):
            new_lines = log_buffer[sent_index:]
            sent_index = len(log_buffer)
            for line in new_lines:
                for client in list(console_clients):
                    try:
                        await client.send_text(line)
                    except Exception:
                        console_clients.discard(client)
        await asyncio.sleep(0.1)

async def _broadcast_telemetry():
    """Push latest drone positions to all telemetry WebSocket clients at ~30 FPS."""
    prev_frame = ""
    sent_buildings = False
    while True:
        # Send buildings once when they become available
        if buildings_data and not sent_buildings:
            bldg_msg = json.dumps({"type": "buildings", "data": buildings_data})
            for client in list(telemetry_clients):
                try:
                    await client.send_text(bldg_msg)
                except Exception:
                    telemetry_clients.discard(client)
            sent_buildings = True

        # Reset building flag when engine is killed
        if not buildings_data:
            sent_buildings = False

        frame = latest_frame
        if frame and frame != prev_frame:
            prev_frame = frame
            for client in list(telemetry_clients):
                try:
                    await client.send_text(frame)
                except Exception:
                    telemetry_clients.discard(client)
        await asyncio.sleep(0.033)  # ~30 FPS

@asynccontextmanager
async def lifespan(app):
    t1 = asyncio.create_task(_broadcast_logs())
    t2 = asyncio.create_task(_broadcast_telemetry())
    yield
    t1.cancel()
    t2.cancel()

app = FastAPI(lifespan=lifespan)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

# ----------------------------------------------------------------
# REST API - Engine lifecycle
# ----------------------------------------------------------------
@app.post("/api/engine/start")
async def start_engine():
    global cpp_process, buildings_data
    print(f"[API] Received start request. Current process: {cpp_process}")
    
    if cpp_process is not None:
        poll = cpp_process.poll()
        print(f"[API] Current process poll state: {poll}")
        if poll is None:
            return {"status": "already_running", "pid": cpp_process.pid}

    project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
    print(f"[API] Launching 'make run_server' in {project_root}")
    
    try:
        cpp_process = subprocess.Popen(
            ["make", "run_server"],
            cwd=project_root,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1
        )
        log_buffer.clear()
        buildings_data = [] 
        log_buffer.append("[SwarmOS] Engine process started.\n")
        print(f"[API] Process started with PID: {cpp_process.pid}")
        return {"status": "started", "pid": cpp_process.pid}
    except Exception as e:
        print(f"[API] Failed to start process: {e}")
        return {"status": "error", "message": str(e)}

@app.post("/api/engine/kill")
async def kill_engine():
    global cpp_process, buildings_data
    print("[API] Received kill request.")
    if cpp_process is not None:
        pid = cpp_process.pid
        if os.name == 'nt':
            print(f"[API] Killing process tree for PID {pid}")
            # Kill the process tree (make -> server.exe)
            subprocess.run(["taskkill", "/F", "/T", "/PID", str(pid)], capture_output=True)
            # Extra safety: clean up any lingering build/server processes
            subprocess.run(["taskkill", "/F", "/IM", "server.exe"], capture_output=True)
            subprocess.run(["taskkill", "/F", "/IM", "make.exe"], capture_output=True)
            subprocess.run(["taskkill", "/F", "/IM", "g++.exe"], capture_output=True)
        else:
            cpp_process.kill()
        
        # CRITICAL: Close the stdout pipe to unblock the reader thread
        try:
            cpp_process.stdout.close()
        except Exception:
            pass
        
        cpp_process = None
        buildings_data = [] 
        log_buffer.append("[SwarmOS] Engine process killed and state reset.\n")
        return {"status": "killed"}
    
    # Even if we don't have a record of the process, try to clean up orphaned ones
    if os.name == 'nt':
        subprocess.run(["taskkill", "/F", "/IM", "server.exe"], capture_output=True)
        
    return {"status": "not_running"}

@app.get("/api/engine/status")
async def engine_status():
    if cpp_process is not None and cpp_process.poll() is None:
        return {"running": True, "pid": cpp_process.pid}
    return {"running": False}

@app.get("/api/buildings")
async def get_buildings():
    return buildings_data

@app.get("/api/telemetry/sessions")
async def get_sessions():
    import os
    import sys
    
    log_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "telemetry", "logs"))
    telemetry_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "telemetry"))
    if telemetry_dir not in sys.path:
        sys.path.insert(0, telemetry_dir)
        
    try:
        from dashboard import get_available_sessions
        return get_available_sessions(log_dir)
    except ImportError as e:
        print(f"Error importing dashboard module: {e}")
        return []

@app.get("/api/telemetry/data")
async def get_telemetry_data(session: str):
    import os
    import sys
    
    log_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "telemetry", "logs"))
    file_path = os.path.join(log_dir, session, "Full_Telemetry.csv")
    telemetry_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "telemetry"))
    
    if telemetry_dir not in sys.path:
        sys.path.insert(0, telemetry_dir)
        
    try:
        from dashboard import load_full_data, process_telemetry_data
        
        df = load_full_data(file_path)
        if df is None:
            return {"error": "Invalid or missing data"}
            
        return process_telemetry_data(df)
    except Exception as e:        
        import traceback
        traceback.print_exc()
        return {"error": str(e)}

@app.get("/api/config")
async def get_config():
    """Parse global_config.h and return all constexpr values as JSON."""
    import re
    config = {}
    project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
    config_path = os.path.join(project_root, "include", "shared", "global_config.h")
    
    try:
        with open(config_path, "r", encoding="utf-8", errors="replace") as f:
            for line in f:
                line = line.strip()
                # Match: constexpr float/int/bool NAME = VALUE;
                m = re.match(r'constexpr\s+(\w+)\s+(\w+)\s*=\s*(.+?)\s*;', line)
                if m:
                    ctype, name, val = m.group(1), m.group(2), m.group(3)
                    try:
                        if ctype == "bool":
                            config[name] = val.strip().lower() == "true"
                        elif ctype == "int":
                            config[name] = int(val)
                        elif ctype == "float":
                            config[name] = float(val.replace("f", ""))
                        else:
                            config[name] = val
                    except ValueError:
                        config[name] = val
                
                # Match: constexpr Vector2 NAME = {X, Y};
                m2 = re.match(r'constexpr\s+Vector2\s+(\w+)\s*=\s*\{(.+?),\s*(.+?)\}', line)
                if m2:
                    name = m2.group(1)
                    try:
                        config[name] = [
                            float(m2.group(2).strip().replace("f", "")),
                            float(m2.group(3).strip().replace("f", ""))
                        ]
                    except ValueError:
                        pass
    except FileNotFoundError:
        return {"error": "global_config.h not found"}
    
    return config

# ----------------------------------------------------------------
# WebSocket - Console output (stdout)
# ----------------------------------------------------------------
@app.websocket("/ws/console")
async def websocket_console(websocket: WebSocket):
    await websocket.accept()
    for line in log_buffer:
        await websocket.send_text(line)
    console_clients.add(websocket)
    try:
        while True:
            await websocket.receive_text()
    except Exception:
        pass
    finally:
        console_clients.discard(websocket)

# ----------------------------------------------------------------
# WebSocket - Telemetry (drone positions for Canvas)
# ----------------------------------------------------------------
@app.websocket("/ws/telemetry")
async def websocket_telemetry(websocket: WebSocket):
    await websocket.accept()
    telemetry_clients.add(websocket)
    try:
        while True:
            await websocket.receive_text()
    except Exception:
        pass
    finally:
        telemetry_clients.discard(websocket)

# ----------------------------------------------------------------
# Static files
# ----------------------------------------------------------------
web_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
app.mount("/", StaticFiles(directory=web_dir, html=True), name="static")

if __name__ == "__main__":
    print("======================================================")
    print(" SWARM OS - ORCHESTRATOR BRIDGE STARTED ")
    print(" Open http://localhost:8000 in your browser ")
    print("======================================================")
    uvicorn.run(app, host="0.0.0.0", port=8000)
