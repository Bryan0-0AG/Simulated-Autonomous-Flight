document.addEventListener('DOMContentLoaded', () => {
    const selectSession = document.getElementById('dash-session-select');
    const btnRefresh = document.getElementById('btn-refresh-dash');
    
    // Shared Plotly layout styling for dark mode
    const darkLayout = {
        paper_bgcolor: 'rgba(0,0,0,0)',
        plot_bgcolor: 'rgba(0,0,0,0)',
        font: { color: '#c9d1d9', family: 'Inter, sans-serif' },
        margin: { l: 40, r: 20, t: 10, b: 30 },
        xaxis: { gridcolor: '#30363d', zerolinecolor: '#30363d' },
        yaxis: { gridcolor: '#30363d', zerolinecolor: '#30363d' }
    };

    let currentSessions = [];

    async function loadSessions() {
        try {
            const res = await fetch('/api/telemetry/sessions');
            const sessions = await res.json();
            
            // Check if the list of sessions has changed (e.g., new simulation started)
            if (JSON.stringify(sessions) !== JSON.stringify(currentSessions)) {
                currentSessions = sessions;
                
                selectSession.innerHTML = '';
                if (sessions.length === 0) {
                    selectSession.innerHTML = '<option value="">No sessions found</option>';
                    return true; // We handled it
                }
                
                sessions.forEach(s => {
                    const opt = document.createElement('option');
                    opt.value = s;
                    opt.textContent = s;
                    selectSession.appendChild(opt);
                });
                
                // Automatically pick the newest session (which is index 0)
                selectSession.value = sessions[0];
                loadDashboardData();
                return true; // Indicates we already loaded data
            }
            return false; // List hasn't changed
        } catch (e) {
            console.error("Failed to load sessions:", e);
            return false;
        }
    }

    async function loadDashboardData() {
        const session = selectSession.value;
        if (!session) return;
        
        try {
            const res = await fetch(`/api/telemetry/data?session=${session}`);
            const data = await res.json();
            
            if (data.error) {
                console.error("Dashboard data error:", data.error);
                return;
            }
            
            // 1. Update Metrics
            document.getElementById('m-duration').textContent = `${data.metrics.duration.toFixed(1)} s`;
            document.getElementById('m-swarm').textContent = `${data.metrics.drones} agents`;
            if (document.getElementById('m-matrices')) {
                document.getElementById('m-matrices').textContent = `${data.metrics.matrices} agents`;
            }
            document.getElementById('m-velocity').textContent = `${data.metrics.peak_velocity.toFixed(1)} px/s`;
            document.getElementById('m-battery').textContent = `${data.metrics.avg_battery.toFixed(1)}%`;
            
            // 2. Trajectories
            const trajTraces = data.trajectories.map(t => ({
                x: t.x, y: t.y, mode: 'lines', name: `Agent ${t.id}`,
                line: { width: 1 }
            }));
            Plotly.newPlot('plot-traj', trajTraces, {
                ...darkLayout, showlegend: false,
                xaxis: { ...darkLayout.xaxis, title: 'World X' },
                yaxis: { ...darkLayout.yaxis, title: 'World Y', scaleanchor: "x", scaleratio: 1 }
            }, {responsive: true});
            
            // 3. States Pie
            Plotly.newPlot('plot-states', [{
                values: data.states.values,
                labels: data.states.labels,
                type: 'pie',
                hole: 0.4,
                marker: { colors: ['#2ca02c', '#1f77b4', '#ff7f0e', '#d62728', '#9467bd'] }
            }], { ...darkLayout, margin: { l: 0, r: 0, t: 10, b: 0 } }, {responsive: true});
            
            // 4. PID Control
            Plotly.newPlot('plot-pid', [{
                x: data.time_series.time,
                y: data.time_series.error,
                type: 'scatter', mode: 'lines', fill: 'tozeroy',
                line: { color: '#00ffcc' }, name: 'Mean Error'
            }], { ...darkLayout, xaxis: { title: 'Time (s)', gridcolor: '#30363d'}, yaxis: { title: 'Tracking Error (px)', gridcolor: '#30363d'} }, {responsive: true});
            
            // 5. Power Consumption
            Plotly.newPlot('plot-power', [{
                x: data.time_series.time,
                y: data.time_series.battery,
                type: 'scatter', mode: 'lines',
                line: { color: '#ff3366' }, name: 'Battery %'
            }], { ...darkLayout, xaxis: { title: 'Time (s)', gridcolor: '#30363d'}, yaxis: { title: 'Battery %', gridcolor: '#30363d'} }, {responsive: true});
            
            // 6. Actuator
            Plotly.newPlot('plot-actuator', [{
                x: data.actuator.time,
                y: data.actuator.thrust,
                mode: 'markers',
                marker: {
                    color: data.actuator.angle,
                    colorscale: 'RdYlGn',
                    showscale: true,
                    size: 6
                }
            }], { ...darkLayout, xaxis: { title: 'Time (s)', gridcolor: '#30363d'}, yaxis: { title: 'Thrust', gridcolor: '#30363d'} }, {responsive: true});
            
            // 7. Collision Avoidance
            Plotly.newPlot('plot-collision', [{
                x: data.time_series.time,
                y: data.time_series.repulsion,
                type: 'scatter', mode: 'lines', fill: 'tozeroy',
                line: { color: '#ff6600' }
            }], { ...darkLayout, xaxis: { title: 'Time (s)', gridcolor: '#30363d'}, yaxis: { title: 'Repulsion Force', gridcolor: '#30363d'} }, {responsive: true});
            
            // 8. Kinematic Profile
            Plotly.newPlot('plot-kinematic', [
                { x: data.time_series.time, y: data.time_series.speed, type: 'scatter', mode: 'lines', name: 'Velocity', line: { color: '#3399ff' } },
                { x: data.time_series.time, y: data.time_series.acceleration, type: 'scatter', mode: 'lines', name: 'Acceleration', yaxis: 'y2', line: { color: '#ff33cc', dash: 'dot' } }
            ], {
                ...darkLayout, 
                xaxis: { title: 'Time (s)', gridcolor: '#30363d'},
                yaxis: { title: 'Velocity', gridcolor: '#30363d'},
                yaxis2: { title: 'Acceleration', overlaying: 'y', side: 'right', gridcolor: 'rgba(0,0,0,0)' },
                legend: { orientation: "h", yanchor: "bottom", y: 1.02, xanchor: "right", x: 1 }
            }, {responsive: true});
            
        } catch (e) {
            console.error("Failed to load dashboard data:", e);
        }
    }

    btnRefresh.addEventListener('click', loadDashboardData);
    selectSession.addEventListener('change', loadDashboardData);

    // Initial load
    loadSessions();
    
    // Auto refresh every 5 seconds if view is active
    setInterval(async () => {
        if (document.getElementById('view-analytics').classList.contains('active')) {
            const listChanged = await loadSessions();
            if (!listChanged) {
                // If the list didn't change (no new session), just fetch new data for the current one
                loadDashboardData();
            }
        }
    }, 5000);
});
