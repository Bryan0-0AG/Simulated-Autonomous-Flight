#ifndef WORLD_CLASS_H
#define WORLD_CLASS_H

#include <SFML/Graphics.hpp>
#include <vector>
#include "utils/vector2.h"

// world class in lowercase following the new convention
class world {
public:
    world(Vector2 windowSize);

    // Draws all static environment elements
    void draw(sf::RenderWindow& window) const;

    // Collision physics
    bool resolveGroundCollision(class DroneChassis& drone) const;

private:
    float groundLevel; // physical Y=0 in screen coordinates
    sf::RectangleShape groundShape;
    sf::RectangleShape groundLine;
};

#endif // WORLD_CLASS_H
