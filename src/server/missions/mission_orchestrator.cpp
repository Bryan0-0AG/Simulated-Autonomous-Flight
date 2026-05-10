#include "missions/mission_orchestrator.h"
#include "swarm/swarm_manager.h"
#include "AI/matrix/spawner.h"
#include "AI/matrix/highways.h"
#include "global_config.h"
#include <iostream>
#include <algorithm>

// ==========================================
// ESTADO GLOBAL
// ==========================================
int MissionOrchestrator::mission_counter = 0;
std::vector<DeploymentPlan> MissionOrchestrator::active_spawn_plans;

// ==========================================
// 1. FASE DE GENERACIÓN
// ==========================================
TransportMissionInfo MissionOrchestrator::generateTransportMission(ProceduralCity& city) {
    std::vector<Building>& buildings = city.getMutableBuildings();
    
    std::vector<int> collect_indices;
    std::vector<int> deploy_indices;
    
    for (int i = 0; i < (int)buildings.size(); ++i) {
        if (buildings[i].type == BuildingType::COLLECTION && !buildings[i].is_mission_active) {
            collect_indices.push_back(i);
        } else if (buildings[i].type == BuildingType::DEPLOY && !buildings[i].is_mission_active) {
            deploy_indices.push_back(i);
        }
    }
    
    if (collect_indices.empty() || deploy_indices.empty()) {
        std::cout << "[ORCHESTRATOR] Generation failed: missing COLLECT or DEPLOY candidates." << std::endl;
        return {{0,0}, {0,0}, 0, false};
    }
    
    int c_idx = collect_indices[std::rand() % collect_indices.size()];
    int d_idx = deploy_indices[std::rand() % deploy_indices.size()];
    
    buildings[c_idx].is_mission_active = true;
    buildings[c_idx].pending_packages = PACKAGE_MIN_COUNT + (std::rand() % (PACKAGE_MAX_COUNT - PACKAGE_MIN_COUNT + 1));
    buildings[c_idx].target_building_idx = d_idx;
    buildings[d_idx].is_mission_active = true; 
    
    Vector2 startPos = {buildings[c_idx].bounds.position.x + buildings[c_idx].bounds.size.x / 2.0f,
                        buildings[c_idx].bounds.position.y};
    Vector2 endPos = {buildings[d_idx].bounds.position.x + buildings[d_idx].bounds.size.x / 2.0f,
                      buildings[d_idx].bounds.position.y};
    
    return {startPos, endPos, buildings[c_idx].pending_packages, true};
}

// ==========================================
// 2. FASE DE PLANIFICACIÓN
// ==========================================
std::vector<DeploymentPlan> MissionOrchestrator::createDeploymentPlans(
    const ProceduralCity& city, 
    Vector2 missionTarget, 
    int droneCount, 
    int buildingCount,
    int mission_id) 
{
    std::vector<DeploymentPlan> plans;
    std::vector<int> selectedIndices = MissionCalculator::findBestSpawnBuildings(city, missionTarget, buildingCount);
    
    if (selectedIndices.empty()) return plans;

    int drones_per_building = droneCount / (int)selectedIndices.size();
    const auto& buildings = city.getBuildings();

    for (int b_idx : selectedIndices) {
        auto dims = MissionCalculator::calculateOptimalDimensions(buildings[b_idx], drones_per_building);
        plans.push_back({
            b_idx, 0, dims.batchSize, (dims.rows * dims.cols + dims.batchSize - 1) / dims.batchSize, 
            0.0f, dims.rows * dims.cols, 0, mission_id
        });
    }

    return plans;
}

// ==========================================
// 3. FASE DE EJECUCIÓN (START)
// ==========================================
void MissionOrchestrator::startMission(SwarmManager* swarm, ProceduralCity* city) {
    if (!swarm || !city) return;

    // STEP 1: GENERAR
    TransportMissionInfo info = generateTransportMission(*city);
    if (!info.valid) return;

    mission_counter++;
    std::cout << "[ORCHESTRATOR] Starting Mission #" << mission_counter << std::endl;

    // Sincronización de Altura con Autopistas
    if (info.endPos.x > info.startPos.x) {
        info.startPos.y = HIGHWAY_Y_OUTBOUND;
        info.endPos.y = HIGHWAY_Y_OUTBOUND;
    } else {
        info.startPos.y = HIGHWAY_Y_INBOUND;
        info.endPos.y = HIGHWAY_Y_INBOUND;
    }

    // STEP 2: PLANIFICAR
    auto plans = createDeploymentPlans(*city, info.startPos, info.pendingPackages, 5, mission_counter);
    
    // STEP 3: DESPLEGAR
    auto& matrix_groups = swarm->getMatrixGroups();
    for (auto& plan : plans) {
        MatrixGroup m = MatrixSpawner::spawn(matrix_groups.size(), city->getBuildings()[plan.building_idx], plan.total_drones);
        m.mission_id = mission_counter;
        m.final_target = info.endPos;
        
        plan.matrix_id = m.id;
        plan.mission_id = mission_counter;

        MatrixAI::Highways::assignLane(m);

        matrix_groups.push_back(m);
        active_spawn_plans.push_back(plan);

        // Libera el edificio para que pueda ser usado en la proxima mision una vez spawneado el enjambre
        city->getMutableBuildings()[plan.building_idx].is_mission_active = false;
    }
}
