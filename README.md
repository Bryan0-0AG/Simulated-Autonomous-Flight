# ✈️ Simulated Autonomous Flight — Advanced Physics Engine

> A high-performance C++ physics simulation engine designed for autonomous flight. Currently implements robust **Multi-Body Dynamics**, force systems, environment interaction, and **real-time 2D visualization**.

---

## 📋 Current Status

**Phase 6 (Visualization) Complete.** The simulation now supports **Multi-Body entities**, rendering multiple physical bodies simultaneously with randomized properties and real-time console telemetry.

---

## ✨ Key Features

- **Multi-Body Dynamics**: Simultaneous simulation of multiple independent physical entities using `std::vector` and unique `id` tracking.
- **Force-Based Physics**: Implements Newton's Second Law ($F=ma$) using Semi-Implicit Euler integration for every object in the scene.
- **Procedural Generation**: Integrated `randint` utility based on `std::mt19937` for randomized spawning (colors, impulses, sizes).
- **Real-Time 2D Visualization**: Powered by **SFML**, featuring:
  - Coordinate Mapping (World-to-Screen inversion).
  - Dynamic color and size rendering per entity.
  - High-precision frame synchronization.
- **Environment Interaction**: Realistic ground detection with configurable `ENERGY_RESTITUTION` and ground tracking per body.
- **Friction & Stability**:
  - **Air Friction (Drag)**: Linear air resistance opposing movement.
  - **Ground Friction**: Kinetic friction with robust clamping via `snap_zero` utility to prevent horizontal jitter.
- **Detailed Telemetry**: Console logging system providing per-second snapshots of every simulated body's position and velocity.

---

## 📁 Project Structure

```
SimulatedAutonomousFlight/
├── include/
│   ├── physics/
│   │   ├── motion.h          # Motion update declarations
│   │   └── physics_config.h  # Universal physics constants
│   ├── environment/
│   │   └── world.h           # Environment and collision headers
│   ├── rendering/
│   │   └── BasicRenderer.h   # SFML Visualization class
│   ├── utils/
│   │   ├── vector2.h         # Basic 2D math structures
│   │   └── math_utils.h      # Precision utilities (Snap to Zero, Randint)
│   └── body.h                # Multi-body physical definitions (color, id, grounded)
├── src/
│   ├── main.cpp              # Multi-body simulation loop & logging
│   ├── physics/
│   │   └── motion.cpp        # Force and integration implementation
│   ├── environment/
│   │   └── world.cpp         # Collision resolution logic
│   ├── rendering/
│   │   └── BasicRenderer.cpp # Rendering implementation
│   └── utils/
│       └── math_utils.cpp    # Auxiliary math implementations
├── Makefile                  # Build system (SFML Linking)
└── README.md
```

---

## 🚀 Build and Run

### Requirements
- `g++` with C++17 support
- `make`
- **SFML 3.x** library installed

```bash
make       # Build the application (links SFML)
make run   # Run the multi-body simulation
make clean # Remove binary files
```

---

## 🗺️ Roadmap

| Phase | Focus | Status |
|---|---|---|
| 1 – Base | URM, vehicle struct, simulation loop | ✅ Done |
| 2 – Physics | Acceleration, forces, Euler integrator, Friction/Drag | ✅ Done |
| 3 – Environment | World bounds, collision detection, Stability Fixes | ✅ Done |
| 4 – Control | PID controller, point navigation, **Thrust system** | 🔜 Next |
| 5 – AI | Pathfinding (A*), obstacle avoidance | ⬜ |
| 6 – Visualization | **Multi-Body SFML Renderer**, telemetry logs | ✅ Done |
| 7 – Polish | Metrics, comparisons, demo video | ⬜ |

---

## 🛠️ Technologies

- **C++17** — Focus on efficiency and DOD (Data-Oriented Design)
- **SFML 3.0** — High-performance 2D multimedia framework
- **MT19937** — Standard Mersenne Twister for high-quality random generation
- **Make** — Cross-platform build automation

---

## 📄 License

MIT — free to use and modify.

> Developed by [Bryan0-0AG](https://github.com/Bryan0-0AG)
