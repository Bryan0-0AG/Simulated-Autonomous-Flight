import socket
import json
import time
from strategist import DroneStrategist

def main():
    # 1. Inicializar el Cerebro (LLM)
    print("[INFO] Inicializando el Estratega LLM...")
    brain = DroneStrategist()

    # 2. Configurar el Servidor (El "Oido" del Orquestador)
    HOST = '127.0.0.1'  # Localhost
    PORT = 9999         # Puerto de comunicacion
    
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((HOST, PORT))
    server_socket.listen(1)
    
    print(f"[INFO] Orquestador escuchando en {HOST}:{PORT}...")
    print("[HINT] Esperando conexion del simulador C++...")

    try:
        while True:
            # Esperar a que el simulador se conecte
            conn, addr = server_socket.accept()
            with conn:
                print(f"[OK] Simulacion conectada desde {addr}")
                
                while True:
                    # Recibir datos de la simulacion
                    try:
                        data = conn.recv(4096).decode('utf-8')
                        if not data:
                            break
                        
                        # 3. Procesar datos recibidos
                        swarm_data = json.loads(data)
                        print(f"\n[DATOS] Recibidos datos de {swarm_data.get('total_drones')} drones.")

                        # 4. Consultar al Estratega (con manejo de errores de cuota)
                        print("[IA] Consultando estrategia tactica...")
                        try:
                            strategy = brain.generate_strategy(swarm_data)
                        except Exception as e:
                            print(f"[ERROR] Al consultar al LLM: {e}")
                            strategy = {
                                "global_order": "Mantener protocolos estándar de vuelo. Monitoreo de sensores activado.",
                                "analysis": "Error de cuota en API de IA. Usando protocolo de contingencia local."
                            }
                        
                        print(f"[ORDEN] {strategy.get('global_order')}")
                        print(f"[ANALISIS] {strategy.get('analysis')}")

                        # 5. Enviar respuesta de vuelta a C++
                        response = json.dumps(strategy) + "\n"
                        conn.sendall(response.encode('utf-8'))
                        
                    except json.JSONDecodeError:
                        # A veces los paquetes de red llegan fragmentados o pegados
                        continue
                    except Exception as e:
                        print(f"[ERROR] En el bucle de comunicacion: {e}")
                        break

    except KeyboardInterrupt:
        print("\n[INFO] Apagando el Orquestador...")
    finally:
        server_socket.close()

if __name__ == "__main__":
    main()
