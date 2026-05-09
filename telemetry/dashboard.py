import os
import re
import pandas as pd
import numpy as np

def get_available_sessions(log_dir):
    """Returns a list of valid session directories sorted from newest to oldest."""
    if not os.path.exists(log_dir):
        return []
    
    date_pattern = re.compile(r'\d{4}-\d{2}-\d{2}_\d{2}-\d{2}-\d{2}')
    all_entries = os.listdir(log_dir)
    actual_dirs = sorted([d for d in all_entries if os.path.isdir(os.path.join(log_dir, d)) and date_pattern.match(d)], reverse=True)
    return actual_dirs

def load_full_data(path):
    """Loads and cleans the telemetry CSV data."""
    if not os.path.exists(path):
        return None
    try:
        df = pd.read_csv(path, low_memory=False)
        if df.empty or len(df.columns) < 10: 
            return None
            
        # Data Cleaning
        numeric_cols = ['vel_x', 'vel_y', 'f_sep_x', 'f_sep_y', 'error_x', 'error_y', 'thrust_val', 'angle_val', 'battery', 'time']
        for col in numeric_cols:
            if col in df.columns:
                df[col] = pd.to_numeric(df[col], errors='coerce')
        
        df = df.dropna(subset=['time', 'id'])
        
        # Outlier filtering for Actuator Response
        df.loc[df['thrust_val'].abs() > 1000, 'thrust_val'] = 0
        df.loc[df['angle_val'].abs() > 5, 'angle_val'] = 0
        
        # Calculate magnitudes
        df['speed'] = np.sqrt(df['vel_x']**2 + df['vel_y']**2)
        df['f_total_sep'] = np.sqrt(df['f_sep_x']**2 + df['f_sep_y']**2)
        df['error_total'] = np.sqrt(df['error_x']**2 + df['error_y']**2)
        return df
    except Exception as e:
        print(f"Error loading CSV: {e}")
        return None

def process_telemetry_data(df):
    """Processes the DataFrame into the JSON structure needed by the frontend Plotly charts."""
    if df is None or df.empty:
        return {"error": "Invalid or empty data"}
        
    try:
        # 1. Global Metrics
        last_time = df['time'].max()
        num_drones = int(df['num_drones'].max()) if 'num_drones' in df else 0
        num_matrices = len(df['id'].unique())
        avg_battery = df.groupby('id')['battery'].last().mean()
        max_speed = df['speed'].max()
        
        # 2. Global Swarm Trajectories (Sample first 15 drones)
        sample_size = 15
        df_traj = df[df['id'] < sample_size].copy()
        trajectories = []
        for drone_id, group in df_traj.groupby('id'):
            trajectories.append({
                "id": int(drone_id),
                "x": group['pos_x'].tolist(),
                "y": group['pos_y'].tolist(),
                "time": group['time'].tolist()
            })
            
        # 3. AI State Distribution
        latest_states = df.sort_values('time').groupby('id').tail(1)
        state_counts = latest_states['state'].value_counts()
        states_pie = {
            "labels": state_counts.index.tolist(),
            "values": state_counts.values.tolist()
        }
        
        # 4. PID Error over time
        avg_err = df.groupby('time')['error_total'].mean().reset_index()
        
        # 5. Power Consumption over time
        avg_bat = df.groupby('time')['battery'].mean().reset_index()
        
        # 6. Actuator Response (first drone)
        target_id = df['id'].unique()[0] if len(df['id'].unique()) > 0 else 0
        d_target = df[df['id'] == target_id]
        actuator = {
            "time": d_target['time'].tolist(),
            "thrust": d_target['thrust_val'].tolist(),
            "angle": d_target['angle_val'].tolist()
        }
        
        # 7. Collision Avoidance Stress
        sep_data = df.groupby('time')['f_total_sep'].mean().reset_index()
        
        # 8. Kinematic Profile
        swarm_stats = df.groupby('time').agg({
            'speed': 'mean'
        }).reset_index()
        swarm_stats['acceleration'] = swarm_stats['speed'].diff() / swarm_stats['time'].diff()
        swarm_stats['acceleration'] = swarm_stats['acceleration'].fillna(0).abs()
        
        return {
            "metrics": {
                "duration": float(last_time),
                "drones": int(num_drones),
                "matrices": int(num_matrices),
                "peak_velocity": float(max_speed),
                "avg_battery": float(avg_battery)
            },
            "trajectories": trajectories,
            "states": states_pie,
            "time_series": {
                "time": avg_err['time'].tolist(),
                "error": avg_err['error_total'].tolist(),
                "battery": avg_bat['battery'].tolist(),
                "repulsion": sep_data['f_total_sep'].tolist(),
                "speed": swarm_stats['speed'].tolist(),
                "acceleration": swarm_stats['acceleration'].tolist()
            },
            "actuator": actuator
        }
    except Exception as e:
        import traceback
        traceback.print_exc()
        return {"error": str(e)}
