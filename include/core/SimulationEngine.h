#ifndef SIMULATION_ENGINE_H
#define SIMULATION_ENGINE_H

#include <SFML/Graphics.hpp>
#include "swarm/swarm_manager.h"
#include "rendering/renderer.h"
#include "world/procedural_city.h"
#include "world/world.h"
#include "telemetry/telemetry_logger.h"
#include "network/bridge.h"

enum class EngineMode { SERVER, CLIENT, LOCAL };

class SimulationEngine {
public:
    SimulationEngine(EngineMode mode = EngineMode::LOCAL);
    ~SimulationEngine();

    void init();
    void run();

private:
    // Systems
    world* virtualWorld;
    Renderer* renderer;
    TelemetryLogger* logger;
    NetworkBridge* bridge;
    SwarmManager* swarm;
    ProceduralCity* city;

    EngineMode currentMode;

    // State
    sf::Clock clock;
    float total_time;
    float last_brain_update;
    float accumulator;
    int seconds_passed;

    void handleLogicPerSecond();
    void render();
};

#endif // SIMULATION_ENGINE_H
