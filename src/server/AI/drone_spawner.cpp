#include "AI/drone_spawner.h"
#include "AI/matrix/properties.h"
#include "AI/drone_states.h"
#include <algorithm>

void DroneSpawner::updateSpawning(float dt, const ProceduralCity& city, 
                                  std::vector<DroneChassis>& global_drones, 
                                  std::vector<MatrixGroup>& matrix_groups, 
                                  std::vector<DeploymentPlan>& active_spawn_plans, 
                                  float& spawn_timer) 
{
    if (active_spawn_plans.empty()) return;

    spawn_timer += dt;
    if (spawn_timer >= BATCH_INTERVAL) {
        const auto& buildings = city.getBuildings();
        
        static size_t last_plan_idx = 0;
        size_t num_plans = active_spawn_plans.size();
        
        // Intentar encontrar un plan que necesite spawnear, empezando desde el siguiente al último
        for (size_t i = 0; i < num_plans; ++i) {
            size_t current_idx = (last_plan_idx + 1 + i) % num_plans;
            auto& plan = active_spawn_plans[current_idx];
            
            if (plan.actual_batch < plan.total_batches) {
                auto m_it = std::find_if(matrix_groups.begin(), matrix_groups.end(), 
                                       [&](const MatrixGroup& m) { return m.id == plan.matrix_id; });
                
                if (m_it != matrix_groups.end()) {
                    int to_spawn = std::min(plan.drones_per_batch, plan.total_drones - plan.drones_spawned);
                    spawnBatch(global_drones, *m_it, buildings[plan.building_idx], plan.drones_spawned, to_spawn);
                    
                    plan.drones_spawned += to_spawn;
                    plan.actual_batch++;
                    
                    last_plan_idx = current_idx; // Guardar donde nos quedamos
                    spawn_timer = 0.0f;
                    return; 
                }
            }
        }

        // Limpiar planes completados (esto ocurre si el bucle termina sin retornar)
        for (auto it = active_spawn_plans.begin(); it != active_spawn_plans.end(); ) {
            if (it->actual_batch >= it->total_batches) {
                it = active_spawn_plans.erase(it);
                last_plan_idx = 0; // Resetear índice tras limpieza para evitar desbordamientos
            } else {
                ++it;
            }
        }
        
        spawn_timer = 0.0f;
    }
}

void DroneSpawner::spawnBatch(std::vector<DroneChassis>& global_drones, MatrixGroup& matrix, const Building& b, int startIdx, int count) {
    float roof_spacing = 20.0f;
    int drones_per_row_on_roof = static_cast<int>(b.bounds.size.x / roof_spacing);
    if (drones_per_row_on_roof < 1) drones_per_row_on_roof = 1;

    for (int i = 0; i < count; ++i) {
        int currentIdx = startIdx + i;
        DroneChassis drone = {};
        drone.id = static_cast<int>(global_drones.size());
        
        int roof_col = currentIdx % drones_per_row_on_roof;
        
        drone.position.x = b.bounds.position.x + (roof_col * roof_spacing);
        drone.position.y = b.bounds.size.y + 10.0f;
        
        drone.battery = 100.0f;
        drone.max_battery = 100.0f;
        drone.velocity = {0, 0};

        drone.group_id = matrix.id;
        int row_from_top = currentIdx / matrix.cols;
        drone.group_row = (matrix.rows - 1) - row_from_top;
        drone.group_col = currentIdx % matrix.cols;
        
        drone.target = MatrixAI::Properties::getSlotPosition(matrix, drone.group_row, drone.group_col);
        drone.current_state = toInt(DroneState::FLYING);
        drone.current_action = toInt(DroneAction::TAKEOFF);

        global_drones.push_back(drone);
        
        MatrixAI::Properties::insertChild(matrix, drone.id, drone.group_row, drone.group_col);
    }
}
