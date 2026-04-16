# ✈️ Simulated Autonomous Flight — MRU Physics

> A C++ physics simulation project. Currently implements **Uniform Rectilinear Motion (URM)** as the foundational module of a broader autonomous flight simulator.

---

## 📋 Current status

Early development — Phase 1 complete. The simulation runs a basic **URM loop** in console.

---

## 📁 Project structure

```
SimulatedAutonomousFlight/
├── include/
│   └── physics/
│       └── motion.h
├── src/
│   ├── main.cpp
│   └── physics/
│       └── motion.cpp
├── Makefile
└── README.md
```

---

## 🚀 Build and run

### Requirements
- `g++` with C++17 support
- `make`

```bash
make       # build
make run   # run simulation
make clean # remove binary
```

---

## 🗺️ Roadmap

| Phase | Focus | Status |
|---|---|---|
| 1 – Base | URM, vehicle struct, simulation loop | ✅ Done |
| 2 – Physics | Acceleration, forces, Euler integrator | 🔜 Next |
| 3 – Environment | World bounds, obstacles, collision detection | ⬜ |
| 4 – Control | PID controller, point navigation | ⬜ |
| 5 – AI | Pathfinding (A*), obstacle avoidance | ⬜ |
| 6 – Visualization | OpenGL — drone, obstacles, trajectory | ⬜ |
| 7 – Polish | Metrics, comparisons, demo video | ⬜ |

---

## 🛠️ Technologies

- **C++17** — g++ / Make
- **OpenGL** — planned for Phase 6

---

## 📄 License

MIT — free to use and modify.

> Developed by [Bryan0-0AG](https://github.com/Bryan0-0AG)
