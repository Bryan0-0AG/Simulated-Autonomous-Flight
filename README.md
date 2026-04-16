# ✈️ Simulated Autonomous Flight — Advanced Physics Engine

> A high-performance C++ physics simulation engine designed for autonomous flight. Currently implements robust **Euler Integration**, force systems, environment interaction, and **real-time 2D visualization**.

---

## 📋 Current Status

**Phase 6 (Visualization) Preview.** The simulation now features a real-time 2D window using **SFML**, allowing for visual debugging of physics, collisions, and movement.

---

## ✨ Key Features

- **Force-Based Physics**: Implements Newton's Second Law ($F=ma$) using Semi-Implicit Euler integration.
- **Real-Time 2D Visualization**: Powered by **SFML**, rendering physical bodies with customizable size and color in a dedicated window.
- **Environment Interaction**: Realistic ground detection handling object size and configurable `ENERGY_RESTITUTION` (bouncing).
- **Friction & Stability**:
  - **Air Friction (Drag)**: Linear air resistance opposing movement.
  - **Ground Friction**: Kinetic friction with robust clamping to prevent horizontal jitter.
  - **Snap to Zero**: Math utility to eliminate micro-movements and floating-point noise.
- **Coordinate Mapping**: World-to-Screen coordinate transformation (Y-axis inversion) for intuitive visualization.

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
│   │   └── math_utils.h      # Precision utilities (Snap to Zero, etc)
│   └── body.h                # Physical body definitions (color, size, etc)
├── src/
│   ├── main.cpp              # Real-time simulation loop
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
make run   # Run the simulation + 2D window
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
| 6 – Visualization | **SFML 2D Renderer**, trajectory logging | ✅ Done |
| 7 – Polish | Metrics, comparisons, demo video | ⬜ |

---

## 🛠️ Technologies

- **C++17** — Focus on efficiency and DOD (Data-Oriented Design)
- **SFML 3.0** — High-performance 2D multimedia framework
- **Make** — Cross-platform build automation

---

## 📄 License

MIT — free to use and modify.

> Developed by [Bryan0-0AG](https://github.com/Bryan0-0AG)
