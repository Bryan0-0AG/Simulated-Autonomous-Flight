#ifndef PROCEDURAL_CITY_H
#define PROCEDURAL_CITY_H

#include <vector>
#include <SFML/Graphics.hpp>
#include "utils/vector2.h"

enum class BuildingType {
    OBSTACLE,   // Edificio normal (Gris)
    CHARGER,    // Zona de carga (Azul)
    COLLECTION  // Zona de recolección (Verde)
};

struct Building {
    sf::FloatRect bounds;
    BuildingType type;
    float height; // Para futuras visualizaciones 3D o capas de vuelo
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
