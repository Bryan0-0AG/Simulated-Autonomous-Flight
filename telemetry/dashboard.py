import streamlit as st
import pandas as pd
import numpy as np
import os
import time
import matplotlib.pyplot as plt

st.set_page_config(page_title="Advanced Flight Simulation Dashboard", layout="wide")

st.title("🚁 Advanced Flight Simulation Dashboard")
st.markdown("Análisis en Tiempo Real de la Telemetría de los Drones.")

LOG_DIR = "telemetry/logs"
if not os.path.exists(LOG_DIR):
    st.error(f"No log directory found at {LOG_DIR}")
    st.stop()

dirs = [d for d in os.listdir(LOG_DIR) if os.path.isdir(os.path.join(LOG_DIR, d))]
if not dirs:
    st.warning("No logs generated yet.")
    st.stop()

sorted_dirs = sorted(dirs, reverse=True)

# Buscar "Actual Simulation" para que sea el valor por defecto si existe
default_index = 0
if "Actual Simulation" in sorted_dirs:
    default_index = sorted_dirs.index("Actual Simulation")

selected_run = st.sidebar.selectbox("Select Simulation Run", sorted_dirs, index=default_index)

# Checkbox for real-time updates
auto_refresh = st.sidebar.checkbox("Activar Actualización en Tiempo Real", value=True)

run_path = os.path.join(LOG_DIR, selected_run)

def load_data(path, filename):
    file_path = os.path.join(path, filename)
    if os.path.exists(file_path):
        try:
            return pd.read_csv(file_path)
        except Exception:
            return None
    return None

df_physics = load_data(run_path, "Physics.csv")
df_control = load_data(run_path, "Control.csv")
df_ai = load_data(run_path, "AI.csv")

st.markdown("---")
st.header("🌎 Análisis Físico y Cinemático")
if df_physics is not None and not df_physics.empty:
    st.markdown("### Métricas Generales")
    col1, col2, col3, col4 = st.columns(4)
    
    # Calculations
    df_physics['speed'] = np.sqrt(df_physics['vel_x']**2 + df_physics['vel_y']**2)
    avg_speed = df_physics['speed'].mean()
    max_speed = df_physics['speed'].max()
    
    # Approximate distance
    df_physics['dx'] = df_physics.groupby('id')['pos_x'].diff().fillna(0)
    df_physics['dy'] = df_physics.groupby('id')['pos_y'].diff().fillna(0)
    df_physics['step_dist'] = np.sqrt(df_physics['dx']**2 + df_physics['dy']**2)
    total_dist = df_physics.groupby('id')['step_dist'].sum().mean()
    
    col1.metric("Velocidad Promedio", f"{avg_speed:.2f} px/s")
    col2.metric("Velocidad Máxima", f"{max_speed:.2f} px/s")
    col3.metric("Distancia Promedio Recorrida", f"{total_dist:.2f} px")
    col4.metric("Drones Activos", f"{df_physics['id'].nunique()}")

    col_plot1, col_plot2 = st.columns(2)
    with col_plot1:
        st.subheader("Trayectorias en Tiempo Real")
        fig, ax = plt.subplots()
        for body_id in df_physics["id"].unique():
            body = df_physics[df_physics["id"] == body_id]
            ax.plot(body["pos_x"], body["pos_y"], label=f"Drone {body_id}")
        ax.set_title("Trayectoria en el espacio")
        ax.set_xlabel("Posición X")
        ax.set_ylabel("Posición Y")
        ax.grid(True, linestyle='--', alpha=0.6)
        st.pyplot(fig)
    
    with col_plot2:
        st.subheader("Perfil de Velocidades")
        fig, ax = plt.subplots()
        for body_id in df_physics["id"].unique():
            body = df_physics[df_physics["id"] == body_id]
            ax.plot(body["time"], body["speed"], alpha=0.5, label=f"Drone {body_id}")
        ax.set_title("Magnitud de Velocidad vs Tiempo")
        ax.set_xlabel("Tiempo (s)")
        ax.set_ylabel("Velocidad (px/s)")
        ax.grid(True, linestyle='--', alpha=0.6)
        st.pyplot(fig)
else:
    st.warning("No hay datos de Física (Aún).")

