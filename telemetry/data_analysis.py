import pandas as pd
import matplotlib.pyplot as plt
import os

# Ruta donde están tus logs
LOG_DIR = "telemetry/logs"
files = [f for f in os.listdir(LOG_DIR) if f.endswith(".csv")]
if not files:
    print("No hay logs generados aún.")
    exit()

latest_file = max(files, key=lambda f: os.path.getctime(os.path.join(LOG_DIR, f)))

path = os.path.join(LOG_DIR, latest_file)
print(f"Analizando: {path}")

# Cargar CSV
df = pd.read_csv(path)

# =========================
# INFO BÁSICA
# =========================
print("\n--- INFO ---")
print(df.head())
print(df.describe())

# =========================
# TRAZADO DE POSICIÓN
# =========================
plt.figure()
for body_id in df["id"].unique():
    body = df[df["id"] == body_id]
    plt.plot(body["pos_x"], body["pos_y"], label=f"Body {body_id}")

plt.title("Trayectoria de cuerpos")
plt.xlabel("X")
plt.ylabel("Y")
plt.legend()
plt.grid()
plt.show()

# =========================
# VELOCIDAD EN EL TIEMPO
# =========================
# Velocidad X
plt.figure()
for body_id in df["id"].unique():
    body = df[df["id"] == body_id]
    plt.plot(body["time"], body["vel_x"], label=f"Body {body_id} vel_x")

plt.title("Velocidad X en el tiempo")
plt.xlabel("Tiempo")
plt.ylabel("Velocidad X")
plt.legend()
plt.grid()
plt.show()

# Velocidad Y
plt.figure()
for body_id in df["id"].unique():
    body = df[df["id"] == body_id]
    plt.plot(body["time"], body["vel_y"], label=f"Body {body_id} vel_y")

plt.title("Velocidad Y en el tiempo")
plt.xlabel("Tiempo")
plt.ylabel("Velocidad Y")
plt.legend()
plt.grid()
plt.show()