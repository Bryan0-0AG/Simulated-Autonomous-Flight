#ifndef MISSION_ORCHESTRATOR_H
#define MISSION_ORCHESTRATOR_H

#include <vector>
#include "world/procedural_city.h"
#include "AI/matrix/matrix_group.h"
#include "missions/mission_calculator.h"

struct TransportMissionInfo {
    Vector2 startPos;
    Vector2 endPos;
    int pendingPackages;
    bool valid;
};

struct DeploymentPlan {
    int building_idx;
    int actual_batch;
    int drones_per_batch;
    int total_batches; 
    float xspacing;
    int total_drones;
    int drones_spawned;
};

class MissionOrchestrator {
public:
    // Retorna los planes lógicos de despliegue
    static std::vector<DeploymentPlan> createDeploymentPlans(
        const ProceduralCity& city, 
        Vector2 missionTarget, 
        int droneCount, 
        int buildingCount);

    static TransportMissionInfo startTransportMission(ProceduralCity& city);
};

#endif
