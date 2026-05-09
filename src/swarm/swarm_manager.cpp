#include "swarm/swarm_manager.h"
#include "global_config.h"
#include "AI/drone_ai.h"
#include "AI/matrix_ai.h"
#include "utils/math_utils.h"
#include <iostream>
#include <algorithm>
#include <cmath>

SwarmManager::SwarmManager(Vector2 worldSize) {
    drones.reserve(DRONE_COUNT);
}

void SwarmManager::deploySwarmOnBuilding(const Building& b, MatrixGroup& matrix, int startIdx, int count) {
    float roof_spacing = 20.0f;
    int drones_per_row_on_roof = static_cast<int>(b.bounds.size.x / roof_spacing);
    if (drones_per_row_on_roof < 1) drones_per_row_on_roof = 1;

    for (int i = 0; i < count; ++i) {
        int currentIdx = startIdx + i;
        DroneChassis drone = {};
        drone.id = static_cast<int>(drones.size());
        
        // 1. Posicionamiento en el techo
        int roof_col = currentIdx % drones_per_row_on_roof;
        
        drone.position.x = b.bounds.position.x + (roof_col * roof_spacing);
        drone.position.y = b.bounds.size.y + 10.0f;
        
        // 2. Datos básicos
        drone.battery = 100.0f;
        drone.max_battery = 100.0f;
        drone.velocity = {0, 0};

        // 3. Asignación de Matrix (Invertida: Primeros drones arriba)
        drone.group_id = matrix.id;
        int row_from_top = currentIdx / matrix.cols;
        drone.group_row = (matrix.rows - 1) - row_from_top;
        drone.group_col = currentIdx % matrix.cols;
        
        // 4. Target y Estado inicial
        drone.target = matrix.getSlotPosition(drone.group_row, drone.group_col);
        drone.current_state = toInt(DroneState::FLYING);
        drone.current_action = toInt(DroneAction::TAKEOFF);

        drones.push_back(drone);
        
        // Vincular a la matriz por ID (SEGURO)
        matrix.insertChild(drone.id, drone.group_row, drone.group_col);
    }
}

TransportMissionInfo SwarmManager::startRandomTransportMission(ProceduralCity& city) {
    return MissionOrchestrator::startTransportMission(city);
}

void SwarmManager::processSpawning(float dt, const ProceduralCity& city) {
    if (active_spawn_plans.empty()) return;

    spawn_timer += dt;
    if (spawn_timer >= 1.0f) {
        const auto& buildings = city.getBuildings();
        bool all_finished = true;

        for (size_t i = 0; i < active_spawn_plans.size(); ++i) {
            auto& plan = active_spawn_plans[i];
            if (plan.actual_batch < plan.total_batches) {
                int to_spawn = std::min(plan.drones_per_batch, plan.total_drones - plan.drones_spawned);
                deploySwarmOnBuilding(buildings[plan.building_idx], matrix_groups[i], plan.drones_spawned, to_spawn);
                
                plan.drones_spawned += to_spawn;
                plan.actual_batch++;
                all_finished = false;
            }
        }

        if (all_finished) {
            // No limpiamos los planes inmediatamente si queremos conservarlos para algo, 
            // pero para esta refactorizacion, si todos terminaron, podriamos limpiar.
            // active_spawn_plans.clear();
        }
        spawn_timer = 0.0f;
    }
}

void SwarmManager::update(float dt, const world& vWorld, const ProceduralCity& city) {
    // 0. Spawning Logic
    processSpawning(dt, city);

    if (drones.empty()) return;

    // 1. AI Decisions & Mission Orchestration (CPU)
    runAI(vWorld, city, dt);

    // 2. Prepare Obstacles for GPU (Buildings)
    const auto& buildings = city.getBuildings();
    std::vector<GPUObstacle> gpu_obstacles;
    gpu_obstacles.reserve(buildings.size());
    
    for (const auto& b : buildings) {
        GPUObstacle obs;
        obs.x = b.bounds.position.x;
        obs.y = b.bounds.position.y;
        obs.w = b.bounds.size.x;
        obs.h = b.bounds.size.y;
        gpu_obstacles.push_back(obs);
    }

    // 3. Physics Kernel (GPU/Fallback) with Obstacles
    launch_physics_kernel(
        drones.data(), 
        static_cast<int>(drones.size()), 
        dt, 
        //gpu_obstacles.data(), 
        //static_cast<int>(gpu_obstacles.size()),
        nullptr,
        0
    );
}

void SwarmManager::runAI(const world& vWorld, const ProceduralCity& city, float dt) {
    // Run matrix AI as a single batch (which dispatches to GPU)
    update_matrix_ai(matrix_groups, drones, dt);
    
    for(auto& drone : drones) {
        // Resolve world collisions before AI decisions
        const_cast<world&>(vWorld).resolveGroundCollision(drone);

        // If part of a group, follow the matrix slot dynamically
        if (drone.group_id != -1) {
            Vector2 slotPos = matrix_groups[drone.group_id].getSlotPosition(drone.group_row, drone.group_col);
            
            // TAKEOFF: vertical-first ascent to avoid diagonal paths through other formations
            // Phase 1: Ascend vertically (keep current X, go to slot Y)
            // Phase 2: Once at altitude, fly horizontally to slot X
            if (toAction(drone.current_action) == DroneAction::TAKEOFF) {
                float yDiff = std::abs(drone.position.y - slotPos.y);
                if (yDiff > 50.0f) {
                    // Phase 1: Go straight up to the matrix altitude
                    drone.target = { drone.position.x, slotPos.y };
                } else {
                    // Phase 2: At altitude, now move horizontally to the slot
                    drone.target = slotPos;
                }
            } else {
                // FOLLOW_MATRIX and other states: track slot position directly
                drone.target = slotPos;
            }
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
