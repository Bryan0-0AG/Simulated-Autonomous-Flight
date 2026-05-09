#pragma once
#include <vector>
#include "world/procedural_city.h"
#include "utils/vector2.h"

class MissionCalculator {
public:
    struct BuildingCandidate {
        int index;
        float distanceSq;
    };

    struct MatrixDimensions {
        int rows;
        int cols;
        int batchSize;
    };

    static std::vector<int> findBestSpawnBuildings(const ProceduralCity& city, Vector2 target, int count);
    static MatrixDimensions calculateOptimalDimensions(const Building& b, int dronesForBuilding);
};
