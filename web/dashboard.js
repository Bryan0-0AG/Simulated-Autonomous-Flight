document.addEventListener('DOMContentLoaded', () => {
    console.log("[Dashboard] Initializing tactical intelligence...");
    
    const selectSession = document.getElementById('dash-session-select');
    const btnRefresh = document.getElementById('btn-refresh-dash');
    const filterType = document.getElementById('filter-type');
    const filterValue = document.getElementById('filter-value');
    
    const darkLayout = {
        paper_bgcolor: 'rgba(0,0,0,0)',
        plot_bgcolor: 'rgba(0,0,0,0)',
        font: { color: '#c9d1d9', family: 'Inter, sans-serif', size: 10 },
        margin: { l: 45, r: 15, t: 10, b: 35 },
        xaxis: { gridcolor: '#30363d', zerolinecolor: '#30363d', tickfont: { size: 9 } },
        yaxis: { gridcolor: '#30363d', zerolinecolor: '#30363d', tickfont: { size: 9 } }
    };

    let lastDashboardData = null;
    let currentSessions = [];

    // --- EVENT LISTENERS ---
    filterType.addEventListener('change', () => {
        console.log("[Filter] Type changed to:", filterType.value);
        updateFilterValues();
        renderTrajectories();
    });

    filterValue.addEventListener('change', () => {
        console.log("[Filter] Value changed to:", filterValue.value);
        renderTrajectories();
    });

    btnRefresh.addEventListener('click', loadDashboardData);
    selectSession.addEventListener('change', loadDashboardData);

    // --- LOGIC ---
    function updateFilterValues() {
        const type = filterType.value;
        const currentSelected = filterValue.value; // Save current selection

        if (type === 'global' || !lastDashboardData || !lastDashboardData.filter_data) {
            filterValue.innerHTML = '<option value="">Select ID</option>';
            return;
        }

        const data = lastDashboardData.filter_data;
        const ids = type === 'mission' ? data.missions : data.matrices;
        
        filterValue.innerHTML = ''; 
        
        if (ids && Array.isArray(ids) && ids.length > 0) {
            const def = document.createElement('option');
            def.value = ""; def.textContent = "Select ID";
            filterValue.appendChild(def);

            ids.forEach(id => {
                const opt = document.createElement('option');
                opt.value = id;
                opt.textContent = `${type.toUpperCase()} #${id}`;
                filterValue.appendChild(opt);
            });

            // Restore selection if the ID still exists in the new list
            if (currentSelected && ids.includes(parseInt(currentSelected))) {
                filterValue.value = currentSelected;
            }
        } else {
            filterValue.innerHTML = '<option value="">No data</option>';
        }
    }

    function renderTrajectories() {
        if (!lastDashboardData || !lastDashboardData.trajectories) return;

        const type = filterType.value;
        const targetId = parseInt(filterValue.value);
        let filtered = [];

        if (type === 'mission' && !isNaN(targetId)) {
            filtered = lastDashboardData.trajectories.filter(t => t.mission_id === targetId);
        } else if (type === 'matrix' && !isNaN(targetId)) {
            filtered = lastDashboardData.trajectories.filter(t => t.id === targetId);
        } else {
            // Global view: limit to first 30
            filtered = lastDashboardData.trajectories.slice(0, 30);
        }

        const trajTraces = filtered.map(t => ({
            x: t.x, y: t.y, mode: 'lines', name: `ID:${t.id} (M:${t.mission_id})`,
            line: { width: 1.5 },
            opacity: 0.8
        }));

        Plotly.react('plot-traj', trajTraces, {
            ...darkLayout, 
            showlegend: filtered.length > 0 && filtered.length < 12,
            xaxis: { ...darkLayout.xaxis, title: 'World X' },
            yaxis: { ...darkLayout.yaxis, title: 'World Y', scaleanchor: "x", scaleratio: 1 }
        }, {responsive: true});
    }

    async function loadSessions() {
        try {
            const res = await fetch('/api/telemetry/sessions');
            const sessions = await res.json();
            if (JSON.stringify(sessions) !== JSON.stringify(currentSessions)) {
                currentSessions = sessions;
                selectSession.innerHTML = '';
                sessions.forEach(s => {
                    const opt = document.createElement('option');
                    opt.value = s; opt.textContent = s;
                    selectSession.appendChild(opt);
                });
                if (sessions.length > 0) {
                    selectSession.value = sessions[0];
                    loadDashboardData();
                }
                return true;
            }
        } catch (e) { console.error("Session load error:", e); }
        return false;
    }

    async function loadDashboardData() {
        const session = selectSession.value;
        if (!session) return;
        
        let url = `/api/telemetry/data?session=${session}`;
        if (filterType.value === 'mission' && filterValue.value) url += `&mission_id=${filterValue.value}`;
        if (filterType.value === 'matrix' && filterValue.value) url += `&matrix_id=${filterValue.value}`;
        
        try {
            const res = await fetch(url);
            const data = await res.json();
            if (data.error) return;

            lastDashboardData = data;
            
            if (filterValue.options.length <= 1 || filterType.value !== 'global') {
                updateFilterValues();
            }
            
            document.getElementById('m-duration').textContent = `${data.metrics.duration.toFixed(1)} s`;
            document.getElementById('m-swarm').textContent = `${data.metrics.drones} agents`;
            if (document.getElementById('m-matrices')) {
                document.getElementById('m-matrices').textContent = `${data.metrics.matrices} matrices`;
            }
            document.getElementById('m-velocity').textContent = `${data.metrics.peak_velocity.toFixed(1)} px/s`;
            document.getElementById('m-battery').textContent = `${data.metrics.avg_battery.toFixed(1)}%`;
            
            renderTrajectories();

            // --- 2x2 AI Intelligence Grid ---
            const aiConf = { 
                ...darkLayout, 
                margin: {l:10, r:10, t:10, b:10}, 
                showlegend: false,
                autosize: true
            };
            
            const aiOptions = { responsive: true, displayModeBar: false };

            // 1. Drone States
            Plotly.react('plot-drone-states', [{
                values: data.ai_dist.drone_states.values, labels: data.ai_dist.drone_states.labels, 
                type: 'pie', hole: 0.4, marker: { colors: ['#58a6ff', '#3fb950', '#f85149', '#dbab09'] }
            }], aiConf, aiOptions);

            // 2. Drone Actions
            Plotly.react('plot-drone-actions', [{
                values: data.ai_dist.drone_actions.values, labels: data.ai_dist.drone_actions.labels, 
                type: 'pie', hole: 0.4, marker: { colors: ['#bc8cff', '#1f6feb', '#238636', '#9e6a03'] }
            }], aiConf, aiOptions);

            // 3. Matrix States
            Plotly.react('plot-matrix-states', [{
                values: data.ai_dist.matrix_states.values, labels: data.ai_dist.matrix_states.labels, 
                type: 'pie', hole: 0.4
            }], aiConf, aiOptions);

            // 4. Matrix Actions
            Plotly.react('plot-matrix-actions', [{
                values: data.ai_dist.matrix_actions.values, labels: data.ai_dist.matrix_actions.labels, 
                type: 'pie', hole: 0.4
            }], aiConf, aiOptions);

            // --- Other Metrics ---
            Plotly.react('plot-pid', [{
                x: data.time_series.time, y: data.time_series.error, type: 'scatter', mode: 'lines', fill: 'tozeroy', line: {color:'#f85149'}
            }], { ...darkLayout, xaxis: {title:'Time (s)'}, yaxis: {title:'PID Error'} });

            Plotly.react('plot-power', [{
                x: data.time_series.time, y: data.time_series.battery, type: 'scatter', mode: 'lines', line: {color:'#3fb950'}
            }], { ...darkLayout, xaxis: {title:'Time (s)'}, yaxis: {title:'Battery %'} });

            Plotly.react('plot-actuator', [{
                x: data.actuator.time, y: data.actuator.thrust, mode: 'markers',
                marker: { color: data.actuator.angle, colorscale: 'RdYlGn', showscale: true, size: 4 }
            }], { ...darkLayout, xaxis: {title:'Time (s)'}, yaxis: {title:'Thrust'} });

            Plotly.react('plot-collision', [{
                x: data.time_series.time, y: data.time_series.repulsion, type: 'scatter', mode: 'lines', fill: 'tozeroy', line: {color:'#dbab09'}
            }], { ...darkLayout, xaxis: {title:'Time (s)'}, yaxis: {title:'Avoidance Stress (N)'} });

            Plotly.react('plot-kinematic', [
                { x: data.time_series.time, y: data.time_series.speed, type: 'scatter', mode: 'lines', name: 'Velocity', line: {color:'#58a6ff'} },
                { x: data.time_series.time, y: data.time_series.acceleration, type: 'scatter', mode: 'lines', name: 'Accel', yaxis: 'y2', line: {color:'#bc8cff', dash:'dot'} }
            ], {
                ...darkLayout, xaxis: {title:'Time (s)'}, yaxis: {title:'Vel (px/s)'}, yaxis2: {overlaying:'y', side:'right', title:'Acc (px/s²)'}
            });

        } catch (e) { console.error("Data load error:", e); }
    }

    // Init
    loadSessions();
    setInterval(async () => {
        if (document.getElementById('view-analytics').classList.contains('active')) {
            const changed = await loadSessions();
            if (!changed) loadDashboardData();
        }
    }, 5000);
});