st.markdown("---")
st.header("⚙️ Estabilidad y Control PID")
if df_control is not None and not df_control.empty:
    st.markdown("### Métricas de Precisión")
    col1, col2, col3 = st.columns(3)
    
    rmse_x = np.sqrt((df_control['error_x']**2).mean())
    rmse_y = np.sqrt((df_control['error_y']**2).mean())
    avg_thrust = df_control['thrust'].mean()
    
    col1.metric("RMSE Error X", f"{rmse_x:.2f}")
    col2.metric("RMSE Error Y", f"{rmse_y:.2f}")
    col3.metric("Empuje Promedio (Thrust)", f"{avg_thrust:.2f}")
    
    col_plot1, col_plot2 = st.columns(2)
    with col_plot1:
        st.subheader("Evolución del Error")
        fig, ax = plt.subplots()
        mean_err_x = df_control.groupby('time')['error_x'].mean()
        mean_err_y = df_control.groupby('time')['error_y'].mean()
        ax.plot(mean_err_x.index, mean_err_x.values, label="Error Medio X", color='blue')
        ax.plot(mean_err_y.index, mean_err_y.values, label="Error Medio Y", color='red')
        ax.set_title("Error de Posición Promedio vs Tiempo")
        ax.set_xlabel("Tiempo (s)")
        ax.set_ylabel("Error (px)")
        ax.legend()
        ax.grid(True, linestyle='--', alpha=0.6)
        st.pyplot(fig)
        
    with col_plot2:
        st.subheader("Uso de Empuje (Thrust)")
        fig, ax = plt.subplots()
        mean_thrust = df_control.groupby('time')['thrust'].mean()
        ax.plot(mean_thrust.index, mean_thrust.values, color='green')
        ax.set_title("Empuje Promedio vs Tiempo")
        ax.set_xlabel("Tiempo (s)")
        ax.set_ylabel("Empuje")
        ax.grid(True, linestyle='--', alpha=0.6)
        st.pyplot(fig)
else:
    st.warning("No hay datos de Control (Aún).")

st.markdown("---")
st.header("🧠 Autonomía y Toma de Decisiones (IA)")
if df_ai is not None and not df_ai.empty:
    st.markdown("### Rendimiento y Misiones")
    col1, col2, col3 = st.columns(3)
    
    state_counts = df_ai['state'].value_counts()
    most_common_state = state_counts.idxmax() if not state_counts.empty else "N/A"
    
    df_ai['bat_drop'] = df_ai.groupby('id')['battery'].diff()
    avg_drop = abs(df_ai[df_ai['bat_drop'] < 0]['bat_drop'].mean()) * 60 # por min
    
    col1.metric("Estado más común", most_common_state)
    col2.metric("Consumo Batería", f"{avg_drop:.2f} / min" if pd.notna(avg_drop) else "N/A")
    
    col_plot1, col_plot2 = st.columns(2)
    with col_plot1:
        st.subheader("Nivel de Batería")
        fig, ax = plt.subplots()
        mean_bat = df_ai.groupby('time')['battery'].mean()
        ax.plot(mean_bat.index, mean_bat.values, color='purple', linewidth=2)
        ax.fill_between(mean_bat.index, mean_bat.values, color='purple', alpha=0.2)
        ax.set_title("Descarga Promedio de la Flota")
        ax.set_xlabel("Tiempo (s)")
        ax.set_ylabel("Batería (%)")
        ax.grid(True, linestyle='--', alpha=0.6)
        st.pyplot(fig)
        
    with col_plot2:
        st.subheader("Distribución de Estados")
        st.bar_chart(state_counts)
else:
    st.warning("No hay datos de IA (Aún).")

# Guardado automático de métricas a través del tiempo
try:
    if df_physics is not None and not df_physics.empty:
        sim_time = float(df_physics['time'].max())
        metrics = {
            "Sim_Time": sim_time,
            "Avg_Speed_px_s": round(avg_speed, 2) if 'avg_speed' in locals() else 0.0,
            "Max_Speed_px_s": round(max_speed, 2) if 'max_speed' in locals() else 0.0,
            "Avg_Dist_px": round(total_dist, 2) if 'total_dist' in locals() else 0.0,
            "Active_Drones": int(df_physics['id'].nunique()),
            "RMSE_X": round(rmse_x, 2) if 'rmse_x' in locals() else 0.0,
            "RMSE_Y": round(rmse_y, 2) if 'rmse_y' in locals() else 0.0,
            "Avg_Thrust": round(avg_thrust, 2) if 'avg_thrust' in locals() else 0.0,
            "Most_Common_State": most_common_state if 'most_common_state' in locals() else "N/A",
            "Battery_Drop_per_min": round(avg_drop, 2) if 'avg_drop' in locals() and pd.notna(avg_drop) else 0.0
        }
        
        metrics_df = pd.DataFrame([metrics])
        metrics_path = os.path.join(run_path, "Summary_Stats.csv")
        
        # Solo escribimos si el archivo no existe o si el tiempo de simulación ha avanzado
        if not os.path.exists(metrics_path):
            metrics_df.to_csv(metrics_path, index=False)
        else:
            existing_df = pd.read_csv(metrics_path)
            if existing_df.empty or existing_df.iloc[-1]['Sim_Time'] != sim_time:
                metrics_df.to_csv(metrics_path, mode='a', header=False, index=False)
except Exception as e:
    pass

# Auto-refresh logic
if auto_refresh:
    time.sleep(1)
    st.rerun()
