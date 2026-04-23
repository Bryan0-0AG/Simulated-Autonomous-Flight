# Simulated-Autonomous-Flight 🚀
An advanced multi-agent autonomous drone swarm simulator. Built with C++, SFML, and powered by AMD ROCm for massive scale and LLM-driven strategic orchestration.

## 🌟 Current Status
- **Agentic Orchestration**: Integrated **Gemini 1.5 Flash** as a high-level strategic commander.
- **Edge Intelligence Architecture**: High-speed C++ engine calculates O(N) statistical telemetry, sending "Smart Packets" to the Python Brain.
- **Real-Time Visualization**: GPU-accelerated rendering and a live Streamlit dashboard for telemetry analysis.
- **Scalable Physics**: Autonomous state machines (PID, Battery, Missions) managing 5,000+ active agents.

---

## 🧠 Brain-Body Architecture
The system uses a **Decoupled Orchestration** model:
1. **The Body (C++ Engine)**: Handles physics, collision avoidance (Spatial Grid), and real-time flight control.
2. **The Bridge (SFML Network)**: Low-latency TCP communication between C++ and Python.
3. **The Brain (Python Orchestrator)**: Consolidates metrics and queries the LLM Strategist every 10s to issue tactical commands.

---

## 🗺️ Master Roadmap: The Path to 100k+ Agents

### Phase 1: Urban Logistics & Swarm Intelligence (DONE ✅)
- [x] **Modular Architecture**: Decoupled systems.
- [x] **Formation Flight**: 2D Grid navigation and desync spawning.

### Phase 2: Agentic Commander (DONE ✅)
- [x] **LLM Strategic Orchestrator**: Integration with Google Gemini for real-time mission analysis.
- [x] **Statistical Telemetry**: Real-time calculation of battery status, mission progress, and swarm health.
- [x] **Unified Launcher**: Single-command orchestration via `make run`.

### Phase 3: AMD High-Performance Computing (HPC Leap - NEXT 🚀)
- [ ] **ROCm/HIP Integration**: Offloading core physics and swarm neighbor calculations to AMD Instinct GPU Kernels.
- [ ] **GPU-Graphics Interop**: Zero-copy memory sharing between ROCm and SFML/OpenGL for maximum throughput.
- [ ] **Mass Scale Optimization**: Reaching 100,000+ active agents at 60 FPS on AMD hardware.

### Phase 4: Production-Ready Vision (Multimodal AI)
- [ ] **Virtual Vision Sensors**: Simulated drone cameras feeding into Vision AI models.
- [ ] **Deployment at Scale**: Packaging for AMD Cloud infrastructure (MI300X).

---

## 🛠️ Requirements
- C++17 Compiler (g++ suggested)
- SFML 3.0+
- Python 3.10+ (`google-generativeai`, `streamlit`)

## 🚀 Getting Started
1. **Setup API Key**: Export `GEMINI_API_KEY` in your environment.
2. **Compile**: `make`
3. **Launch All Systems**: `make run` (Starts AI Brain, Dashboard, and Simulation).
