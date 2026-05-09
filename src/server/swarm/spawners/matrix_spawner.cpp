#include "swarm/spawners/matrix_spawner.h"
#include "missions/mission_calculator.h"
#include "global_config.h"

MatrixGroup MatrixSpawner::spawn(int matrix_id, const Building& b, int dronesForBuilding) {
    auto dims = MissionCalculator::calculateOptimalDimensions(b, dronesForBuilding);
    Vector2 spawnTarget = { b.bounds.position.x + b.bounds.size.x / 2.0f, 1500.0f };
    
    MatrixGroup matrix(matrix_id, spawnTarget, dims.cols, FORMATION_SPACING_X, dims.rows, FORMATION_SPACING_Y);
    matrix.target_count = dronesForBuilding;
    
    return matrix;
}
