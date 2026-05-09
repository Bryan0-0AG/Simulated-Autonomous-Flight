#include "world/procedural_city.h"
#include "global_config.h"
#include <cstdlib>
#include <ctime>

ProceduralCity::ProceduralCity(float width, float height) 
    : worldWidth(width), worldHeight(height) {}

void ProceduralCity::generate(unsigned int seed) {
    std::srand(seed);
    buildings.clear();

    float currentX = 100.0f; 
    while (currentX < worldWidth - BUILDING_MAX_WIDTH) {
        
        float gap = 20.0f + (std::rand() % 100); 
        currentX += gap;

        if (std::rand() % 100 < BUILDING_PROBABILITY) {
            float bWidth = BUILDING_MIN_WIDTH + (std::rand() % static_cast<int>(BUILDING_MAX_WIDTH - BUILDING_MIN_WIDTH));
            float bHeight = BUILDING_MIN_HEIGHT + (std::rand() % static_cast<int>(BUILDING_MAX_HEIGHT - BUILDING_MIN_HEIGHT)); 
            
            BuildingType type = BuildingType::OBSTACLE;
            int r = std::rand() % 100;

            if (r < PROB_CHARGER) type = BuildingType::CHARGER;
            else if (r < PROB_CHARGER + PROB_COLLECT) type = BuildingType::COLLECTION;
            else if (r < PROB_CHARGER + PROB_COLLECT + PROB_DEPLOY) type = BuildingType::DEPLOY;
            else if (r < PROB_CHARGER + PROB_COLLECT + PROB_DEPLOY + PROB_SPAWN) type = BuildingType::SPAWN;

            addBuilding(currentX, 0.0f, bWidth, bHeight, type);
            currentX += bWidth;
        } else {
            currentX += 150.0f;
        }
    }
}

void ProceduralCity::addBuilding(float x, float y, float w, float h, BuildingType type) {
    Building b;
    b.bounds = sf::FloatRect({x, y}, {w, h});
    b.type = type;
    b.height = static_cast<float>(100 + (std::rand() % 400));
    buildings.push_back(b);
}
