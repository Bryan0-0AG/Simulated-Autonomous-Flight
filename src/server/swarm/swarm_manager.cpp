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


void SwarmManager::update(float dt, const world& vWorld, const ProceduralCity& city) {
    // 0. Spawning Logic handled by DroneSpawner using Orchestrator plans
    DroneSpawner::updateSpawning(dt, city, drones, matrix_groups, MissionOrchestrator::getActiveSpawnPlans(), spawn_timer);

    if (drones.empty()) return;

    // 1. AI Decisions & Mission Orchestration (CPU)
    runAI(dt, vWorld, city);

    // 2. Physics Kernel (GPU/Fallback)
    launch_physics_kernel(
        drones.data(), 
        static_cast<int>(drones.size()), 
        dt
    );
}

void SwarmManager::runAI(float dt, const world& vWorld, const ProceduralCity& city) {
    for (auto& matrix : matrix_groups) {
        MatrixAI::orchestrate(matrix, matrix_groups, drones, dt);
    }

    // APLICAR REPULSIÓN AQUÍ: Después de que la IA decida a dónde ir, 
    // pero ANTES de que los drones sigan al centro.
    for (auto& matrix : matrix_groups) {
        MatrixAI::Properties::applyDynamicRepulsion(matrix, matrix_groups, drones, dt);
        MatrixAI::Properties::updateMatrixPhysics(matrix, dt);
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
    SwarmStats stats = {0, 0, 0, 0, 0.0f, 0.0f, 0.0f};
    if (drones.empty()) return stats;

    float total_battery = 0.0f;
    float total_speed = 0.0f;
    float total_dist = 0.0f;

    stats.active_drones = static_cast<int>(drones.size());

    // Calcular misiones únicas reales
    std::vector<int> mission_ids;
    for(const auto& m : matrix_groups) {
        if (m.mission_id > 0) {
            if (std::find(mission_ids.begin(), mission_ids.end(), m.mission_id) == mission_ids.end()) {
                mission_ids.push_back(m.mission_id);
            }
        }
    }
    stats.live_missions = static_cast<int>(mission_ids.size());

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
