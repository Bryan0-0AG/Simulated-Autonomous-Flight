#include "SimulationEngine.h"
#include "global_config.h"
#include "swarm/swarm_manager.h"
#include "rendering/renderer.h"
#include "lab.h"
#include <iostream>

SimulationEngine::SimulationEngine() 
    : total_time(0.0f), last_brain_update(0.0f), accumulator(0.0f), seconds_passed(0) {
    
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

    // Llamar al lab para iniciar la misión
    triggerLabMission(swarm, city);
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
    for (const auto& drone : swarm->getDrones()) {
        if (drone.id < 9) {
            logger->log(total_time, stats.active_drones, drone);
        }
    }
}
