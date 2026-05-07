# Simulated-Autonomous-Flight 🚀
An advanced multi-agent autonomous drone swarm simulator. Built with C++, SFML, and powered by **AMD ROCm/HIP** for massive scale and LLM-driven strategic orchestration.

> [!IMPORTANT]
> **Hackathon Update**: This repository is currently in active development for the AMD Developer Hackathon. We are focusing on massive scalability using GPU Computing and Heterogeneous High-Performance Computing (HPC).

## 🌟 Current Status
- **AMD HPC Integration (Phase 3 Alpha)**: Core physics engine fully migrated to **AMD HIP Kernels**.
- **Intelligent Heterogeneous Computing**: Automatic **CPU/GPU Fallback** system. The simulation detects available hardware at runtime and switches between HIP Kernels and high-performance CPU execution using the same mathematical logic (`__host__ __device__`).
- **Unified Data Architecture**: Implementation of the `DroneChassis` high-performance structure, optimized for zero-copy memory alignment between host and device.
- **Agentic Orchestration**: Integrated **Google Gemini 1.5 Flash** as the global strategic commander, processing swarm-wide telemetry in real-time.
- **Real-Time Visualization**: GPU-accelerated batch rendering and a dynamic **Streamlit Dashboard** for deep-dive telemetry analysis.

---

## 🧠 Swarm Orchestration Engine
The simulation now implements a **Hierarchical Swarm Logic**:
- **Massive Deployment (5,000+ Agents)**: Time-sliced batch spawning across a procedural city, preventing frame-time spikes and ensuring a visual "logistics" effect.
- **Matrix State Machine**: Autonomous groups (`MatrixGroup`) manage their own lifecycle through states: `TAKEOFF` → `STABILIZATION` → `MISSION_ACTIVE`.
- **Memory-Safe Architecture**: Re-engineered telemetry system using **Persistent Drone IDs** instead of memory pointers, enabling stable performance even during massive vector reallocations.
- **Procedural Urban Generation**: Configurable building distributions (Spawn, Charger, Collection, Obstacle) through a centralized `global_config.h`.

## 🏗️ Brain-Body Architecture
The system uses a **Decoupled Orchestration** model:
1. **The Body (C++ / AMD HIP)**: Massive physics calculation, collision avoidance (Spatial Grid), and PID flight control.
2. **The Bridge (SFML Network)**: High-speed TCP bridge for C++/Python interop.
3. **The Brain (Python / Gemini)**: High-level reasoning, mission strategy, and tactical command issuance.

---

## 🗺️ Hackathon Roadmap

### Phase 1: Foundations (DONE ✅)
- [x] Modular C++ engine and basic flight state machine.
- [x] Spatial Grid for efficient neighbor detection.

### Phase 2: Agentic Commander (DONE ✅)
- [x] Integration with Gemini 1.5 for strategic autonomy.
- [x] Professional telemetry dashboard (Streamlit + Plotly).

### Phase 3: AMD HPC Leap (IN PROGRESS 🚀)
- [x] **HIP Migration**: Porting core physics to `.hip` kernels.
- [x] **Swarm Orchestration**: Hierarchical AI for 5,000+ agents.
- [x] **Dynamic Deployment**: Building-aware spawning system.
- [x] **Memory Stability**: ID-based persistent telemetry.
- [ ] **GPU-Graphics Interop**: Zero-copy textures between ROCm and SFML.
- [ ] **Massive Scaling**: Stress testing towards 100,000 agents.

---

## 🛠️ Requirements
- C++17 / HIP Compiler (`hipcc` or `g++`)
- **AMD ROCm 6.0+** (For GPU acceleration)
- **SFML 3.0+**
- **Python 3.10+** (`google-generativeai`, `streamlit`, `pandas`)

## 🚀 Quick Start
1. **API Security**: Create a `.env` file in the root and add `GEMINI_API_KEY=your_key_here`.
2. **Build**: Run `make` to compile the system.
3. **Simulate**: `make run` launches the simulation, the AI brain, and the dashboard simultaneously.
