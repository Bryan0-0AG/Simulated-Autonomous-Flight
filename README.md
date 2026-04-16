# ✈️ Simulated Autonomous Flight — Advanced Physics Engine

> A high-performance C++ physics simulation engine designed for autonomous flight. Currently implements robust **Euler Integration**, force systems, and environment interaction.

---

## 📋 Current Status

**Phase 2 & 3 Complete.** The simulation now handles advanced dynamics including gravity, air resistance, and ground interactions with high stability.

---

## ✨ Key Features

- **Force-Based Physics**: Implements Newton's Second Law ($F=ma$) using Semi-Implicit Euler integration.
- **Environment Interaction**: Realistic ground detection with configurable `ENERGY_RESTITUTION` (bouncing) and `COLLISION_EPSILON` for stability.
- **Friction System**:
  - **Air Friction (Drag)**: Linear air resistance opposing movement.
  - **Ground Friction**: Kinetic friction with robust clamping to prevent horizontal jitter.
- **Delta Time (DT) Control**: Fixed timestep simulation for consistent behavior across different hardware.

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
│   ├── utils/
│   │   └── vector2.h         # Basic 2D math structures
│   └── body.h                # Physical body definitions
├── src/
│   ├── main.cpp              # Simulation loop and entry point
│   ├── physics/
│   │   └── motion.cpp        # Force and integration implementation
│   ├── environment/
│   │   └── world.cpp         # Collision resolution logic
│   └── utils/
│       └── math_utils.cpp    # Auxiliary math functions
├── Makefile                  # Build system
└── README.md
```

---

## 🚀 Build and Run

### Requirements
- `g++` with C++17 support
- `make`

```bash
make       # Build the application
make run   # Run the simulation console logs
make clean # Remove binary files
```

---

## 🗺️ Roadmap

| Phase | Focus | Status |
|---|---|---|
| 1 – Base | URM, vehicle struct, simulation loop | ✅ Done |
| 2 – Physics | Acceleration, forces, Euler integrator, **Friction/Drag** | ✅ Done |
| 3 – Environment | World bounds, collision detection, **Stability Fixes** | ✅ Done |
| 4 – Control | PID controller, point navigation, **Thrust system** | 🔜 Next |
| 5 – AI | Pathfinding (A*), obstacle avoidance | ⬜ |
| 6 – Visualization | OpenGL — drone, obstacles, trajectory | ⬜ |
| 7 – Polish | Metrics, comparisons, demo video | ⬜ |

---

## 🛠️ Technologies

- **C++17** — Focus on efficiency and DOD (Data-Oriented Design)
- **Make** — Cross-platform build automation
- **OpenGL** — Planned for real-time visualization

---

## 📄 License

MIT — free to use and modify.

> Developed by [Bryan0-0AG](https://github.com/Bryan0-0AG)
