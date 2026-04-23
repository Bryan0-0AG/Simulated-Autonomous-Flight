import subprocess
import sys
import time
import os

def launch_process(command, name):
    """Lanza un proceso y maneja errores básicos."""
    print(f"[+] Iniciando {name}...")
    try:
        # En Windows, usamos sys.executable para asegurar que usamos el mismo Python
        return subprocess.Popen(command)
    except Exception as e:
        print(f"[!] Error al iniciar {name}: {e}")
        return None

def main():
    # Asegurar que estamos en la raíz del proyecto
    # Si brain.py está en ai_commander/, necesitamos subir un nivel
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    os.chdir(project_root)

    print("\n" + "="*40)
    print("🧠 AMD SWARM BRAIN - ORQUESTADOR")
    print("="*40 + "\n")

    # 1. Configuración de comandos
    app_exe = "app.exe" if sys.platform == "win32" else "./app"
    
    if not os.path.exists(app_exe):
        print(f"❌ Error: No se encuentra '{app_exe}'. Compila primero con 'make'.")
        return

    # 2. Lanzar Servicios en Segundo Plano
    commander = launch_process([sys.executable, "ai_commander/main_commander.py"], "Orquestador IA")
    dashboard = launch_process([sys.executable, "-m", "streamlit", "run", "telemetry/dashboard.py"], "Dashboard")

    time.sleep(4) # Pausa para inicialización

    # 3. Lanzar Simulacion (Proceso Principal)
    print("\n🚀 Lanzando simulacion de enjambre...")
    try:
        # Usamos subprocess.run para que bloquee hasta que se cierre la ventana
        subprocess.run([app_exe])
    except Exception as e:
        print(f"❌ Error en el motor: {e}")

    # 4. Limpieza Final
    print("\n" + "-"*40)
    print("Simulacion finalizada. Cerrando servicios...")
    
    for proc in [commander, dashboard]:
        if proc:
            try:
                proc.terminate()
                proc.wait(timeout=2)
            except:
                proc.kill()

    print("¡Mision cumplida! Hasta la proxima.")

if __name__ == "__main__":
    main()
