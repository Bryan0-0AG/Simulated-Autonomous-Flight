#ifndef PROCEDURAL_CITY_H
#define PROCEDURAL_CITY_H

#include <vector>
#include <SFML/Graphics.hpp>
#include "utils/vector2.h"

enum class BuildingType {
    OBSTACLE,   // Normal building (Gray)
    CHARGER,    // Charging zone (Blue)
    COLLECTION, // Collection zone (Green)
    SPAWN       // Spawn/Launch zone (Yellow/Orange)
};

struct Building {
    sf::FloatRect bounds;
    BuildingType type;
    float height; // For future 3D visualizations or flight layers
};

class ProceduralCity {
public:
    ProceduralCity(float width, float height);
    
    void generate(unsigned int seed);
    const std::vector<Building>& getBuildings() const { return buildings; }
    float getWidth() const { return worldWidth; }
    float getHeight() const { return worldHeight; }
    
private:
    float worldWidth, worldHeight;
    std::vector<Building> buildings;
    
    void addBuilding(float x, float y, float w, float h, BuildingType type);
};

#endif
