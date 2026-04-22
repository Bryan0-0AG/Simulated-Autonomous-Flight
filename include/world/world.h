#ifndef WORLD_CLASS_H
#define WORLD_CLASS_H

#include <SFML/Graphics.hpp>
#include <vector>
#include "utils/vector2.h"

// Clase world en minúsculas siguiendo la nueva convención
class world {
public:
    world(Vector2 windowSize);

    // Dibuja todos los elementos estáticos del entorno
    void draw(sf::RenderWindow& window) const;

    // Física de colisiones
    bool resolveGroundCollision(class Body& body) const;

private:
    float groundLevel; // Y=0 físico en coordenadas de pantalla
    sf::RectangleShape groundShape;
    sf::RectangleShape groundLine;
};

#endif // WORLD_CLASS_H
