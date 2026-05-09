#ifndef MISSION_ORCHESTRATOR_H
#define MISSION_ORCHESTRATOR_H

#include <vector>
#include "world/procedural_city.h"
#include "swarm/spawn_logic/matrix_group.h"
#include "swarm/spawn_logic/deployment_calculator.h"

struct TransportMissionInfo {
    Vector2 startPos;
    Vector2 endPos;
    int pendingPackages;
    bool valid;
};

class MissionOrchestrator {
public:
    static void initiateDeployment(
        const ProceduralCity& city, 
        Vector2 missionTarget, 
        int droneCount, 
        int buildingCount,
        std::vector<MatrixGroup>& matrix_groups,
        std::vector<SpawnBuilding>& active_spawn_plans
    );

    // Creates a new transport mission linking one COLLECT and one DEPLOY building
    static TransportMissionInfo startTransportMission(ProceduralCity& city);

private:
    static void createDeploymentUnit(
        const Building& b, 
        int b_idx, 
        int dronesForBuilding, 
        std::vector<MatrixGroup>& matrix_groups,
        std::vector<SpawnBuilding>& active_spawn_plans
    );
};

#endif
