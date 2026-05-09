#pragma once
#include "AI/matrix/matrix_group.h"
#include "shared/world/procedural_city.h"

class MatrixSpawner {
public:
    static MatrixGroup spawn(int matrix_id, const Building& b, int dronesForBuilding);
};
