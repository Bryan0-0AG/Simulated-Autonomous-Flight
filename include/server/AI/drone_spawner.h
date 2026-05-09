#pragma once
#include "AI/matrix/matrix_group.h"
#include "shared/world/procedural_city.h"
#include "AI/drone_ai.h"
#include <vector>

#include "missions/mission_calculator.h"
#include "missions/mission_orchestrator.h"

class DroneSpawner {
public:
    static void updateSpawning(float dt, const ProceduralCity& city, 
                               std::vector<DroneChassis>& global_drones, 
                               std::vector<MatrixGroup>& matrix_groups, 
                               std::vector<DeploymentPlan>& active_spawn_plans, 
                               float& spawn_timer);

private:
    static void spawnBatch(std::vector<DroneChassis>& global_drones, MatrixGroup& matrix, const Building& b, int startIdx, int count);
};
