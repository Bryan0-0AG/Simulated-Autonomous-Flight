import streamlit as st
import pandas as pd
import numpy as np
import os
import time
import plotly.express as px
import plotly.graph_objects as go

st.set_page_config(page_title="AMD Hackathon - Drone Swarm Telemetry", layout="wide")

# Estilos premium para una apariencia de hackathon
st.markdown("""
    <style>
    .main { background-color: #0e1117; color: #c9d1d9; }
    .stMetric { background-color: #161b22; border-radius: 10px; padding: 15px; border: 1px solid #30363d; box-shadow: 0 4px 6px rgba(0,0,0,0.3); }
    [data-testid="stSidebar"] { background-color: #0d1117; border-right: 1px solid #30363d; }
    </style>
    """, unsafe_allow_html=True)

st.title("🚁 Swarm Intelligence Dashboard")
st.markdown("---")

LOG_DIR = "telemetry/logs"
if not os.path.exists(LOG_DIR):
    os.makedirs(LOG_DIR, exist_ok=True)
    st.info("Directorio de logs creado. Ejecuta la simulación para generar datos.")
    st.stop()

# Selección de Simulación
dirs = sorted([d for d in os.listdir(LOG_DIR) if os.path.isdir(os.path.join(LOG_DIR, d))], reverse=True)
if not dirs:
    st.warning("No se encontraron carpetas de logs. Inicia la simulación en C++.")
    st.stop()

selected_run = st.sidebar.selectbox("Seleccionar Sesión", dirs)
auto_refresh = st.sidebar.checkbox("Actualización en Tiempo Real", value=True)

run_path = os.path.join(LOG_DIR, selected_run)
file_path = os.path.join(run_path, "Full_Telemetry.csv")

@st.cache_data(ttl=1)
def load_full_data(path):
    if os.path.exists(path):
        try:
            df = pd.read_csv(path)
            if df.empty: return None
            # Cálculo de magnitudes para análisis cinemático
            df['speed'] = np.sqrt(df['vel_x']**2 + df['vel_y']**2)
            df['f_total_sep'] = np.sqrt(df['f_sep_x']**2 + df['f_sep_y']**2)
            df['error_total'] = np.sqrt(df['error_x']**2 + df['error_y']**2)
            return df
        except Exception:
            return None
    return None

df = load_full_data(file_path)

