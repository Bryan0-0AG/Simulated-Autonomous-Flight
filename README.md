# ✈️ Simulated Autonomous Flight — Advanced Swarm Engine

> A high-performance C++ engine for autonomous drone swarm simulation. Optimized for **10,000+ agents** using **Spatial Partitioning**, **Grid Formation Navigation**, and **Real-Time Interactive Telemetry**.

---

## 📋 Current Status

**Massive Swarm Scaling & High-Fidelity Analytics Completed.** 
The simulation is now capable of managing thousands of drones simultaneously. Key optimizations include an $O(N)$ Spatial Grid for neighbor detection and a deterministic Grid Formation system. Telemetry has been overhauled to a unified high-frequency stream analyzed by a premium Plotly-powered dashboard.

---

## ✨ Key Features

- **🚀 Swarm Scaling (Spatial Partitioning)**: Implements a `SpatialGrid` system that reduces neighbor lookup complexity from $O(N^2)$ to $O(N)$, enabling real-time simulation of massive fleets.
- **🏁 Grid Formation Navigation**: Automated aerial slot assignment using coordinate-based mapping (`getCenterByCoord`). Drones organize themselves into a perfect grid, eliminating spatial conflicts.
- **📦 Synchronized Batch Spawning**: Controlled deployment system that instantiates drones in batches every second, ensuring physics stability during takeoff.
- **🎮 Advanced PID Control**: Dual-axis PID controllers tuned for high-density environments, featuring "Hovering Intelligence" (compensating gravity $F_g = m \cdot g$).
- **📊 Interactive Plotly Dashboard**: 
  - **Force Analysis**: Real-time breakdown of Gravity vs. Thrust vs. Separation forces.
  - **Dynamic Visuals**: Zoomable trajectories and error convergence plots using `plotly.graph_objects`.
  - **KPI Tracking**: Automatic generation of `Summary_Stats.csv` for post-flight evaluation.
- **🧠 Autonomous State Machine**: Managed battery life cycles with Return-to-Base (RTB) logic and charging protocols.

---

## 📁 Project Structure

```
SimulatedAutonomousFlight/
├── include/
│   ├── utils/              # SpatialGrid, Vector math, and PID Utils
│   ├── physics/            # Force accumulation and motion integration
│   ├── control/            # PID Controller logic
│   ├── AI/                 # Mission state machines
│   ├── telemetry/          # Unified Full_Telemetry logger
│   └── body.h              # Physical body state & actuator data
├── src/
│   ├── main.cpp            # Main loop & Batch Spawning logic
│   ├── AI/                 # Autonomous decisions
│   ├── physics/            # Physics Engine implementation
│   └── control/            # PID and actuator response
├── telemetry/
│   ├── logs/               # Timestamped session archives
│   ├── dashboard.py        # Streamlit + Plotly Interactive Web App
│   └── run_sim.py          # Unified Python Orchestrator
├── Makefile                # Multi-platform build system
└── README.md
```

---

## 🚀 Build and Run

### Requirements
- `g++` (C++17)
- `make`
- **SFML 3.x**
- **Python 3.x** (`streamlit`, `plotly`, `pandas`, `numpy`)

```bash
# Install new visualization dependencies
pip install streamlit plotly pandas numpy

# Build and Launch the entire system (C++ Sim + Python Dashboard)
python telemetry/run_sim.py
```

---

## 🗺️ Roadmap

| Phase | Focus | Status |
|---|---|---|
| 1-7 | Core Physics, PID, AI, and Telemetry | ✅ Done |
| 8 | **Massive Scaling**: Spatial Partitioning & $O(N)$ Optimization | ✅ Done |
| 9 | **Grid Formation**: Deterministic slot assignment | ✅ Done |
| 10 | **Interactive Analytics**: Plotly Integration & Force Breakdown | ✅ Done |
| 11 | **GPU Acceleration**: Migrating render loop to `sf::VertexArray` | ⏳ In Progress |

---

## 🛠️ Technologies

- **C++17** — High-performance computation.
- **SFML 3.0** — Graphics and Windowing.
- **Streamlit & Plotly** — Premium telemetry dashboard.
- **Python 3** — Data analysis and system orchestration.

---

## 📄 License
MIT — free to use and modify.

> Developed by [Bryan0-0AG](https://github.com/Bryan0-0AG)
