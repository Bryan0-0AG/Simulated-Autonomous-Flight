#ifndef PROCEDURAL_CITY_H
#define PROCEDURAL_CITY_H

#include <vector>
#include <SFML/Graphics.hpp>
#include "utils/vector2.h"

enum class BuildingType {
    OBSTACLE,   // Normal building (Gray)
    CHARGER,    // Charging zone (Blue)
    COLLECTION, // Collection zone (Pink)
    DEPLOY,     // Deploy zone (Purple)
    SPAWN       // Spawn/Launch zone (Yellow)
};

struct Building {
    sf::FloatRect bounds;
    BuildingType type;
    float height; // For future 3D visualizations or flight layers
    
    // Mission State
    bool is_mission_active = false;
    int pending_packages = 0;
    int target_building_idx = -1;
};

class ProceduralCity {
public:
    ProceduralCity(float width, float height);
    
    void generate(unsigned int seed);
    const std::vector<Building>& getBuildings() const { return buildings; }
    std::vector<Building>& getMutableBuildings() { return buildings; }
    float getWidth() const { return worldWidth; }
    float getHeight() const { return worldHeight; }
    
private:
    float worldWidth, worldHeight;
    std::vector<Building> buildings;
    
    void addBuilding(float x, float y, float w, float h, BuildingType type);
};

#endif
