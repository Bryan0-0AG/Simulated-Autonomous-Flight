# Simulated-Autonomous-Flight 🚀
An advanced autonomous drone swarm simulator developed in C++ using SFML and GPU-accelerated rendering.

## 🌟 Current Status
- **Modular Architecture**: Fully decoupled `world` and `camera` systems for independent environment and viewport management.
- **Efficient Rendering**: High-performance rendering using `VertexArrays` and custom Shaders to handle thousands of agents simultaneously.
- **Cinematic Camera**: Intelligent auto-zoom system that keeps the entire swarm in focus, with full manual interactivity (Scroll to zoom, Right-click to pan).
- **Virtual Environment**: Infinite world ground with a tech-inspired neon horizon and a standardized physical coordinate system.

---

## 🗺️ Roadmap: Scaling to Mega-Swarms (10k - 100k agents)

### Phase 1: Dynamic Environment & Obstacles (Next Steps)
- [ ] **Procedural Generation**: Automatic creation of buildings, towers, and obstacles based on `WORLD_SIZE`.
- [ ] **Proximity Sensors**: Raycasting implementation for drones to sense the environment in real-time.
- [ ] **Steering Behaviors**: Smooth obstacle avoidance logic to navigate complex structures without breaking formation.

### Phase 2: Advanced Environmental Physics
- [ ] **Dynamic Force Fields**: Global wind systems and local turbulence using Perlin noise.
- [ ] **Downwash Effect**: Physics-based interaction where drones affect the stability of those flying directly beneath them.
- [ ] **Energy Management 2.0**: Physical charging stations requiring drones to queue and land autonomously.

### Phase 3: Massive Optimization (The 100k Leap)
- [ ] **GPU Compute Shaders**: Migrating physics logic (Euler/Verlet integration) from CPU to GPU.
- [ ] **Spatial Partitioning (3D Grid)**: Optimizing `SpatialGrid` for millisecond neighbor searches among >50,000 agents.
- [ ] **Instanced Rendering**: Minimizing draw calls to maintain 60 FPS with 100,000 active agents.

---

## 🛠️ Requirements
- C++17 Compiler (g++ suggested)
- SFML 3.0+
- Python 3.x (for the telemetry dashboard)

## 🚀 Getting Started
1. Compile: `make`
2. Run simulation and dashboard: `make run`
