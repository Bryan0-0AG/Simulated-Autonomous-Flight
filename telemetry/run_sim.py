import subprocess
import sys
import time
import os
import shutil
import datetime

def main():
    # Asegurarnos de que el script se ejecuta desde la raíz del proyecto
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    os.chdir(project_root)

    log_dir = os.path.join("telemetry", "logs", "Actual Simulation")
    
    # 1. Preparar el directorio "Actual Simulation" limpiándolo si ya existe
    if os.path.exists(log_dir):
        shutil.rmtree(log_dir, ignore_errors=True)
    os.makedirs(log_dir, exist_ok=True)
    
    # Crear archivos vacíos para que Streamlit no tire error al iniciar antes que C++
    for f in ["Physics.csv", "Control.csv", "AI.csv", "All.csv"]:
        open(os.path.join(log_dir, f), 'w').close()

    print("Iniciando el Dashboard...")
    # Iniciar Streamlit en un proceso separado (abre el navegador web automáticamente)
    streamlit_proc = subprocess.Popen([sys.executable, "-m", "streamlit", "run", "telemetry/dashboard.py"])
    
    # Darle un par de segundos para que levante el servidor web
    time.sleep(3)
    
    print("Iniciando Simulación...")
    # Ejecutar la aplicación de C++
    app_executable = "app.exe" if sys.platform == "win32" else "./app"
    try:
        app_proc = subprocess.Popen([app_executable])
        # Esperar hasta que se cierre la ventana de la simulación
        app_proc.wait()
    except FileNotFoundError:
        print(f"Error: No se encontró el ejecutable {app_executable}. Compila el proyecto primero.")
    
    print("Simulación cerrada. Finalizando el Dashboard...")
    # Matar el proceso de Streamlit
    streamlit_proc.terminate()
    streamlit_proc.wait()
    
    # 2. Renombrar el directorio a la fecha y hora de finalización
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    new_dir = os.path.join("telemetry", "logs", timestamp)
    try:
        os.rename(log_dir, new_dir)
        print(f"Log guardado exitosamente como: {timestamp}")
    except Exception as e:
        print(f"No se pudo renombrar el log final: {e}")
        
    print("Ejecución finalizada correctamente.")

if __name__ == "__main__":
    main()
