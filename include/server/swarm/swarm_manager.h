#ifndef SWARM_MANAGER_H
#define SWARM_MANAGER_H

#include <vector>
#include "drone_dynamics.h"
#include "AI/matrix/matrix_group.h"
#include "missions/mission_calculator.h"
#include "missions/mission_orchestrator.h"
#include "world/procedural_city.h"
#include "world/world.h"
#include "AI/drone_states.h"

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
    std::vector<DroneChassis>& getDronesRef() { return drones; }
    std::vector<MatrixGroup>& getMatrixGroups() { return matrix_groups; }
    std::vector<DeploymentPlan>& getActiveSpawnPlans() { return active_spawn_plans; }
    SwarmStats getStats() const;

    // El puente que une todo
    void startMission(const ProceduralCity& city, Vector2 missionTarget, int droneCount, int buildingCount);

private:
    std::vector<DroneChassis> drones;
    std::vector<MatrixGroup> matrix_groups;
    std::vector<DeploymentPlan> active_spawn_plans;
    
    void runAI(const world& vWorld, const ProceduralCity& city);
    void processSpawning(float dt, const ProceduralCity& city);
    
    float spawn_timer = 0.0f;
};

#endif // SWARM_MANAGER_H
