#ifndef SWARM_MANAGER_H
#define SWARM_MANAGER_H

#include <vector>
#include "swarm/swarm_dynamics.h"
#include "swarm/spawn_logic/matrix_group.h"
#include "swarm/spawn_logic/deployment_calculator.h"
#include "swarm/spawn_logic/mission_orchestrator.h"
#include "world/procedural_city.h"
#include "world/world.h"
#include "AI/states.h"

struct SwarmStats {
    int active_drones;
    int critical_battery_count;
    int drones_in_mission;
    float avg_battery;
    float avg_speed;
    float avg_dist_to_target;
};



class SwarmManager {
public:
    SwarmManager(Vector2 worldSize);

    // Ciclo de Vida
    TransportMissionInfo startRandomTransportMission(ProceduralCity& city);
    void update(float dt, const world& vWorld, const ProceduralCity& city);
    
    // Getters para renderizado y telemetría
    const std::vector<DroneChassis>& getDrones() const { return drones; }
    std::vector<MatrixGroup>& getMatrixGroups() { return matrix_groups; }
    std::vector<SpawnBuilding>& getActiveSpawnPlans() { return active_spawn_plans; }
    SwarmStats getStats() const;

private:
    std::vector<DroneChassis> drones;
    std::vector<MatrixGroup> matrix_groups;
    std::vector<SpawnBuilding> active_spawn_plans;
    
    void deploySwarmOnBuilding(const Building& b, MatrixGroup& matrix, int startIdx, int count);
    void runAI(const world& vWorld, const ProceduralCity& city);
    void processSpawning(float dt, const ProceduralCity& city);
    
    float spawn_timer = 0.0f;
};

#endif // SWARM_MANAGER_H
