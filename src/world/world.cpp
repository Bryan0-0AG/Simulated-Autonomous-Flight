#include "world/world.h"
#include "body.h"
#include "global_config.h"
#include <cmath>

world::world(Vector2 windowSize) {
    groundLevel = windowSize.y;

    // Configuración paramétrica del suelo
    float groundWidth = WORLD_SIZE.x * 2.0f;
    float groundHeight = 1000.0f;

    groundShape.setSize({groundWidth, groundHeight});
    groundShape.setFillColor(sf::Color(30, 30, 30));
    groundShape.setPosition({-WORLD_SIZE.x / 2.0f, groundLevel});

    // Línea de neón
    groundLine.setSize({groundWidth, 4.0f});
    groundLine.setFillColor(sf::Color(0, 255, 255));
    groundLine.setPosition({-WORLD_SIZE.x / 2.0f, groundLevel - 2.0f});
}

void world::draw(sf::RenderWindow& window) const {
    window.draw(groundShape);
    window.draw(groundLine);
}

bool world::resolveGroundCollision(Body& body) const {
    bool currently_grounded = false;
    float realGroundY = body.size; // Y=0 es el nivel físico del suelo

    // 1. Resolver penetración y rebote
    if (body.position.y < realGroundY) {
        body.position.y = realGroundY;

        if (body.velocity.y < 0) {
            // Umbral de contacto en reposo
            if (std::abs(body.velocity.y) < 0.2f) {
                body.velocity.y = 0.0f;
                currently_grounded = true;
            } else {
                body.velocity.y = -body.velocity.y * ENERGY_RESTITUTION;
                currently_grounded = false; // ¡Estamos rebotando!
            }
        }
    } else if (std::abs(body.position.y - realGroundY) <= COLLISION_EPSILON && std::abs(body.velocity.y) < 0.01f) {
        // Ya está en el nivel del suelo y quieto
        currently_grounded = true;
    }

    return currently_grounded;
}
