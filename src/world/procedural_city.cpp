#include "world/procedural_city.h"
#include <cstdlib>
#include <ctime>

ProceduralCity::ProceduralCity(float width, float height) 
    : worldWidth(width), worldHeight(height) {}

void ProceduralCity::generate(unsigned int seed) {
    std::srand(seed);
    buildings.clear();

    float currentX = 100.0f; // Empezamos un poco después del borde
    while (currentX < worldWidth - 200.0f) {
        
        // 1. Decidir si hay un edificio o un espacio vacío (calle)
        float gap = 20.0f + (std::rand() % 100); // Espacio entre edificios
        currentX += gap;

        if (std::rand() % 100 < 80) { // 80% de probabilidad de edificio
            float bWidth = 80.0f + (std::rand() % 200);
            float bHeight = 150.0f + (std::rand() % 600); // Edificios de hasta 600m de altura
            
            BuildingType type = BuildingType::OBSTACLE;
            int r = std::rand() % 100;
            if (r < 10) type = BuildingType::CHARGER;
            else if (r < 20) type = BuildingType::COLLECTION;

            // Todos inician en Y = 0 (el suelo)
            addBuilding(currentX, 0.0f, bWidth, bHeight, type);
            
            currentX += bWidth;
        } else {
            // Un espacio vacío más grande (una plaza o calle ancha)
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
