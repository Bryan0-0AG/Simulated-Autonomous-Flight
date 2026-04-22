# Simulated-Autonomous-Flight 🚀
An advanced multi-agent autonomous drone swarm simulator. Built with C++, SFML, and powered by AMD ROCm for massive scale and LLM-driven strategic orchestration.

## 🌟 Current Status
- **Modular Architecture**: Decoupled `world`, `camera`, and `renderer` systems.
- **High-Performance Rendering**: GPU-accelerated visualization via custom shaders.
- **Cinematic Experience**: Intelligent auto-zoom and interactive camera controls.
- **Physics Core**: Real-time PID control, battery management, and autonomous state machines.

---

## 🗺️ Master Roadmap: The Path to 100k+ Agents

### Phase 1: Urban Logistics & Swarm Intelligence
- [ ] **Procedural Environment**: Dynamic generation of city-scale obstacles and delivery packages.
- [ ] **Logistics Logic**: Implementation of "Pick-and-Drop" systems for mass cargo transport missions.
- [ ] **Proximity & Steering**: Raycasting-based obstacle avoidance and formation stability during high-speed transit.

### Phase 2: Agentic Commander (Brain-Body Integration)
- [ ] **Python-C++ Bridge**: High-speed real-time communication via ZeroMQ for telemetry and command sync.
- [ ] **LLM Strategic Orchestrator**: Integrating a Large Language Model (LLM) to analyze swarm telemetry and issue high-level tactical orders.
- [ ] **Dynamic Environment Physics**: Global wind systems, turbulence, and downwash effects.

### Phase 3: AMD High-Performance Computing (HPC Leap)
- [ ] **ROCm/HIP Integration**: Offloading core physics and swarm neighbor calculations to AMD Instinct GPU Kernels.
- [ ] **GPU-Graphics Interop**: Zero-copy memory sharing between ROCm and SFML/OpenGL for maximum throughput.
- [ ] **Mass Scale Optimization**: Reaching 100,000+ active agents at 60 FPS on AMD hardware.

### Phase 4: Production-Ready Vision (Multimodal AI)
- [ ] **Virtual Vision Sensors**: Simulated drone cameras feeding into Vision AI models for object detection and search-and-rescue tasks.
- [ ] **Deployment at Scale**: Packaging the system for AMD Cloud infrastructure (MI300X) as a production-ready simulation tool.

---

## 🛠️ Requirements
- C++17 Compiler (g++ suggested)
- SFML 3.0+
- Python 3.x (Real-time dashboard and Agentic Bridge)
- **AMD ROCm / HIP SDK** (For Phase 3/4)

## 🚀 Getting Started
1. Compile: `make`
2. Run simulation and dashboard: `make run`
