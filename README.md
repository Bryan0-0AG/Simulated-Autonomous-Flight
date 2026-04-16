# ✈️ Simulated Autonomous Flight — Advanced Physics Engine

> A high-performance C++ physics simulation engine designed for autonomous flight. Currently implements robust **Multi-Body Dynamics**, **Automated Telemetry**, and **Post-Processing Data Analysis**.

---

## 📋 Current Status

**Integrated Analytics System.** The simulation now creates persistent binary/text logs and includes a Python-powered suite for trajectory and performance visualization.

---

## ✨ Key Features

- **Multi-Body Dynamics**: Simultaneous simulation of multiple independent physical entities using `std::vector` and unique `id` tracking.
- **Automated Telemetry**: Integrated C++ `TelemetryLogger` that generates timestamped CSV logs for every simulation run.
- **Post-Processing Analytics**: Python-based diagnostic suite (`pandas` & `matplotlib`) to plot:
  - Multi-agent spatial trajectories (2D bird's-eye view).
  - Velocity components over time for performance profiling.
- **Force-Based Physics**: Implements Newton's Second Law ($F=ma$) using Semi-Implicit Euler integration.
- **Real-Time 2D Visualization**: Powered by **SFML**, featuring world-to-screen mapping and dynamic per-entity rendering.
- **Environment Interaction**: Realistic ground detection with configurable `ENERGY_RESTITUTION` and per-body grounding states.
- **Friction & Stability**:
  - **Air Friction (Drag)**: Linear air resistance opposing movement.
  - **Ground Friction**: Kinetic friction with robust `snap_zero` clamping.

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
│   ├── telemetry/
│   │   └── telemetry_logger.h # Persistent logging system
│   ├── utils/
│   │   ├── vector2.h         # Basic 2D math structures
│   │   └── math_utils.h      # Precision & Randomization utilities
│   └── body.h                # Physical body definitions
├── src/
│   ├── main.cpp              # Simulation loop & telemetry orchestration
│   ├── physics/
│   │   └── motion.cpp        # Newton/Euler implementation
│   └── ...                   # Other implementation files
├── telemetry/
│   ├── logs/                 # Folder for CSV simulation reports
│   └── data_analysis.py      # Python analysis & plotting script
├── Makefile                  # Build system (SFML Linking)
└── README.md
```

---

## 🚀 Build and Run

### Requirements
- `g++` with C++17 support
- `make`
- **SFML 3.x** library
- **Python 3.x** with `pandas` and `matplotlib` (for analytics)

```bash
# 1. Build and Run Simulation
make
make run

# 2. Analyze Latest Results
python telemetry/data_analysis.py
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
| 6 – Visualization | Multi-Body SFML Renderer | ✅ Done |
| 7 – Analytics | **Telemetry logs, Python analysis suite** | ✅ Done |

---

## 🛠️ Technologies

- **C++17** — Engine core and real-time computation.
- **SFML 3.0** — Real-time window and graphics.
- **Python 3 (Pandas/Matplotlib)** — Data post-processing and scientific plotting.
- **Make** — Build automation.

---

## 📄 License

MIT — free to use and modify.

> Developed by [Bryan0-0AG](https://github.com/Bryan0-0AG)
