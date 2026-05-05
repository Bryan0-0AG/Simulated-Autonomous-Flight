import streamlit as st
import pandas as pd
import numpy as np
import os
import time
import plotly.express as px
import plotly.graph_objects as go

# Page Configuration
st.set_page_config(page_title="AMD Hackathon - Swarm Telemetry", layout="wide")

# Custom Styles for Hackathon Aesthetics
st.markdown("""
    <style>
    .main { background-color: #0e1117; color: #c9d1d9; }
    .stMetric { 
        background-color: #161b22; 
        border-radius: 10px; 
        padding: 15px; 
        border: 1px solid #30363d; 
        box-shadow: 0 4px 12px rgba(0,0,0,0.5); 
    }
    div[data-testid="metric-container"] {
       border: 1px solid #30363d;
       padding: 10px;
       border-radius: 10px;
       background: rgba(48, 54, 61, 0.2);
    }
    </style>
    """, unsafe_allow_html=True)

st.title("🚁 Swarm Intelligence Dashboard")
st.caption("Real-time telemetry analysis for massive autonomous agent systems - Powered by AMD ROCm/HIP")

LOG_DIR = "telemetry/logs"
if not os.path.exists(LOG_DIR):
    os.makedirs(LOG_DIR, exist_ok=True)
    st.info("Log directory created. Run the C++ simulation to generate data.")
    st.stop()

import re

# Session Selection - Automatically pick the latest by timestamp name (Ignoring 'old' or other text folders)
date_pattern = re.compile(r'\d{4}-\d{2}-\d{2}_\d{2}-\d{2}-\d{2}')
all_entries = os.listdir(LOG_DIR)
actual_dirs = sorted([d for d in all_entries if os.path.isdir(os.path.join(LOG_DIR, d)) and date_pattern.match(d)], reverse=True)

if not actual_dirs:
    st.warning("No log sessions with date format found. Start the simulation first.")
    st.stop()

# Sidebar for controls
with st.sidebar:
    st.header("Control Center")
    selected_option = st.selectbox("Select Mission Session", actual_dirs, index=0)
    auto_refresh = st.checkbox("Live Update", value=True)
    st.markdown("---")
    st.markdown("### Mission Status")

selected_run = selected_option
run_path = os.path.join(LOG_DIR, selected_run)
file_path = os.path.join(run_path, "Full_Telemetry.csv")

@st.cache_data(ttl=1)
def load_full_data(path):
    if os.path.exists(path):
        try:
            df = pd.read_csv(path, low_memory=False)
            if df.empty or len(df.columns) < 10: return None
            
            # Data Cleaning
            numeric_cols = ['vel_x', 'vel_y', 'f_sep_x', 'f_sep_y', 'error_x', 'error_y', 'thrust_val', 'angle_val', 'battery', 'time']
            for col in numeric_cols:
                if col in df.columns:
                    df[col] = pd.to_numeric(df[col], errors='coerce')
            
            df = df.dropna(subset=['time', 'id'])
            
            # Outlier filtering for Actuator Response (Avoid 10^35 garbage)
            df.loc[df['thrust_val'].abs() > 1000, 'thrust_val'] = 0
            df.loc[df['angle_val'].abs() > 5, 'angle_val'] = 0
            
            # Calculate magnitudes for analysis
            df['speed'] = np.sqrt(df['vel_x']**2 + df['vel_y']**2)
            df['f_total_sep'] = np.sqrt(df['f_sep_x']**2 + df['f_sep_y']**2)
            df['error_total'] = np.sqrt(df['error_x']**2 + df['error_y']**2)
            return df
        except Exception as e:
            print(f"Error loading CSV: {e}")
            return None
    return None

df = load_full_data(file_path)

