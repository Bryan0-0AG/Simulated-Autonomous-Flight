#include "SimulationEngine.h"
#include "global_config.h"
#include "swarm/swarm_manager.h"
#include "rendering/renderer.h"
#include "lab.h"
#include <iostream>

SimulationEngine::SimulationEngine(EngineMode mode) 
    : currentMode(mode), total_time(0.0f), last_brain_update(0.0f), accumulator(0.0f), seconds_passed(0) {
    
    virtualWorld = new world(WINDOW_SIZE);
    
    if (currentMode != EngineMode::SERVER) {
        renderer = new Renderer(WINDOW_SIZE);
    } else {
        renderer = nullptr;
    }
    
    logger       = new TelemetryLogger();
    bridge       = new NetworkBridge();
    swarm        = new SwarmManager(WINDOW_SIZE);
    city         = new ProceduralCity(WORLD_SIZE.x, WORLD_SIZE.y);
}

SimulationEngine::~SimulationEngine() {
    delete virtualWorld;
    if (renderer) delete renderer;
    delete logger;
    delete bridge;
    delete swarm;
    delete city;
}

void SimulationEngine::init() {
    std::cout << "[ENGINE] Initializing Systems... Mode: " << (int)currentMode << std::endl;
    city->generate(static_cast<unsigned int>(std::time(nullptr)));

    if (currentMode == EngineMode::SERVER || currentMode == EngineMode::LOCAL) {
        bridge->connect(SERVER_IP, SERVER_PORT); // Conectar al Orquestador Python
        
        if (currentMode == EngineMode::SERVER) {
            bridge->startVisualizerServer(9998); // Abrir puerto para que el Visor se conecte
        }

        for (int i = 0; i<6; i++){
            triggerLabMission(swarm, city);
        }
    } else if (currentMode == EngineMode::CLIENT) {
        std::cout << "[ENGINE] Connecting to Cloud Simulator at " << SERVER_IP << "..." << std::endl;
        while (!bridge->connectToCloud(SERVER_IP, 9998)) {
            std::cout << "[ENGINE] Retrying in 2 seconds..." << std::endl;
            sf::sleep(sf::seconds(2.0f));
        }
    }
}

void SimulationEngine::run() {
    std::cout << "[ENGINE] Starting Main Loop..." << std::endl;

    while (currentMode == EngineMode::SERVER || (renderer && renderer->isOpen())) {
        // [NUBE] Limitar la velocidad del servidor a 60 Ticks Per Second
        if (currentMode == EngineMode::SERVER) {
            float elapsed = clock.getElapsedTime().asSeconds();
            if (elapsed < DT) {
                sf::sleep(sf::seconds(DT - elapsed));
            }
        }

        float frameTime = clock.restart().asSeconds();
        total_time += frameTime;

        // 1. Events (Solo en Cliente/Local)
        if (currentMode != EngineMode::SERVER && renderer) {
            renderer->handleEvents();
        }

        // 2. Lógica del Enjambre o Recepción de Red
        if (currentMode == EngineMode::SERVER || currentMode == EngineMode::LOCAL) {
            swarm->update(DT * TIME_SCALE, *virtualWorld, *city);
            
            if (currentMode == EngineMode::SERVER) {
                bridge->sendPositionsToViewer(swarm->getDrones());
                bridge->sendBuildingsToViewer(*city);
            }
        } else if (currentMode == EngineMode::CLIENT) {
            // El cliente no simula fisicas, solo recibe posiciones
            bridge->receivePositions(swarm->getDronesRef());
        }

        // 3. Render (Solo en Cliente/Local)
        if (currentMode != EngineMode::SERVER) {
            render();
        }

        // 4. Logic Per Second (Telemetry & Spawning)
        accumulator += frameTime;
        if (accumulator >= 1.0f) {
            if (currentMode == EngineMode::SERVER || currentMode == EngineMode::LOCAL) {
                handleLogicPerSecond();
            } else {
                std::cout << "Viewer frame: Rendering " << swarm->getDrones().size() << " drones." << std::endl;
            }
            accumulator -= 1.0f;
        }
    }
}

void SimulationEngine::render() {
    if (!renderer) return;
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

    // Stats
    SwarmStats stats = swarm->getStats();

    // Network Telemetry (LLM Brain)
    if (total_time - last_brain_update >= LLM_QUERY_INTERVAL) {
        bridge->sendSwarmStatus(
            stats.active_drones,
            stats.critical_battery_count,
            stats.drones_in_mission,
            stats.avg_battery,
            stats.avg_speed,
            stats.avg_dist_to_target
        );
        last_brain_update = total_time;
    }

    // File Telemetry (Local Dashboard)
    for (const auto& matrix : swarm->getMatrixGroups()) {
        logger->log(total_time, stats.active_drones, matrix, swarm->getDrones());
    }
}
