# AMD SwarmOS: Autonomous Flight Swarm Simulator

![SwarmOS Header](https://img.shields.io/badge/Powered%20By-AMD%20ROCm-ED1C24?style=for-the-badge&logo=amd&logoColor=white)
![Status](https://img.shields.io/badge/Status-Experimental-blueviolet?style=for-the-badge)

**SwarmOS** is a high-performance autonomous drone swarm simulation engine designed to leverage the massive parallel computing power of **AMD ROCm™** and **MI300X** accelerators. It enables the orchestration of thousands of intelligent agents in a procedural city environment, featuring real-time telemetry, advanced AI matrix coordination, and cloud-to-edge connectivity.

## 🚀 Key Features

*   **GPU Accelerated Dynamics**: Core physics and flight dynamics calculated via **AMD HIP** kernels, allowing for stable simulation of 10,000+ drones at high frame rates.
*   **Intelligent Matrix Orchestration**: Drones are managed as collective "Matrix Groups" that coordinate behaviors like highway cruising, formation flight, and synchronized docking.
*   **Tactical Dashboard**: A modern web interface built with **FastAPI** and **Plotly**, providing real-time 2D visualization and deep analytical insights (PID performance, battery health, AI state distribution).
*   **Procedural Urban Environment**: Dynamic city generation including obstacles, charging stations, collection zones, and multi-lane aerial highways.
*   **Hybrid Cloud Execution**: Seamlessly switch between local development and remote high-performance droplets (AMD Cloud MI300X) with one-click code synchronization.

## 🛠 Technology Stack

*   **Backend Engine**: C++17, AMD ROCm/HIP, SFML (for networking/windowing).
*   **Web Orchestrator**: Python (FastAPI, Uvicorn, WebSockets).
*   **Frontend Dashboard**: Vanilla JS, Plotly.js, CSS Grid/Flexbox.
*   **Data Analysis**: Python (Pandas, NumPy, Matplotlib).

## 📂 Project Structure

```text
├── src/                    # C++ and HIP Source Code
│   ├── server/             # Simulation logic & Swarm Management
│   ├── client/             # C++ Visualizer (Optional Client)
│   ├── shared/             # Common world and network utilities
│   └── core/               # Entry points (main_server, main_client)
├── include/                # Header files and global configurations
├── web/                    # Dashboard UI and Python Backend Bridge
├── telemetry/              # Data processing and historical analytics
├── Makefile                # Build system (Windows/Linux compatible)
└── cloud_config.json       # Remote connection credentials
```

## 🚦 Getting Started

### Prerequisites
*   **Local**: Windows/Linux with G++ and SFML installed.
*   **AMD GPU**: ROCm 6.x+ installed for hardware acceleration.
*   **Python**: 3.9+ with `fastapi`, `uvicorn`, `pandas` installed.

### Installation
1. Clone the repository.
2. Install dependencies:
   ```bash
   pip install fastapi uvicorn pandas websockets
   ```
3. Compile the system:
   ```bash
   make all
   ```

### Running the System
1. Start the Orchestrator (Web Server):
   ```bash
   python web/connection/web_backend.py
   ```
2. Open `http://localhost:8000` in your browser.
3. Use the **Server Management** panel to "Boot Engine".

## ☁️ Cloud Deployment (AMD MI300X)

To run the simulation on a high-performance AMD ROCm Cloud droplet:
1. Enable **Cloud Mode** in the Server Management panel.
2. Configure your Droplet IP and SSH credentials.
3. Click **Sync Code to Droplet** to push your local changes.
4. Click **Boot Engine** to launch the remote ROCm process.

## 📊 Analytics & Telemetry

The system records full mission telemetry in CSV format. Use the **Fleet Intelligence** tab in the dashboard to:
*   Analyze PID controller stability and error margins.
*   Monitor swarm battery consumption trends.
*   Inspect AI state transitions (Searching, Cruising, Landing).
*   View historical trajectories of individual matrices.

---
*Built for the AMD ROCm Hackathon - Advancing Autonomous Swarm Intelligence.*
