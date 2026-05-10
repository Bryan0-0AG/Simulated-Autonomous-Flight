#include "server/lab.h"
#include "swarm/swarm_manager.h"
#include "world/procedural_city.h"
#include "AI/matrix/highways.h"
#include "AI/matrix/spawner.h"
#include "missions/mission_orchestrator.h"
#include <iostream>

void triggerLabMission(SwarmManager* swarm, ProceduralCity* city) {
    if (!swarm || !city) return;
    
    std::cout << "[LAB] Starting experimental mission coordination..." << std::endl;
    
    // Delegate all orchestration to the mission expert
    MissionOrchestrator::startMission(swarm, city);
}
