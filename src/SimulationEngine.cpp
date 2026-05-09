#include "SimulationEngine.h"
#include "global_config.h"
#include "swarm/swarm_manager.h"
#include "rendering/renderer.h"
#include "lab.h"
#include <iostream>

SimulationEngine::SimulationEngine() 
    : total_time(0.0f), last_brain_update(0.0f), accumulator(0.0f), seconds_passed(0),
      missions_remaining(MISSION_SPAWN_AMOUNT), mission_dispatch_timer(0.0f) {
    
    virtualWorld = new world(WINDOW_SIZE);
    renderer     = new Renderer(WINDOW_SIZE);
    logger       = new TelemetryLogger();
    bridge       = new NetworkBridge();
    swarm        = new SwarmManager(WINDOW_SIZE);
    city         = new ProceduralCity(WORLD_SIZE.x, WORLD_SIZE.y);
}

SimulationEngine::~SimulationEngine() {
    delete virtualWorld;
    delete renderer;
    delete logger;
    delete bridge;
    delete swarm;
    delete city;
}

void SimulationEngine::init() {
    std::cout << "[ENGINE] Initializing Systems..." << std::endl;
    city->generate(static_cast<unsigned int>(std::time(nullptr)));
    bridge->connect(SERVER_IP, SERVER_PORT);

    // First mission launches immediately on start
    triggerLabMission(swarm, city);
    missions_remaining--;
}

void SimulationEngine::run() {
    std::cout << "[ENGINE] Starting Main Loop..." << std::endl;

    while (renderer->isOpen()) {
        float frameTime = clock.restart().asSeconds();
        total_time += frameTime;

        // 1. Events
        renderer->handleEvents();

        // 2. Update Swarm (Physics & AI)
        swarm->update(DT * TIME_SCALE, *virtualWorld, *city);

        // 3. Render
        render();

        // 4. Logic Per Second (Telemetry & Spawning)
        accumulator += frameTime;
        if (accumulator >= 1.0f) {
            handleLogicPerSecond();
            accumulator -= 1.0f;
        }
    }
}

void SimulationEngine::render() {
    renderer->clear(total_time);
    renderer->drawCity(*city);
    renderer->drawWorld(*virtualWorld);
    renderer->drawSwarm(swarm->getDrones());
    renderer->display();
    renderer->updateCamera(swarm->getDrones());
}

void SimulationEngine::handleLogicPerSecond() {
    seconds_passed++;
    std::cout << "Second: " << seconds_passed << " | Drones: " << swarm->getDrones().size() << std::endl;

    // Mission Queue: dispatch one mission every MISSION_DISPATCH_DELAY seconds
    // so matrices have time to form up and move away before the next batch spawns
    if (missions_remaining > 0) {
        mission_dispatch_timer += 1.0f; // Called once per second
        if (mission_dispatch_timer >= MISSION_DISPATCH_DELAY) {
            std::cout << "[ENGINE] Lanzando mision " << (MISSION_SPAWN_AMOUNT - missions_remaining + 1)
                      << " de " << MISSION_SPAWN_AMOUNT << std::endl;
            triggerLabMission(swarm, city);
            missions_remaining--;
            mission_dispatch_timer = 0.0f;
        }
    }

    // Stats
    SwarmStats stats = swarm->getStats();

    // Retry connection if it failed on startup
    if (!bridge->getIsConnected()) {
        bridge->connect(SERVER_IP, SERVER_PORT);
    }

    // Network Telemetry (LLM Brain)
    if (bridge->getIsConnected() && total_time - last_brain_update >= LLM_QUERY_INTERVAL) {
        bridge->sendMatricesStatus(swarm->getMatrixGroups());
        last_brain_update = total_time;
    }

    // Read LLM Commands
    std::string cmds = bridge->receiveCommands();
    if (!cmds.empty()) {
        std::cout << "[LLM COMMAND] Recibido: " << cmds << std::endl;
        
        // Simple manual JSON parser for: {"id": X, "action": "MOVE", "target": [X, Y]}
        size_t pos = 0;
        while ((pos = cmds.find("\"id\"", pos)) != std::string::npos) {
            size_t colon = cmds.find(":", pos);
            size_t end_id = cmds.find_first_of(",}", colon);
            if (colon == std::string::npos || end_id == std::string::npos) break;
            
            try {
                int id = std::stoi(cmds.substr(colon + 1, end_id - colon - 1));
                
                size_t act_pos = cmds.find("\"action\"", pos);
                size_t act_colon = cmds.find(":", act_pos);
                size_t q1 = cmds.find("\"", act_colon);
                size_t q2 = cmds.find("\"", q1 + 1);
                std::string action = cmds.substr(q1 + 1, q2 - q1 - 1);
                
                auto& matrices = swarm->getMatrixGroups();
                MatrixGroup* target_matrix = nullptr;
                for (auto& m : matrices) {
                    if (m.id == id) { target_matrix = &m; break; }
                }
                
                if (target_matrix && action == "MOVE") {
                    size_t target_pos = cmds.find("\"target\"", pos);
                    if (target_pos != std::string::npos && target_pos < cmds.find("\"id\"", pos + 4)) {
                        size_t b1 = cmds.find("[", target_pos);
                        size_t comma = cmds.find(",", b1);
                        size_t b2 = cmds.find("]", comma);
                        float x = std::stof(cmds.substr(b1 + 1, comma - b1 - 1));
                        float y = std::stof(cmds.substr(comma + 1, b2 - comma - 1));
                        
                        target_matrix->moveTo({x, y});
                        // Aseguramos que la matriz cambie a modo de movimiento activo
                        target_matrix->current_action = 1; // 1 = FOLLOW_MATRIX
                        std::cout << "[TACTICAL] Matrix " << id << " moving to (" << x << ", " << y << ")" << std::endl;
                    }
                }
            } catch(...) {
                // Ignore parse errors for a partial chunk
            }
            pos += 4;
        }
    }

    // File Telemetry (Local Dashboard)
    for (const auto& drone : swarm->getDrones()) {
        if (drone.id < 9) {
            logger->log(total_time, stats.active_drones, drone);
        }
    }
}
