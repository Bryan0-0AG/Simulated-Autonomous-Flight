#ifndef DEPLOYMENT_CALCULATOR_H
#define DEPLOYMENT_CALCULATOR_H

#include <vector>
#include <algorithm>
#include <set>
#include <cmath>
#include "world/procedural_city.h"
#include "utils/vector2.h"
#include "global_config.h"
#include "utils/math_utils.h"


struct SpawnBuilding {
    int building_idx;
    int actual_batch;
    int drones_per_batch;
    int total_batches; 
    float xspacing;
    int total_drones;
    int drones_spawned;
};

class DeploymentCalculator {
public:
    struct BuildingCandidate {
        int index;
        float distanceSq;
    };

    // Finds best spawn buildings, excluding occupied ones AND enforcing minimum X spacing
    static std::vector<int> findBestSpawnBuildings(const ProceduralCity& city, Vector2 target, int count,
                                                    const std::set<int>& occupied_buildings = {}) {
        const auto& buildings = city.getBuildings();
        std::vector<BuildingCandidate> candidates;

        for (int i = 0; i < (int)buildings.size(); ++i) {
            if (buildings[i].type != BuildingType::SPAWN) continue;
            // Skip buildings that already have a TAKEOFF matrix above them
            if (occupied_buildings.count(i)) continue;

            float dx = buildings[i].bounds.position.x - target.x;
            float dy = buildings[i].bounds.position.y - target.y;
            candidates.push_back({i, dx*dx + dy*dy});
        }

        if (candidates.empty()) return {};

        std::sort(candidates.begin(), candidates.end(), [](const BuildingCandidate& a, const BuildingCandidate& b) {
            return a.distanceSq < b.distanceSq;
        });

        // Select buildings ensuring minimum X spacing between them
        constexpr float MIN_SPAWN_SPACING = 800.0f; // Minimum X distance between spawn buildings
        std::vector<int> result;
        for (const auto& c : candidates) {
            if ((int)result.size() >= count) break;

            bool tooClose = false;
            for (int selectedIdx : result) {
                float xDist = std::abs(buildings[c.index].bounds.position.x - buildings[selectedIdx].bounds.position.x);
                if (xDist < MIN_SPAWN_SPACING) {
                    tooClose = true;
                    break;
                }
            }
            if (!tooClose) {
                result.push_back(c.index);
            }
        }
        return result;
    }

    struct MatrixDimensions {
        int rows;
        int cols;
        int batchSize;
    };

    static MatrixDimensions calculateOptimalDimensions(const Building& b, int dronesForBuilding) {
        // Calculate cols from total drone count for a roughly square matrix
        // Use sqrt to balance rows vs cols, clamp to DRONES_PER_ROW max
        int idealCols = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(dronesForBuilding))));
        int cols = std::min(idealCols, DRONES_PER_ROW);
        if (cols < 1) cols = 1;
        int rows = (dronesForBuilding + cols - 1) / cols;
        
        // Batch size = 1 full row per spawn wave (clean row filling)
        int batch = std::max(1, static_cast<int>(b.bounds.size.x / 20.0f));
        int rowsPerBatch = std::max(1, batch / cols);
        int alignedBatch = rowsPerBatch * cols;
        
        return {rows, cols, alignedBatch};
    }
};

#endif
