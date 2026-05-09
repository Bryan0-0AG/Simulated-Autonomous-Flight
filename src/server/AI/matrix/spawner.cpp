#include "AI/matrix/spawner.h"
#include "missions/mission_calculator.h"
#include "global_config.h"

MatrixGroup MatrixSpawner::spawn(int matrix_id, const Building& b, int dronesForBuilding) {
    auto dims = MissionCalculator::calculateOptimalDimensions(b, dronesForBuilding);
    
    // Assign lane based on matrix ID (0 = Outbound, 1 = Inbound)
    int assigned_lane = matrix_id % 2;
    float highway_y = (assigned_lane == 0) ? HIGHWAY_Y_OUTBOUND : HIGHWAY_Y_INBOUND;

    Vector2 spawnTarget = { b.bounds.position.x + b.bounds.size.x / 2.0f, highway_y };
    
    MatrixGroup matrix(matrix_id, spawnTarget, dims.cols, FORMATION_SPACING_X, dims.rows, FORMATION_SPACING_Y);
    matrix.current_target = spawnTarget; // Preservamos el punto de ensamblaje
    matrix.target_count = dronesForBuilding;
    matrix.lane = assigned_lane;
    
    return matrix;
}
