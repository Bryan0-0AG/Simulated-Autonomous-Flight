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
    
    # Fallback: if no date-patterned dirs, return all subdirectories
    if not actual_dirs:
        actual_dirs = sorted([d for d in all_entries if os.path.isdir(os.path.join(log_dir, d))], reverse=True)
        
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

def process_telemetry_data(df, mission_id=None, matrix_id=None):
    """Processes the DataFrame into the JSON structure needed by the frontend Plotly charts."""
    if df is None or df.empty:
        return {"error": "Invalid or empty data"}
    
    try:
        # --- NAVIGATION METADATA (Before filtering) ---
        if 'mission_id' not in df.columns:
            df['mission_id'] = 0
        df['mission_id'] = df['mission_id'].fillna(0).astype(int)
        df['id'] = df['id'].fillna(0).astype(int)

        # Standardize 'children' column name (legacy support for 'childs' or 'hijos')
        if 'childs' in df.columns:
            df.rename(columns={'childs': 'children'}, inplace=True)
        elif 'hijos' in df.columns:
            df.rename(columns={'hijos': 'children'}, inplace=True)
        
        if 'children' not in df.columns:
            df['children'] = 0

        # Get ALL available IDs to maintain navigability
        all_missions = sorted([int(m) for m in df['mission_id'].unique().tolist() if m > 0])
        all_matrices = sorted([int(m) for m in df['id'].unique().tolist() if m > 0])
        
        # Fallback for sessions without missions
        if not all_missions and all_matrices:
            all_missions = all_matrices

        # --- GLOBAL FILTERING (For metrics and charts) ---
        if mission_id is not None:
            df = df[df['mission_id'] == int(mission_id)]
        elif matrix_id is not None:
            df = df[df['id'] == int(matrix_id)]
            
        if df.empty:
            return {
                "error": "No data for this filter", 
                "filter_data": {"missions": all_missions, "matrices": all_matrices},
                "metrics": {"duration":0,"drones":0,"matrices":0,"peak_velocity":0,"avg_battery":0},
                "trajectories": [],
                "states": {"labels":[], "values":[]},
                "time_series": {"time":[], "error":[], "battery":[], "repulsion":[], "speed":[], "acceleration":[]},
                "actuator": {"time":[], "thrust":[], "angle":[]}
            }

        # 1. Global Metrics
        last_time = df['time'].max()
        num_drones = int(df['num_drones'].max()) if 'num_drones' in df else 0
        num_matrices = len(df['id'].unique())
        avg_battery = df.groupby('id')['battery'].last().mean()
        max_speed = df['speed'].max()
        
        trajectories = []
        for drone_id, group in df.groupby('id'):
            trajectories.append({
                "id": int(drone_id),
                "mission_id": int(group['mission_id'].iloc[0]),
                "x": group['pos_x'].tolist(),
                "y": group['pos_y'].tolist(),
                "time": group['time'].tolist()
            })
            
        # 3. AI Intelligence Distribution (2x2 Grid)
        latest_data = df.sort_values('time').groupby('id').tail(1)
        
        # Rows with children > 0 are Matrices. We use them for both.
        matrices_df = latest_data[latest_data['children'] > 0]
        
        # Matrix Distribution: Direct counts of rows
        def get_matrix_dist(df_sub, col):
            if df_sub.empty or col not in df_sub.columns:
                return {"labels": [], "values": []}
            counts = df_sub[col].value_counts()
            return {"labels": counts.index.tolist(), "values": counts.values.tolist()}

        # Drone Distribution: Sum of 'childs' for each state/action
        def get_drone_dist(df_sub, col):
            if df_sub.empty or col not in df_sub.columns:
                return {"labels": [], "values": []}
            # Weighted count: sum children grouped by the column
            counts = df_sub.groupby(col)['children'].sum()
            return {"labels": counts.index.tolist(), "values": [int(v) for v in counts.values.tolist()]}

        dist_data = {
            "drone_states": get_drone_dist(matrices_df, 'state'),
            "drone_actions": get_drone_dist(matrices_df, 'action'),
            "matrix_states": get_matrix_dist(matrices_df, 'state'),
            "matrix_actions": get_matrix_dist(matrices_df, 'action')
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
        # Average only where repulsion is active to see the "stress" during interactions
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
            "filter_data": {
                "missions": all_missions,
                "matrices": all_matrices
            },
            "ai_dist": dist_data,
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
