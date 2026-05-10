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
    int live_missions;       // Contador real de misiones activas
    float avg_battery;
    float avg_speed;
    float avg_dist_to_target;
};


class SwarmManager {
public:
    SwarmManager(Vector2 worldSize);

    // Ciclo de Vida
    void update(float dt, const world& vWorld, const ProceduralCity& city);
    
    // Getters para renderizado y telemetría
    const std::vector<DroneChassis>& getDrones() const { return drones; }
    std::vector<DroneChassis>& getDronesRef() { return drones; }
    std::vector<MatrixGroup>& getMatrixGroups() { return matrix_groups; }
    SwarmStats getStats() const;

    // El puente que une todo

private:
    std::vector<DroneChassis> drones;
    std::vector<MatrixGroup> matrix_groups;
    void runAI(float dt, const world& vWorld, const ProceduralCity& city);
    void processSpawning(float dt, const ProceduralCity& city);
    
    float spawn_timer = 0.0f;
};

#endif // SWARM_MANAGER_H
