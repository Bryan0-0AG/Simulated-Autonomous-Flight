#include "swarm/spawn_logic/mission_orchestrator.h"
#include "global_config.h"
#include "AI/states.h"
#include <iostream>
#include <set>

void MissionOrchestrator::initiateDeployment(
    const ProceduralCity& city, 
    Vector2 missionTarget, 
    int droneCount, 
    int buildingCount,
    std::vector<MatrixGroup>& matrix_groups,
    std::vector<SpawnBuilding>& active_spawn_plans) 
{
    // Build set of buildings already occupied by TAKEOFF matrices
    std::set<int> occupied;
    for (size_t i = 0; i < active_spawn_plans.size(); i++) {
        // Only block if the matrix is still forming (TAKEOFF state)
        if (i < matrix_groups.size() &&
            matrix_groups[i].current_action == toInt(DroneAction::TAKEOFF)) {
            occupied.insert(active_spawn_plans[i].building_idx);
        }
    }

    // Find spawn buildings, excluding occupied ones
    std::vector<int> selectedIndices = DeploymentCalculator::findBestSpawnBuildings(city, missionTarget, buildingCount, occupied);
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
    
    int matrix_id = (int)matrix_groups.size();
    
    // Assign staging slot (where matrix forms up, above buildings, below highways)
    int stagingSlot = matrix_id % MAX_STAGING_LANES;
    float stagingAltitude = STAGING_ALTITUDE_BASE + (stagingSlot * STAGING_LANE_HEIGHT);
    
    // Pre-assign highway lane (where matrix will navigate after formation is stable)
    int highwayLane = matrix_id % MAX_AERIAL_LANES;
    
    float baseX = b.bounds.position.x + b.bounds.size.x / 2.0f;

    // Calculate the projected bounds of the new matrix at its staging position
    float halfW = (dims.cols * FORMATION_SPACING_X) / 2.0f + MATRIX_SAFETY_MARGIN;
    float halfH = (dims.rows * FORMATION_SPACING_Y) / 2.0f + MATRIX_SAFETY_MARGIN;

    // Resolve collisions in the STAGING zone: shift position until no overlap.
    float spawnX = baseX;
    int spawnStaging = stagingSlot;
    float spawnY = stagingAltitude;
    int maxAttempts = MAX_STAGING_LANES * 10; // Safety cap to avoid infinite loop

    for (int attempt = 0; attempt < maxAttempts; attempt++) {
        bool collision = false;
        for (const auto& existing : matrix_groups) {
            // Compute existing matrix bounds
            float exHalfW = (existing.cols * existing.col_spacing) / 2.0f + MATRIX_SAFETY_MARGIN;
            float exHalfH = (existing.rows * existing.row_spacing) / 2.0f + MATRIX_SAFETY_MARGIN;

            // AABB overlap test between projected new position and existing matrix
            bool xOverlap = (spawnX - halfW) < (existing.center.x + exHalfW) &&
                            (spawnX + halfW) > (existing.center.x - exHalfW);
            bool yOverlap = (spawnY - halfH) < (existing.center.y + exHalfH) &&
                            (spawnY + halfH) > (existing.center.y - exHalfH);

            if (xOverlap && yOverlap) {
                collision = true;
                // Shift X to the right edge of the blocking matrix + full width margin
                spawnX = existing.center.x + exHalfW + halfW + MATRIX_SAFETY_MARGIN;
                break; // Re-check all matrices with the new position
            }
        }

        if (!collision) break; // Found a clear spot

        // If X drifted too far from building (beyond 5 matrix widths), try next staging slot
        if (spawnX > baseX + (halfW * 2.0f) * 5.0f) {
            spawnStaging = (spawnStaging + 1) % MAX_STAGING_LANES;
            spawnY = STAGING_ALTITUDE_BASE + (spawnStaging * STAGING_LANE_HEIGHT);
            spawnX = baseX; // Reset X for the new staging slot
        }
    }

    Vector2 spawnTarget = { spawnX, spawnY };
    
    matrix_groups.emplace_back(matrix_id, spawnTarget, dims.cols, FORMATION_SPACING_X, dims.rows, FORMATION_SPACING_Y);
    matrix_groups.back().target_count = dronesForBuilding;
    // assigned_lane is the HIGHWAY lane (used after formation completes and matrix ascends)
    matrix_groups.back().assigned_lane = highwayLane;
    
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
    buildings[c_idx].pending_packages = PACKAGES_PER_MISSION_MIN + (std::rand() % (PACKAGES_PER_MISSION_MAX - PACKAGES_PER_MISSION_MIN));
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
