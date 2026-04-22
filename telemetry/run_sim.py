import subprocess
import sys
import time
import os

def main():
    # Asegurarnos de que el script se ejecuta desde la raíz del proyecto
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    os.chdir(project_root)

    print("\n" + "="*40)
    print("🚀 AMD DRONE SWARM SIMULATOR CONTROL")
    print("="*40 + "\n")
    
    # 1. Verificar si el ejecutable existe
    app_executable = "app.exe" if sys.platform == "win32" else "./app"
    if not os.path.exists(app_executable):
        print(f"❌ Error: No se encontró el binario '{app_executable}'.")
        print("👉 Por favor, compila el proyecto con 'make' antes de ejecutar este script.")
        return

    # 2. Iniciar el Dashboard (Streamlit)
    print("📈 Iniciando Dashboard de Telemetría (Plotly)...")
    try:
        # Iniciamos Streamlit en un proceso separado
        # -u para salida sin buffer para ver logs si es necesario
        streamlit_proc = subprocess.Popen([sys.executable, "-m", "streamlit", "run", "telemetry/dashboard.py"])
        print("✅ Dashboard en línea. Abre tu navegador en http://localhost:8501")
    except Exception as e:
        print(f"❌ Error al iniciar Streamlit: {e}")
        return
    
    # Pequeña pausa para que Streamlit se asiente
    time.sleep(3)
    
    # 3. Ejecutar la Simulación de C++
    print("🛸 Lanzando simulación de enjambre...")
    print("👉 Cierra la ventana de la simulación para terminar la sesión.\n")
    try:
        app_proc = subprocess.Popen([app_executable])
        # Esperamos a que el usuario cierre la ventana de la simulación (SFML)
        app_proc.wait()
    except Exception as e:
        print(f"❌ Error al ejecutar el motor de simulación: {e}")
    
    print("\n" + "-"*40)
    print("✅ Simulación finalizada correctamente.")
    print("📊 Los logs han sido guardados en la carpeta 'telemetry/logs/'.")
    print("-"*40)
    
    try:
        print("\n💡 El Dashboard sigue activo para que analices los últimos resultados.")
        input("👉 Presiona ENTER para cerrar el Dashboard y salir del programa...")
        streamlit_proc.terminate()
        streamlit_proc.wait()
    except (KeyboardInterrupt, EOFError):
        streamlit_proc.terminate()
    
    print("\n👋 ¡Hasta la próxima misión!")

if __name__ == "__main__":
    main()
