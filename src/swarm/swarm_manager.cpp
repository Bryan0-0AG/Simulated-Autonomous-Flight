#include "swarm/swarm_manager.h"
#include "global_config.h"
#include "AI/drone_ai.h"
#include "AI/matrix_ai.h"
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

void SwarmManager::spawnFromRoofs(const ProceduralCity& city, int batchSize) {
    if (drones.size() >= DRONE_COUNT) return;

    const auto& buildings = city.getBuildings();
    if (buildings.empty()) return;

    // 1. FASE DE PLANIFICACIÓN (Solo se ejecuta una vez)
    if (active_spawn_plans.empty()) {
        // Filtrar solo edificios de tipo SPAWN
        std::vector<int> spawn_building_indices;
        for (int i = 0; i < (int)buildings.size(); ++i) {
            if (buildings[i].type == BuildingType::SPAWN) {
                spawn_building_indices.push_back(i);
            }
        }

        if (spawn_building_indices.empty()) {
            std::cout << "[ERROR] No se encontraron edificios de tipo SPAWN en la ciudad." << std::endl;
            return;
        }

        int buildings_to_use = (BUILDINGS_TO_USE == -1) ? 
            (int)spawn_building_indices.size() : 
            std::min((int)spawn_building_indices.size(), BUILDINGS_TO_USE);
        int drones_per_building = DRONE_COUNT / buildings_to_use;

        for (int i = 0; i < buildings_to_use; ++i) {
            int b_idx = spawn_building_indices[i];
            const auto& b = buildings[b_idx];

            Vector2 target_pos = { b.bounds.position.x + b.bounds.size.x / 2, 1500.0f };
            int cols = DRONES_PER_ROW;
            int rows = (drones_per_building + cols - 1) / cols;
            
            MatrixGroup matrix(i, target_pos, cols, FORMATION_SPACING_X, rows, FORMATION_SPACING_Y);
            matrix.target_count = drones_per_building; // <-- AQUÍ
            
            SpawnBuilding plan;
            plan.building_idx = b_idx;
            plan.actual_batch = 0;
            plan.total_drones = matrix.rows * matrix.cols;
            
            float roof_spacing = 20.0f;
            int drones_per_batch = static_cast<int>(buildings[i].bounds.size.x / roof_spacing);
            if (drones_per_batch < 1) drones_per_batch = 1;
            
            plan.drones_per_batch = drones_per_batch;
            plan.total_batches = (plan.total_drones + drones_per_batch - 1) / drones_per_batch;
            plan.drones_spawned = 0;
            
            active_spawn_plans.push_back(plan);
            matrix_groups.push_back(matrix);
        }
        std::cout << "[SWARM] Spawning plan initialized for " << buildings_to_use << " buildings." << std::endl;
    }

    // 2. FASE DE EJECUCIÓN
    for (size_t i = 0; i < active_spawn_plans.size(); ++i) {
        auto& plan = active_spawn_plans[i];
        if (plan.actual_batch < plan.total_batches) {
            int to_spawn = std::min(plan.drones_per_batch, plan.total_drones - plan.drones_spawned);
            
            // USAR LA MATRIZ REAL DEL VECTOR GLOBAL
            deploySwarmOnBuilding(buildings[plan.building_idx], matrix_groups[i], plan.drones_spawned, to_spawn);
            
            plan.drones_spawned += to_spawn;
            plan.actual_batch++;
        }
    }
}

void SwarmManager::update(float dt, const world& vWorld, const ProceduralCity& city) {
    if (drones.empty()) return;

    // 1. AI Decisions & Mission Orchestration (CPU)
    runAI(vWorld, city);

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

void SwarmManager::runAI(const world& vWorld, const ProceduralCity& city) {
    for (auto& matrix : matrix_groups) {
        update_matrix_ai(matrix, vWorld, drones);
    }
    
    for(auto& drone : drones) {
        // Resolve world collisions before AI decisions
        const_cast<world&>(vWorld).resolveGroundCollision(drone);

        // If part of a group, follow the matrix slot dynamically in any action
        if (drone.group_id != -1) {
            drone.target = matrix_groups[drone.group_id].getSlotPosition(drone.group_row, drone.group_col);            
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