if df is not None and not df.empty:
    # Update Mission Status in Sidebar
    with st.sidebar:
        st.success("HPC Engine: ACTIVE")
        st.info(f"Agents: {df['num_drones'].max()}")

    # --- TOP METRICS ---
    last_time = df['time'].max()
    num_drones = df['num_drones'].max()
    avg_battery = df.groupby('id')['battery'].last().mean()
    max_speed = df['speed'].max()

    m_col1, m_col2, m_col3, m_col4 = st.columns(4)
    m_col1.metric("Mission Duration", f"{last_time:.1f} s")
    m_col2.metric("Active Swarm", f"{num_drones} agents")
    m_col3.metric("Peak Velocity", f"{max_speed:.1f} px/s")
    m_col4.metric("Avg. Battery", f"{avg_battery:.1f}%")

    st.markdown("---")

    # --- SINGLE WINDOW LAYOUT ---
    
    # ROW 1: Swarm Dynamics & AI States
    r1_c1, r1_c2 = st.columns([2, 1])
    
    with r1_c1:
        st.subheader("🌐 Global Swarm Trajectories")
        sample_size = 15
        # Filter to ensure we have data for the plot
        df_traj = df[df['id'] < sample_size].copy()
        if not df_traj.empty:
            fig_traj = px.line(df_traj, x="pos_x", y="pos_y", color="id", 
                               hover_data=['battery', 'state', 'time'], 
                               template="plotly_dark")
            fig_traj.update_layout(
                margin=dict(l=0, r=0, t=0, b=0),
                yaxis=dict(scaleanchor="x", scaleratio=1),
                xaxis_title="World X", yaxis_title="World Y"
            )
            st.plotly_chart(fig_traj, width='stretch')
        else:
            st.write("Waiting for trajectory data...")

    with r1_c2:
        st.subheader("🧠 AI State Distribution")
        latest_states = df.sort_values('time').groupby('id').tail(1)
        state_counts = latest_states['state'].value_counts()
        fig_states = px.pie(
            values=state_counts.values, 
            names=state_counts.index,
            hole=0.4,
            color_discrete_sequence=px.colors.sequential.Viridis,
            template="plotly_dark"
        )
        fig_states.update_layout(margin=dict(l=0, r=0, t=30, b=0))
        st.plotly_chart(fig_states, width='stretch')

    # ROW 2: Flight Control & Power Metrics
    st.markdown("---")
    r2_c1, r2_c2, r2_c3 = st.columns(3)

    with r2_c1:
        st.subheader("🎮 PID Control Performance")
        avg_err = df.groupby('time')['error_total'].mean().reset_index()
        fig_error = go.Figure()
        fig_error.add_trace(go.Scatter(x=avg_err['time'], y=avg_err['error_total'], name="Mean Error", fill='tozeroy', line=dict(color='#00ffcc')))
        fig_error.update_layout(
            template="plotly_dark", 
            margin=dict(l=0, r=0, t=30, b=0),
            xaxis_title="Time (s)", yaxis_title="Tracking Error (px)"
        )
        st.plotly_chart(fig_error, width='stretch')

    with r2_c2:
        st.subheader("⚡ Power Consumption")
        avg_bat = df.groupby('time')['battery'].mean().reset_index()
        fig_bat = px.line(avg_bat, x="time", y="battery", template="plotly_dark")
        fig_bat.add_hline(y=20, line_dash="dash", line_color="red", annotation_text="RTB")
        fig_bat.update_layout(margin=dict(l=0, r=0, t=30, b=0), xaxis_title="Time (s)", yaxis_title="Battery %")
        st.plotly_chart(fig_bat, width='stretch')

    with r2_c3:
        st.subheader("⚙️ Actuator Response")
        # Ensure we pick a drone that actually exists in the data
        available_ids = df['id'].unique()
        target_id = available_ids[0] if len(available_ids) > 0 else 0
        d_target = df[df['id'] == target_id].copy()
        
        if not d_target.empty:
            fig_act = px.scatter(d_target, x="time", y="thrust_val", color="angle_val",
                                color_continuous_scale='RdYlGn', template="plotly_dark",
                                title=f"Agent {target_id} Output")
            fig_act.update_layout(margin=dict(l=0, r=0, t=30, b=0), xaxis_title="Time (s)", yaxis_title="Thrust")
            st.plotly_chart(fig_act, width='stretch')
        else:
            st.write("No actuator data yet.")

    # ROW 3: Collision Avoidance & Speed (Advanced Metrics)
    st.markdown("---")
    r3_c1, r3_c2 = st.columns(2)

    with r3_c1:
        st.subheader("🛡️ Collision Avoidance Stress")
        # Group by time and calculate mean repulsion
        sep_data = df.groupby('time')['f_total_sep'].mean().reset_index()
        if not sep_data.empty:
            fig_sep = px.area(sep_data, x="time", y="f_total_sep", template="plotly_dark",
                             color_discrete_sequence=['#ff6600'])
            fig_sep.update_layout(margin=dict(l=0, r=0, t=30, b=0), xaxis_title="Time (s)", yaxis_title="Repulsion Force")
            st.plotly_chart(fig_sep, width='stretch')
        else:
            st.write("No repulsion data.")

    with r3_c2:
        st.subheader("📊 Kinematic Profile (Avg)")
        # Calculate swarm averages over time
        swarm_stats = df.groupby('time').agg({
            'speed': 'mean',
            'thrust_val': 'mean'
        }).reset_index()
        
        # Approximate acceleration from speed change
        swarm_stats['acceleration'] = swarm_stats['speed'].diff() / swarm_stats['time'].diff()
        swarm_stats['acceleration'] = swarm_stats['acceleration'].fillna(0).abs()

        fig_kin = go.Figure()
        fig_kin.add_trace(go.Scatter(
            x=swarm_stats['time'], y=swarm_stats['speed'],
            name="Mean Velocity", line=dict(color='#3399ff', width=2)
        ))
        fig_kin.add_trace(go.Scatter(
            x=swarm_stats['time'], y=swarm_stats['acceleration'],
            name="Mean Acceleration", line=dict(color='#ff33cc', width=2, dash='dot'),
            yaxis="y2"
        ))

        fig_kin.update_layout(
            template="plotly_dark",
            margin=dict(l=0, r=0, t=30, b=0),
            xaxis_title="Time (s)",
            yaxis_title="Velocity (px/s)",
            yaxis2=dict(title="Acceleration (px/s²)", overlaying="y", side="right"),
            legend=dict(orientation="h", yanchor="bottom", y=1.02, xanchor="right", x=1)
        )
        st.plotly_chart(fig_kin, width='stretch')

    # Auto-refresh logic
    if auto_refresh:
        time.sleep(2)
        st.rerun()
else:
    st.sidebar.error("Waiting for valid telemetry data...")
    st.warning("Telemetry file is empty or missing columns. Ensure simulation is running.")
    time.sleep(2)
    st.rerun()
