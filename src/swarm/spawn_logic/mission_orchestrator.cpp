#include "swarm/spawn_logic/mission_orchestrator.h"
#include "global_config.h"
#include <iostream>

void MissionOrchestrator::initiateDeployment(
    const ProceduralCity& city, 
    Vector2 missionTarget, 
    int droneCount, 
    int buildingCount,
    std::vector<MatrixGroup>& matrix_groups,
    std::vector<SpawnBuilding>& active_spawn_plans) 
{
    // 1. Obtener candidatos usando el Calculator (auxiliars)
    std::vector<int> selectedIndices = DeploymentCalculator::findBestSpawnBuildings(city, missionTarget, buildingCount);
    if (selectedIndices.empty()) return;

    int drones_per_building = droneCount / (int)selectedIndices.size();
    const auto& buildings = city.getBuildings();

    // 2. Crear unidades de despliegue
    for (int b_idx : selectedIndices) {
        createDeploymentUnit(buildings[b_idx], b_idx, drones_per_building, matrix_groups, active_spawn_plans);
    }

    std::cout << "[ORCHESTRATOR] Mission started: " << droneCount << " drones across " << selectedIndices.size() << " buildings." << std::endl;
}

void MissionOrchestrator::createDeploymentUnit(
    const Building& b, 
    int b_idx, 
    int dronesForBuilding, 
    std::vector<MatrixGroup>& matrix_groups,
    std::vector<SpawnBuilding>& active_spawn_plans)
{
    // Usar Calculator para las dimensiones
    auto dims = DeploymentCalculator::calculateOptimalDimensions(b, dronesForBuilding);
    
    Vector2 spawnTarget = { b.bounds.position.x + b.bounds.size.x / 2, 1500.0f };
    int matrix_id = (int)matrix_groups.size();
    
    matrix_groups.emplace_back(matrix_id, spawnTarget, dims.cols, FORMATION_SPACING_X, dims.rows, FORMATION_SPACING_Y);
    matrix_groups.back().target_count = dronesForBuilding;
    
    active_spawn_plans.push_back({
        b_idx, 0, dims.batchSize, (dims.rows * dims.cols + dims.batchSize - 1) / dims.batchSize, 0.0f, dims.rows * dims.cols, 0
    });
}

TransportMissionInfo MissionOrchestrator::startTransportMission(ProceduralCity& city) {
    std::vector<Building>& buildings = city.getMutableBuildings();
    
    std::vector<int> collect_indices;
    std::vector<int> deploy_indices;
    
    // 1. Gather candidates
    for (int i = 0; i < (int)buildings.size(); ++i) {
        if (buildings[i].type == BuildingType::COLLECTION && !buildings[i].is_mission_active) {
            collect_indices.push_back(i);
        } else if (buildings[i].type == BuildingType::DEPLOY && !buildings[i].is_mission_active) {
            deploy_indices.push_back(i);
        }
    }
    
    // 2. Need at least one of each
    if (collect_indices.empty() || deploy_indices.empty()) {
        std::cout << "[ORCHESTRATOR] Could not start transport mission: missing COLLECT or DEPLOY buildings." << std::endl;
        return {{0,0}, {0,0}, 0, false};
    }
    
    // 3. Select randomly
    int c_idx = collect_indices[std::rand() % collect_indices.size()];
    int d_idx = deploy_indices[std::rand() % deploy_indices.size()];
    
    // 4. Setup state to visually link them
    buildings[c_idx].is_mission_active = true;
    buildings[c_idx].pending_packages = 50 + (std::rand() % 100); // 50 to 150 packages
    buildings[c_idx].target_building_idx = d_idx;
    
    buildings[d_idx].is_mission_active = true; // Also mark the deploy building as active so it highlights
    
    Vector2 startPos = {buildings[c_idx].bounds.position.x + buildings[c_idx].bounds.size.x / 2.0f,
                        buildings[c_idx].bounds.position.y};
    Vector2 endPos = {buildings[d_idx].bounds.position.x + buildings[d_idx].bounds.size.x / 2.0f,
                      buildings[d_idx].bounds.position.y};
    
    std::cout << "[ORCHESTRATOR] Transport Mission Created! Collect ID: " << c_idx 
              << " -> Deploy ID: " << d_idx << " (" << buildings[c_idx].pending_packages << " pkgs)" << std::endl;
              
    return {startPos, endPos, buildings[c_idx].pending_packages, true};
}
