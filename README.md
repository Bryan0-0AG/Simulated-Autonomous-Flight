# ✈️ Simulated Autonomous Flight — Advanced Physics Engine

> A high-performance C++ physics simulation engine designed for autonomous flight. Currently implements robust **Multi-Body Dynamics**, **PID Control**, **Basic AI State Machines**, and **Real-Time Python Dashboards**.

---

## 📋 Current Status

**Autonomous Fleet & Real-Time Analytics Completed.** The simulation features independent drones governed by individual PID controllers and an AI State Machine (managing battery and flight modes). Telemetry is streamed in real-time to an advanced Streamlit dashboard that calculates aggregated metrics (RMSE, Battery Drop, Velocities) while the C++ engine runs the physics.

---

## ✨ Key Features

- **Autonomous PID Control**: Individual Proportional-Integral-Derivative (PID) controllers for precise navigation.
- **AI State Machine**: Basic autonomy allowing drones to switch between states (e.g., `FLYING`, `CHARGING`) based on dynamic battery consumption.
- **Multi-Body Dynamics**: Simultaneous simulation of multiple independent physical entities interacting with the environment.
- **Advanced Telemetry System**: 
  - Streams categorized data (`Physics.csv`, `Control.csv`, `AI.csv`) to a live "Actual Simulation" folder.
  - Automatically archives logs with timestamps upon simulation closure.
- **Real-Time Streamlit Dashboard**: 
  - Python-based interactive analytics suite (`pandas`, `matplotlib`, `streamlit`).
  - Auto-refreshing visualizers for Spatial Trajectories, Error Convergence (RMSE), Thrust profiles, and Battery lifecycles.
- **Force-Based Physics**: Implements Newton's Second Law ($F=ma$) using Semi-Implicit Euler integration.
- **Real-Time 2D Visualization**: Powered by **SFML**, featuring world-to-screen mapping.

---

## 📁 Project Structure

```
SimulatedAutonomousFlight/
├── include/
│   ├── AI/                 # State machines and decision logic
│   ├── physics/            # Motion and forces
│   ├── control/            # PID Controller logic
│   ├── environment/        # Collision headers
│   ├── rendering/          # SFML Visualization class
│   ├── telemetry/          # Logger system (Categorized I/O)
│   └── body.h              # Physical body definitions
├── src/
│   ├── main.cpp            # Core loop
│   ├── AI/                 # AI Implementation
│   ├── physics/            # Physics Engine
│   └── control/            # PID and actuators
├── telemetry/
│   ├── logs/               # Automated CSV simulation archives
│   ├── dashboard.py        # Streamlit Real-Time Dashboard
│   └── run_sim.py          # Python orchestrator for C++ & Web Server
├── Makefile                # Build system (Auto-launches Python scripts)
└── README.md
```

---

## 🚀 Build and Run

### Requirements
- `g++` with C++17 support
- `make`
- **SFML 3.x** library
- **Python 3.x** with `streamlit`, `pandas`, `numpy`, and `matplotlib`

```bash
# Install Python dependencies
py -m pip install streamlit pandas numpy matplotlib

# Build C++ project and automatically launch the Dashboard & Simulation
make run
```

---

## 🗺️ Roadmap

| Phase | Focus | Status |
|---|---|---|
| 1 – Base | URM, vehicle struct, simulation loop | ✅ Done |
| 2 – Physics | Acceleration, forces, Euler integrator, Friction/Drag | ✅ Done |
| 3 – Environment | World bounds, collision detection | ✅ Done |
| 4 – Control | PID controller, navigation, Thrust system | ✅ Done |
| 5 – AI | State Machine (Battery management, behavior states) | ✅ Done |
| 6 – Visualization | Multi-Body SFML Renderer | ✅ Done |
| 7 – Analytics | Real-Time Telemetry & Streamlit Dashboard | ✅ Done |

### 🚀 Phase 8: AMD Virtual GPU Scaling Initiative (Upcoming)
*The next massive step to demonstrate true compute capability.*

| Milestone | Objective | Target |
|---|---|---|
| **Swarm Instancing** | Refactor `SFML` rendering from singular `Draw()` calls to `sf::VertexArray` Batch Rendering. | Render **10,000+** drones simultaneously without CPU bottleneck. |
| **GPU Compute Shaders** | Implement GLSL Fragment Shaders (`sf::Shader`) to calculate dynamic environmental factors. | Offload interactive fluid dynamics (wind/gravity fields) entirely to the AMD GPU. |
| **Telemetry Optimization** | Restructure disk I/O to handle massive swarm data without freezing the simulation. | Maintain 60 FPS while managing analytics for thousands of agents. |

---

## 🛠️ Technologies

- **C++17** — Engine core and real-time computation.
- **SFML 3.0** — Real-time graphics and GPU shader interface.
- **Python 3 (Streamlit/Pandas)** — Real-time interactive dashboard.
- **Make** — Build automation.

---

## 📄 License

MIT — free to use and modify.

> Developed by [Bryan0-0AG](https://github.com/Bryan0-0AG)
