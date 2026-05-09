# Simulated-Autonomous-Flight 🚀
An advanced multi-agent autonomous drone swarm simulator. Built with C++, SFML, and powered by **AMD ROCm/MI300X** for massive scale and LLM-driven strategic orchestration.

> [!IMPORTANT]
> **Hackathon Update**: This repository has evolved into a **Cloud-Native Hybrid Architecture**. We have successfully decoupled the massive GPU-accelerated physics engine (running on AMD MI300X Cloud) from the visualization client (running locally on Windows).

## 🌟 Current Architecture (Hybrid Cloud)
The system now operates on a cutting-edge decoupled architecture to support 10,000+ drones seamlessly:

1. **The Server (`app_server` / Linux AMD Cloud):**
   - **Headless Execution**: Runs entirely without graphical overhead. Tick rate is explicitly capped at 60 TPS to simulate real-time physics accurately.
   - **GPU Acceleration**: Utilizes `hipcc` to compile ROCm `.hip` kernels for massive parallel boids and collision computations on the AMD MI300X.
   - **TCP Broadcaster**: Streams optimized binary drone coordinate arrays over the network.

2. **The Client (`app_client` / Local Windows):**
   - **Dumb Terminal Viewer**: Does **not** compute physics or AI. It strictly receives the binary stream from the cloud server and renders the swarm smoothly using SFML 3.0 at 60 FPS.
   - **Dynamic Shaders**: Implements beautiful GLSL shaders (e.g., daytime FBM clouds) for premium aesthetics.

3. **The Brain (`brain.py` / Local or Cloud):**
   - **Agentic Orchestration**: Uses Google Gemini to command the swarm strategically via a secondary TCP port.

## 📂 Modular Structure
The codebase has been heavily refactored for enterprise-level maintainability:
- `src/core/`: Main entry points (`main_server.cpp`, `main_client.cpp`, `SimulationEngine.cpp`).
- `src/server/`: AI state machines, GPU Kernels (`.hip`), Swarm Logic, and Spawning orchestrators.
- `src/client/`: Graphical Renderer, Cameras, and SFML window management.
- `src/shared/`: Networking Bridges, Math Utilities, and Procedural City generation.
- `include/`: Mirrored structure for all header files.

## 🛠️ Universal Build System
We unified the build pipeline into a **Single Intelligent Makefile**:
- **Auto-OS Detection**: The `Makefile` detects if it's running on Windows (`ifeq ($(OS),Windows_NT)`) or Linux.
- **Smart Include Paths**: Automatically maps all modular directories (`-Iinclude/core`, `-Iinclude/server`, etc.) so developers don't need to write relative paths (`../../`).
- **Dynamic Linking**: Configured to resolve Linux PIE issues and SFML 3.0 shared library paths (`-rpath`).

---

## 🚀 Quick Start & Deployment

### 1. AMD Cloud Server (Linux)
Connect to your AMD Droplet via SSH. The Makefile will detect Linux and compile with `hipcc` and `-fPIC`.
```bash
git pull origin main
make run_server
```
*The server will start listening on port 9998 and wait for the client.*

### 2. Local Viewer (Windows)
Ensure `SERVER_IP` in `global_config.h` points to your AMD Droplet's public IP. The Makefile will detect Windows and link your local AMD ROCm paths.
```bash
make run_client
```
*The client will connect, and you will see the swarm emerge in real-time!*

### 3. The Brain (Optional)
To launch the LLM Orchestrator:
```bash
make run_brain
```

---

---

## 🛰️ Intelligence Features: Autonomous Swarm Highways
The system now features a robust traffic management system for complex drone swarms:

1. **Dynamic Perception (Radar & Vision)**: 
   - Uses real-time drone centroids and AABB (Bounding Boxes) instead of static matrix centers.
   - Separate X/Y axis detection for precise collision avoidance in high-altitude highways.

2. **Priority Negotiation (`ShouldIWait`)**:
   - Implements a communication-like protocol to resolve traffic deadlocks.
   - Active cruising matrices have priority over staging/takeoff swarms.
   - Unique ID-based tie-breaking ensures orderly flow.

3. **Autonomous "Step" Navigation**:
   - Matrices can "freeze" in place (Dynamic Freezing) upon detecting danger and resume their path towards the `current_target` once the space is clear.

4. **Multi-Lane Orchestration**:
   - Procedural lane assignment (Inbound/Outbound) based on mission objectives.
   - Dynamic vertical layering to optimize airspace usage.

## 📊 Fleet Intelligence Dashboard
A modern web-based monitoring system powered by **FastAPI** and **Plotly**:
- **Individual Agents Tracking**: Real-time telemetry for thousands of discrete drones.
- **Groupal Agents Management**: Monitoring and state tracking of matrix-level collective intelligence.
- **Kinematic Analysis**: Live charts for PID error, battery drain, and actuator response.

---

## 🗺️ Hackathon Roadmap
### Phase 1, 2 & 3 (DONE ✅)
- [x] Port physics to `.hip` kernels and AMD ROCm.
- [x] Decouple Headless Server and SFML Client.
- [x] High-speed TCP Binary streaming.
- [x] Gemini 1.5 Integration and AI Orchestration.

### Phase 4: Autonomous Highways (DONE ✅)
- [x] Implementation of Matrix-level Vision and Radar.
- [x] Dynamic Bounding Boxes based on real drone positions.
- [x] Priority-based Negotiation Protocol for traffic.
- [x] Telemetry refactor to Vector2 (X/Y) for precision.
- [x] Dashboard metrics for Individual vs Groupal agents.

### Phase 5: Extreme Scaling (UP NEXT 🚀)
- [ ] Push to 100,000 agents on AMD MI300X.
- [ ] Implement V2V (Vehicle-to-Vehicle) complex swarm signaling.
- [ ] Real-time procedural city expansion during flight.

