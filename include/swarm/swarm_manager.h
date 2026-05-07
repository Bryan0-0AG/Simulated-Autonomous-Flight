#ifndef SWARM_MANAGER_H
#define SWARM_MANAGER_H

#include <vector>
#include "swarm/swarm_dynamics.h"
#include "swarm/matrix_group.h"
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

struct SpawnBuilding {
    int building_idx;
    int actual_batch;
    int drones_per_batch;
    int total_batches; 
    float xspacing;
    int total_drones;
    int drones_spawned;
};

class SwarmManager {
public:
    SwarmManager(Vector2 worldSize);

    // Ciclo de Vida
    void spawnFromRoofs(const ProceduralCity& city, int batchSize);
    void update(float dt, const world& vWorld, const ProceduralCity& city);
    
    // Getters para renderizado y telemetría
    const std::vector<DroneChassis>& getDrones() const { return drones; }
    SwarmStats getStats() const;

private:
    std::vector<DroneChassis> drones;
    std::vector<MatrixGroup> matrix_groups;
    std::vector<SpawnBuilding> active_spawn_plans;
    
    void deploySwarmOnBuilding(const Building& b, MatrixGroup& matrix, int startIdx, int count);
    void runAI(const world& vWorld, const ProceduralCity& city);
};

#endif // SWARM_MANAGER_H