if df is not None and not df.empty:
    # --- MÉTRICAS DE CABECERA ---
    last_time = df['time'].max()
    num_drones = df['num_drones'].max()
    
    m_col1, m_col2, m_col3, m_col4 = st.columns(4)
    with m_col1:
        st.metric("Duración", f"{last_time:.1f} s")
    with m_col2:
        st.metric("Enjambre", f"{num_drones} drones")
    with m_col3:
        st.metric("V_Max", f"{df['speed'].max():.1f} px/s")
    with m_col4:
        st.metric("Batería Media", f"{df.groupby('id')['battery'].last().mean():.1f}%")

    st.markdown("### Centro de Análisis")
    tab1, tab2, tab3, tab4 = st.tabs(["🚀 Física", "🎮 Control", "🧠 Inteligencia", "📊 Datos Crudos"])

    with tab1:
        st.subheader("Dinámica y Fuerzas")
        c1, c2 = st.columns(2)
        
        with c1:
            st.markdown("#### Balance de Fuerzas (Eje Y)")
            forces_y = df[['f_grav_y', 'f_thrust_y', 'f_sep_y', 'f_drag_y']].abs().mean()
            fig_forces = px.pie(
                values=forces_y.values, 
                names=['Gravedad', 'Empuje (PID)', 'Separación', 'Arrastre Aire'],
                color_discrete_sequence=px.colors.sequential.Electric,
                hole=.4
            )
            st.plotly_chart(fig_forces, width='stretch')
            
        with c2:
            st.markdown("#### Perfil de Velocidad")
            fig_speed = px.line(df[df['id'] < 5], x="time", y="speed", color="id", 
                               title="Velocidad de los primeros 5 drones")
            fig_speed.update_layout(template="plotly_dark")
            st.plotly_chart(fig_speed, width='stretch')

        st.markdown("#### Trayectorias Espaciales")
        # Usamos px.line para ver el "rastro" completo del vuelo
        fig_traj = px.line(df[df['id'] < 15], x="pos_x", y="pos_y", color="id", 
                           hover_data=['battery', 'state', 'time'], 
                           title="Rastro de Vuelo (Drones 0-14)")
        
        fig_traj.update_layout(
            template="plotly_dark",
            yaxis=dict(scaleanchor="x", scaleratio=1), # Proporción 1:1 para que no se deforme el mundo
            xaxis_title="X (px)",
            yaxis_title="Y (px)"
        )
        st.plotly_chart(fig_traj, width='stretch')

    with tab2:
        st.subheader("Rendimiento del Controlador PID")
        c1, c2 = st.columns(2)
        
        with c1:
            st.markdown("#### Error de Seguimiento")
            fig_error = go.Figure()
            avg_err = df.groupby('time')['error_total'].mean()
            fig_error.add_trace(go.Scatter(x=avg_err.index, y=avg_err.values, name="Error Total Medio", line=dict(color='#00ffcc')))
            fig_error.update_layout(template="plotly_dark", xaxis_title="Tiempo (s)", yaxis_title="Error (px)")
            st.plotly_chart(fig_error, width='stretch')
            
        with c2:
            st.markdown("#### Respuesta del Actuador")
            fig_act = px.scatter(df[df['id'] == 0], x="time", y="thrust_val", color="angle_val",
                                color_continuous_scale='Viridis', title="Thrust vs Angle (Dron 0)")
            fig_act.update_layout(template="plotly_dark")
            st.plotly_chart(fig_act, width='stretch')

    with tab3:
        st.subheader("Comportamiento y Autonomía")
        c1, c2 = st.columns(2)
        
        with c1:
            st.markdown("#### Salud de Batería")
            fig_bat = px.line(df.groupby('time')['battery'].mean().reset_index(), 
                             x="time", y="battery", title="Nivel de Batería Promedio")
            fig_bat.add_hline(y=20, line_dash="dash", line_color="red", annotation_text="RTB Threshold")
            fig_bat.update_layout(template="plotly_dark")
            st.plotly_chart(fig_bat, width='stretch')
            
        with c2:
            st.markdown("#### Distribución de Estados (IA)")
            state_counts = df['state'].value_counts().reset_index()
            state_counts.columns = ['Estado', 'Frecuencia']
            fig_state = px.bar(state_counts, x='Estado', y='Frecuencia', color='Estado')
            fig_state.update_layout(template="plotly_dark")
            st.plotly_chart(fig_state, width='stretch')

    with tab4:
        st.subheader("Explorador de Datos")
        st.dataframe(df.tail(100), width='stretch')

    # --- GUARDADO AUTOMÁTICO DE MÉTRICAS RESUMIDAS ---
    try:
        metrics_path = os.path.join(run_path, "Summary_Stats.csv")
        summary_data = {
            "Sim_Time": last_time,
            "Total_Drones": df['id'].nunique(),
            "Avg_Speed": df['speed'].mean(),
            "Max_Speed": df['speed'].max(),
            "Final_Avg_Battery": df.groupby('id')['battery'].last().mean(),
            "Avg_Position_Error": df['error_total'].mean(),
            "Avg_Thrust_Usage": df['thrust_val'].mean()
        }
        # Solo actualizamos el resumen si el tiempo ha avanzado significativamente
        pd.DataFrame([summary_data]).to_csv(metrics_path, index=False)
    except Exception:
        pass

else:
    st.info("📊 Telemetría en espera... Inicia la simulación para visualizar datos.")

# Mecanismo de auto-refresco
if auto_refresh:
    time.sleep(2)
    st.rerun()
