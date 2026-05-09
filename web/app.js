document.addEventListener('DOMContentLoaded', () => {
    // Canvas Setup
    const canvas = document.getElementById('swarmCanvas');
    const ctx = canvas.getContext('2d');
    
    // Navigation Logic
    const navItems = document.querySelectorAll('.nav-item');
    const views = document.querySelectorAll('.view-section');
    const sidebarControls = document.getElementById('sidebar-controls');

    navItems.forEach(item => {
        item.addEventListener('click', (e) => {
            e.preventDefault();
            
            // Handle Slide Panel for Server Management
            if (item.id === 'nav-server-manage') {
                document.getElementById('slide-server').classList.add('open');
                return;
            }

            navItems.forEach(n => n.classList.remove('active'));
            views.forEach(v => v.classList.remove('active'));
            item.classList.add('active');
            
            const targetId = item.getAttribute('data-target');
            if (targetId) {
                document.getElementById(targetId).classList.add('active');
                if(targetId === 'view-command') {
                    sidebarControls.style.display = 'block';
                    setTimeout(resizeCanvas, 10);
                } else {
                    sidebarControls.style.display = 'none';
                }
            }
        });
    });

    // Close Slide Panel
    document.getElementById('btn-close-server').addEventListener('click', () => {
        document.getElementById('slide-server').classList.remove('open');
    });

    // Resize canvas to fit container
    function resizeCanvas() {
        const container = canvas.parentElement;
        canvas.width = container.clientWidth;
        canvas.height = container.clientHeight;
    }
    window.addEventListener('resize', () => {
        resizeCanvas();
        if (typeof clampCamera === 'function') clampCamera();
    });
    resizeCanvas();

    // ================================================================
    // CAMERA (Pan + Zoom) - Mirrors C++ camera.cpp behavior
    // ================================================================

    // World bounds - defaults matching global_config.h, updated dynamically from /api/config
    let WORLD_W = 100000;
    let WORLD_H = 100000;
    let GROUND_MARGIN = 5000;

    // Load config from C++ global_config.h via backend
    fetch('/api/config').then(r => r.json()).then(cfg => {
        if (cfg.WORLD_SIZE) {
            WORLD_W = cfg.WORLD_SIZE[0];
            WORLD_H = cfg.WORLD_SIZE[1];
            GROUND_MARGIN = WORLD_H * 0.05;
            camera.x = WORLD_W / 2;
            console.log(`[Config] World loaded: ${WORLD_W} x ${WORLD_H}`);
            clampCamera();
        }
        // Store full config globally for other panels
        window.SIM_CONFIG = cfg;
    }).catch(() => console.warn('[Config] Could not load, using defaults'));

    const camera = {
        x: WORLD_W / 2, // Start centered horizontally
        y: 1500,         // Start above ground level
        zoom: 0.15,      // Will be adjusted by clampCamera on first frame
        minZoom: 0.015,
        maxZoom: 2.0
    };

    // Ensure initial zoom respects world limits (called after getMinZoom is defined)
    setTimeout(() => { clampCamera(); }, 0);

    let isDragging = false;
    let dragStartX = 0, dragStartY = 0;
    let camStartX = 0, camStartY = 0;

    // Disable right-click context menu on canvas
    canvas.addEventListener('contextmenu', (e) => e.preventDefault());

    // Right-click to drag (like C++ SFML camera)
    canvas.addEventListener('mousedown', (e) => {
        if (e.button === 2 || e.button === 0) { // Right or Left click
            isDragging = true;
            dragStartX = e.clientX;
            dragStartY = e.clientY;
            camStartX = camera.x;
            camStartY = camera.y;
            canvas.style.cursor = 'grabbing';
        }
    });

    // Coordinates overlay element
    const coordsEl = document.querySelector('.canvas-overlay .coords');

    canvas.addEventListener('mousemove', (e) => {
        // Get mouse position relative to canvas
        const rect = canvas.getBoundingClientRect();
        const mx = e.clientX - rect.left;
        const my = e.clientY - rect.top;

        // Convert to world coordinates and update overlay
        const [wx, wy] = screenToWorld(mx, my);
        const [lat, lng] = worldToGeo(wx, wy);
        if (coordsEl) {
            const latDir = lat >= 0 ? 'N' : 'S';
            const lngDir = lng >= 0 ? 'E' : 'W';
            coordsEl.textContent = `LAT: ${Math.abs(lat).toFixed(4)}\u00B0 ${latDir} | LNG: ${Math.abs(lng).toFixed(4)}\u00B0 ${lngDir} | X: ${wx.toFixed(0)}m | Y: ${wy.toFixed(0)}m`;
        }

        // Camera drag
        if (!isDragging) return;
        const dx = (e.clientX - dragStartX) / camera.zoom;
        const dy = (e.clientY - dragStartY) / camera.zoom;
        camera.x = camStartX - dx;
        camera.y = camStartY + dy;
        clampCamera();
    });

    canvas.addEventListener('mouseup', () => {
        isDragging = false;
        canvas.style.cursor = 'grab';
    });

    canvas.addEventListener('mouseleave', () => {
        isDragging = false;
        canvas.style.cursor = 'grab';
    });

    canvas.addEventListener('wheel', (e) => {
        e.preventDefault();
        const zoomFactor = e.deltaY < 0 ? 1.15 : 0.87;
        camera.zoom = Math.max(getMinZoom(), Math.min(camera.maxZoom, camera.zoom * zoomFactor));
        clampCamera();
    }, { passive: false });

    canvas.style.cursor = 'grab';

    // Calculate minimum zoom so the view never exceeds the world
    function getMinZoom() {
        const zoomX = canvas.width / WORLD_W;
        const zoomY = canvas.height / (WORLD_H + GROUND_MARGIN);
        return Math.max(zoomX, zoomY);
    }

    // Clamp so viewport edges never reveal outside the world
    function clampCamera() {
        // Enforce minimum zoom
        camera.zoom = Math.max(getMinZoom(), camera.zoom);

        const halfW = (canvas.width / 2) / camera.zoom;
        const halfH = (canvas.height / 2) / camera.zoom;

        // Left edge >= 0, Right edge <= WORLD_W
        camera.x = Math.max(halfW, Math.min(WORLD_W - halfW, camera.x));

        // Bottom edge >= -GROUND_MARGIN, Top edge <= WORLD_H
        camera.y = Math.max(-GROUND_MARGIN + halfH, Math.min(WORLD_H - halfH, camera.y));
    }

    // Convert world coordinates to screen pixels
    function worldToScreen(wx, wy) {
        const sx = (wx - camera.x) * camera.zoom + canvas.width / 2;
        const sy = canvas.height / 2 - (wy - camera.y) * camera.zoom;
        return [sx, sy];
    }

    // Convert screen pixels to world coordinates
    function screenToWorld(sx, sy) {
        const wx = (sx - canvas.width / 2) / camera.zoom + camera.x;
        const wy = camera.y - (sy - canvas.height / 2) / camera.zoom;
        return [wx, wy];
    }

    // Map world coordinates to geographic coordinates (fictional but consistent)
    // World [0, 100000] x [0, 100000] -> ~100km area around Los Angeles
    const GEO_BASE_LAT = 34.0200;   // Bottom of world
    const GEO_BASE_LNG = -118.3000; // Left of world
    const GEO_SCALE_LAT = 0.9 / WORLD_H;  // ~100km vertical
    const GEO_SCALE_LNG = 1.2 / WORLD_W;  // ~100km horizontal

    function worldToGeo(wx, wy) {
        const lat = GEO_BASE_LAT + wy * GEO_SCALE_LAT;
        const lng = GEO_BASE_LNG + wx * GEO_SCALE_LNG;
        return [lat, lng];
    }

    // ================================================================
    // STATE
    // ================================================================
    let drones = [];
    let fakeDrones = [];
    let buildings = [];  // Will be populated from backend

    // ================================================================
    // ANIMATION LOOP
    // ================================================================
    let lastTime = 0;
    function animate(timestamp) {
        const dt = (timestamp - lastTime) / 1000;
        lastTime = timestamp;

        // Clear
        ctx.fillStyle = '#050508';
        ctx.fillRect(0, 0, canvas.width, canvas.height);

        // Draw world elements
        drawGrid();
        drawGround();
        drawBuildings();

        // Render drones
        if (drones.length > 0) {
            renderDrones(drones);
        } else {
            updateFakeDrones(dt);
            renderDrones(fakeDrones);
        }

        // HUD
        document.getElementById('stat-drones').innerText = drones.length || fakeDrones.length;

        requestAnimationFrame(animate);
    }

    // ================================================================
    // RENDERING
    // ================================================================
    function drawGrid() {
        ctx.strokeStyle = 'rgba(255, 255, 255, 0.04)';
        ctx.lineWidth = 1;
        // Adaptive grid based on zoom
        let gridSize = 500;
        if (camera.zoom > 0.2) gridSize = 100;
        if (camera.zoom > 0.5) gridSize = 50;

        // Calculate visible world range
        const halfW = (canvas.width / 2) / camera.zoom;
        const halfH = (canvas.height / 2) / camera.zoom;
        const xMin = camera.x - halfW;
        const xMax = camera.x + halfW;
        const yMin = camera.y - halfH;
        const yMax = camera.y + halfH;

        const startX = Math.floor(xMin / gridSize) * gridSize;
        const startY = Math.floor(yMin / gridSize) * gridSize;

        for (let wx = startX; wx <= xMax; wx += gridSize) {
            const [sx] = worldToScreen(wx, 0);
            ctx.beginPath();
            ctx.moveTo(sx, 0);
            ctx.lineTo(sx, canvas.height);
            ctx.stroke();
        }
        for (let wy = startY; wy <= yMax; wy += gridSize) {
            const [, sy] = worldToScreen(0, wy);
            ctx.beginPath();
            ctx.moveTo(0, sy);
            ctx.lineTo(canvas.width, sy);
            ctx.stroke();
        }
    }

    function drawGround() {
        const [leftX, groundY] = worldToScreen(0, 0);
        const [rightX] = worldToScreen(WORLD_W, 0);
        const groundWidth = rightX - leftX;
        const groundDepth = canvas.height - groundY;

        if (groundDepth <= 0) return; // Ground is above viewport

        // Deep earth base layer
        const earthGrad = ctx.createLinearGradient(0, groundY, 0, groundY + groundDepth);
        earthGrad.addColorStop(0, '#2d1f0e');    // Dark topsoil
        earthGrad.addColorStop(0.15, '#3a2814');  // Rich brown
        earthGrad.addColorStop(0.4, '#2a1c0a');   // Deep earth
        earthGrad.addColorStop(0.7, '#1a120a');    // Darker rock
        earthGrad.addColorStop(1, '#0d0806');      // Bedrock
        ctx.fillStyle = earthGrad;
        ctx.fillRect(leftX, groundY, groundWidth, groundDepth);

        // Surface grass strip (bright green band)
        const grassHeight = Math.max(4, 30 * camera.zoom);
        const grassGrad = ctx.createLinearGradient(0, groundY - 1, 0, groundY + grassHeight);
        grassGrad.addColorStop(0, '#5cb338');      // Bright grass green
        grassGrad.addColorStop(0.3, '#3d8c2a');    // Medium green
        grassGrad.addColorStop(1, '#2d1f0e');      // Blends into soil
        ctx.fillStyle = grassGrad;
        ctx.fillRect(leftX, groundY, groundWidth, grassHeight);

        // Grass surface highlight line
        ctx.strokeStyle = '#6dd844';
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.moveTo(leftX, groundY);
        ctx.lineTo(rightX, groundY);
        ctx.stroke();

        // Subtle horizontal soil layers (geological strata)
        ctx.strokeStyle = 'rgba(60, 40, 20, 0.3)';
        ctx.lineWidth = 1;
        const strataSpacing = Math.max(20, 150 * camera.zoom);
        for (let dy = strataSpacing * 3; dy < groundDepth; dy += strataSpacing) {
            ctx.beginPath();
            ctx.moveTo(leftX, groundY + dy);
            ctx.lineTo(rightX, groundY + dy);
            ctx.stroke();
        }
    }

    function drawBuildings() {
        const typeColors = {
            0: 'rgba(80, 80, 100, 0.6)',   // OBSTACLE - gray
            1: 'rgba(0, 120, 255, 0.6)',    // CHARGER - blue
            2: 'rgba(220, 50, 180, 0.6)',   // COLLECTION - pink
            3: 'rgba(140, 50, 220, 0.6)',   // DEPLOY - purple
            4: 'rgba(220, 200, 50, 0.6)',   // SPAWN - yellow
        };
        const typeBorders = {
            0: 'rgba(120, 120, 140, 0.8)',
            1: 'rgba(0, 160, 255, 0.9)',
            2: 'rgba(255, 80, 200, 0.9)',
            3: 'rgba(180, 80, 255, 0.9)',
            4: 'rgba(255, 240, 80, 0.9)',
        };

        buildings.forEach(b => {
            const [sx, sy] = worldToScreen(b.x, b.y + b.h);
            const sw = b.w * camera.zoom;
            const sh = b.h * camera.zoom;

            ctx.fillStyle = typeColors[b.type] || typeColors[0];
            ctx.fillRect(sx, sy, sw, sh);

            ctx.strokeStyle = typeBorders[b.type] || typeBorders[0];
            ctx.lineWidth = 1;
            ctx.strokeRect(sx, sy, sw, sh);
        });
    }

    function renderDrones(droneList) {
        // Determine dot size based on zoom
        const dotSize = Math.max(1.5, Math.min(4, camera.zoom * 20));

        droneList.forEach(drone => {
            const [sx, sy] = worldToScreen(drone.x, drone.y);

            // Skip if offscreen
            if (sx < -10 || sx > canvas.width + 10 || sy < -10 || sy > canvas.height + 10) return;

            // Core dot only (no glow to keep formations crisp)
            ctx.beginPath();
            ctx.arc(sx, sy, dotSize, 0, Math.PI * 2);
            ctx.fillStyle = '#00d2ff';
            ctx.fill();
        });
    }

    // ================================================================
    // FAKE DRONES (offline demo)
    // ================================================================
    function updateFakeDrones(dt) {
        fakeDrones.forEach(d => {
            d.x += d.vx * dt;
            d.y += d.vy * dt;
            if (d.x < 0 || d.x > WORLD_W) d.vx *= -1;
            if (d.y < 0 || d.y > WORLD_H) d.vy *= -1;
        });
    }

    document.getElementById('btn-transport').addEventListener('click', () => {
        for(let i=0; i<50; i++) {
            fakeDrones.push({
                x: WORLD_W / 2,
                y: 100,
                vx: (Math.random() - 0.5) * 2000,
                vy: Math.random() * 2000 + 500
            });
        }
        document.getElementById('stat-missions').innerText = 1;
    });

    document.getElementById('btn-recall').addEventListener('click', () => {
        fakeDrones = [];
        drones = [];
        document.getElementById('stat-drones').innerText = 0;
        document.getElementById('stat-missions').innerText = 0;
    });

    // ================================================================
    // TELEMETRY WEBSOCKET (drone positions from C++)
    // ================================================================
    let telemetrySocket = null;
    function connectTelemetryWS() {
        telemetrySocket = new WebSocket(`ws://${window.location.host}/ws/telemetry`);
        telemetrySocket.onmessage = (event) => {
            try {
                const frame = JSON.parse(event.data);
                if (frame.type === 'frame' && frame.pos) {
                    drones = frame.pos.map(p => ({ x: p[0], y: p[1] }));
                } else if (frame.type === 'buildings' && frame.data) {
                    buildings = frame.data;
                }
            } catch(e) {
                console.error('Telemetry parse error:', e);
            }
        };
        telemetrySocket.onclose = () => {
            drones = [];
            setTimeout(connectTelemetryWS, 2000);
        };
    }
    connectTelemetryWS();

    // Also try to load buildings from REST endpoint
    fetch('/api/buildings').then(r => r.json()).then(data => {
        if (data && data.length > 0) buildings = data;
    }).catch(() => {});

    // ================================================================
    // CONSOLE WEBSOCKET (C++ stdout)
    // ================================================================
    const termOutput = document.getElementById('terminal-output');
    function printToTerminal(text, type='stdout') {
        const div = document.createElement('div');
        div.className = `term-line ${type}`;
        div.innerText = text.trimEnd();
        termOutput.appendChild(div);
        termOutput.scrollTop = termOutput.scrollHeight;
    }

    document.getElementById('btn-clear-term').addEventListener('click', () => {
        termOutput.innerHTML = '';
    });

    let consoleSocket = null;
    function connectConsoleWS() {
        consoleSocket = new WebSocket(`ws://${window.location.host}/ws/console`);
        consoleSocket.onmessage = (event) => {
            printToTerminal(event.data, 'stdout');
        };
        consoleSocket.onclose = () => {
            setTimeout(connectConsoleWS, 2000);
        };
    }
    connectConsoleWS();

    // ================================================================
    // ENGINE CONTROLS
    // ================================================================
    const btnBoot = document.getElementById('btn-boot-engine');
    const btnKill = document.getElementById('btn-kill-engine');
    const configInputs = document.querySelectorAll('#engine-config-form input');

    btnBoot.addEventListener('click', async () => {
        try {
            // Read config values to pass to backend (to be implemented on backend)
            const pidP = document.getElementById('cfg-pid-p').value;
            const drain = document.getElementById('cfg-battery').value;
            
            const res = await fetch('/api/engine/start', { method: 'POST' });
            const data = await res.json();
            
            const statusEl = document.getElementById('conn-status');
            statusEl.innerText = 'Online';
            statusEl.classList.remove('offline');
            statusEl.classList.add('online');
            
            const cmdStatus = document.getElementById('cmd-engine-status');
            if (cmdStatus) {
                cmdStatus.innerText = 'ACTIVE';
                cmdStatus.style.color = '#00ff80';
            }

            const gpuStatus = document.getElementById('cmd-gpu-status');
            if (gpuStatus) {
                gpuStatus.innerText = 'ROCm HYPER';
                gpuStatus.style.color = '#00ff80';
            }
            
            // Lock UI
            configInputs.forEach(input => input.disabled = true);
            btnBoot.disabled = true;
            btnKill.disabled = false;

            console.log("Server Started:", data);
        } catch (e) {
            console.error("Failed to start engine", e);
            alert("Could not reach the Python Orchestrator");
        }
    });


    btnKill.addEventListener('click', async () => {
        try {
            const res = await fetch('/api/engine/kill', { method: 'POST' });
            const data = await res.json();

            drones = [];
            buildings = [];
            const statusEl = document.getElementById('conn-status');
            statusEl.innerText = 'Offline';
            statusEl.classList.remove('online');
            statusEl.classList.add('offline');

            const cmdStatus = document.getElementById('cmd-engine-status');
            if (cmdStatus) {
                cmdStatus.innerText = 'INACTIVE';
                cmdStatus.style.color = '#ff4d4d';
            }

            const gpuStatus = document.getElementById('cmd-gpu-status');
            if (gpuStatus) {
                gpuStatus.innerText = 'IDLE';
                gpuStatus.style.color = 'var(--text-main)';
            }
            document.getElementById('stat-drones').innerText = 0;
            document.getElementById('stat-missions').innerText = 0;
            
            // Unlock UI
            const configInputs = document.querySelectorAll('#engine-config-form input');
            const btnBoot = document.getElementById('btn-boot-engine');
            configInputs.forEach(input => input.disabled = false);
            if (btnBoot) btnBoot.disabled = false;
            btnKill.disabled = true;

            console.log("Server Killed:", data);
        } catch (e) {
            console.error("Failed to kill engine", e);
        }
    });

    // Start loop
    requestAnimationFrame(animate);
});
