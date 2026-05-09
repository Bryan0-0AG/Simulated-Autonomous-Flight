#include "missions/mission_calculator.h"
#include "global_config.h"
#include "utils/math_utils.h"
#include <algorithm>

std::vector<int> MissionCalculator::findBestSpawnBuildings(const ProceduralCity& city, Vector2 target, int count) {
    const auto& buildings = city.getBuildings();
    std::vector<BuildingCandidate> candidates;

    for (int i = 0; i < (int)buildings.size(); ++i) {
        if (buildings[i].type == BuildingType::SPAWN) {
            float dx = buildings[i].bounds.position.x - target.x;
            float dy = buildings[i].bounds.position.y - target.y;
            candidates.push_back({i, dx*dx + dy*dy});
        }
    }

    if (candidates.empty()) return {};

    std::sort(candidates.begin(), candidates.end(), [](const BuildingCandidate& a, const BuildingCandidate& b) {
        return a.distanceSq < b.distanceSq;
    });

    std::vector<int> result;
    int limit = std::min((int)candidates.size(), count);
    for (int i = 0; i < limit; ++i) {
        result.push_back(candidates[i].index);
    }
    return result;
}

MissionCalculator::MatrixDimensions MissionCalculator::calculateOptimalDimensions(const Building& b, int dronesForBuilding) {
    int batch = std::max(1, static_cast<int>(b.bounds.size.x / 20.0f));
    int cols = findClosestExact(batch, DRONES_PER_ROW);
    int rows = (dronesForBuilding + cols - 1) / cols;
    
    return {rows, cols, batch};
}