#ifndef WORLD_CLASS_H
#define WORLD_CLASS_H

#include <SFML/Graphics.hpp>
#include <vector>
#include "utils/vector2.h"

// Clase world en minÃºsculas siguiendo la nueva convenciÃ³n
class world {
public:
    world(Vector2 windowSize);

    // Dibuja todos los elementos estÃ¡ticos del entorno
    void draw(sf::RenderWindow& window) const;

    // FÃ­sica de colisiones
    bool resolveGroundCollision(class DroneChassis& drone) const;

private:
    float groundLevel; // Y=0 fÃ­sico en coordenadas de pantalla
    sf::RectangleShape groundShape;
    sf::RectangleShape groundLine;
};

#endif // WORLD_CLASS_H
