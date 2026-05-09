#include "swarm/swarm_manager.h"
#include "global_config.h"
#include "AI/drone_ai.h"
#include "AI/matrix/decisions.h"
#include "AI/matrix/properties.h"
#include "AI/matrix/spawner.h"
#include "AI/drone_spawner.h"
#include "utils/math_utils.h"
#include <iostream>
#include <algorithm>
#include <cmath>

SwarmManager::SwarmManager(Vector2 worldSize) {
    drones.reserve(DRONE_COUNT);
}

void SwarmManager::startMission(const ProceduralCity& city, Vector2 missionTarget, int droneCount, int buildingCount) {
    auto plans = MissionOrchestrator::createDeploymentPlans(city, missionTarget, droneCount, buildingCount);
    
    for (const auto& plan : plans) {
        MatrixGroup m = MatrixSpawner::spawn(matrix_groups.size(), city.getBuildings()[plan.building_idx], plan.total_drones);
        matrix_groups.push_back(m);
        active_spawn_plans.push_back(plan);
    }
}

TransportMissionInfo SwarmManager::startRandomTransportMission(ProceduralCity& city) {
    return MissionOrchestrator::startTransportMission(city);
}


void SwarmManager::update(float dt, const world& vWorld, const ProceduralCity& city) {
    // 0. Spawning Logic handled entirely by DroneSpawner
    DroneSpawner::updateSpawning(dt, city, drones, matrix_groups, active_spawn_plans, spawn_timer);

    if (drones.empty()) return;

    // 1. AI Decisions & Mission Orchestration (CPU)
    runAI(vWorld, city);

    // 2. Physics Kernel (GPU/Fallback)
    launch_physics_kernel(
        drones.data(), 
        static_cast<int>(drones.size()), 
        dt
    );
}

void SwarmManager::runAI(const world& vWorld, const ProceduralCity& city) {
    for (auto& matrix : matrix_groups) {
        MatrixAI::orchestrate(matrix, matrix_groups, drones, DT);
    }
    
    for(auto& drone : drones) {
        // Resolve world collisions before AI decisions
        const_cast<world&>(vWorld).resolveGroundCollision(drone);

        // If part of a group, follow the matrix slot dynamically in any action
        if (drone.group_id != -1) {
            drone.target = MatrixAI::Properties::getSlotPosition(matrix_groups[drone.group_id], drone.group_row, drone.group_col);            
        }

        // AI Decisions        
        update_drone_ai(drone, vWorld, city);
    }
}

SwarmStats SwarmManager::getStats() const {
    SwarmStats stats = {0, 0, 0, 0.0f, 0.0f, 0.0f};
    if (drones.empty()) return stats;

    float total_battery = 0.0f;
    float total_speed = 0.0f;
    float total_dist = 0.0f;

    stats.active_drones = static_cast<int>(drones.size());

    for(const auto& d : drones) {
        total_battery += d.battery;
        if(d.battery < 20.0f) stats.critical_battery_count++;

        float speed = std::sqrt(d.velocity.x * d.velocity.x + d.velocity.y * d.velocity.y);
        total_speed += speed;

        if(toAction(d.current_action) == DroneAction::FOLLOW_MATRIX || 
           toAction(d.current_action) == DroneAction::REJOINING_MATRIX) {
            stats.drones_in_mission++;
            float dx = d.target.x - d.position.x;
            float dy = d.target.y - d.position.y;
            total_dist += std::sqrt(dx*dx + dy*dy);
        }
    }

    stats.avg_battery = total_battery / stats.active_drones;
    stats.avg_speed = total_speed / stats.active_drones;
    stats.avg_dist_to_target = (stats.drones_in_mission > 0) ? (total_dist / stats.drones_in_mission) : 0.0f;

    return stats;
}
