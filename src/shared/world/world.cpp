#include "world/world.h"
#include "drone_dynamics.h"
#include "global_config.h"
#include <cmath>

world::world(Vector2 windowSize) {
    groundLevel = WORLD_SIZE.y;

    // Parametric configuration of the ground
    float groundWidth = WORLD_SIZE.x;
    float groundHeight = 1000.0f;

    groundShape.setSize({groundWidth, groundHeight});
    groundShape.setFillColor(sf::Color(30, 30, 30));
    groundShape.setPosition({0.0f, groundLevel});

    // Neon line
    groundLine.setSize({groundWidth, 4.0f});
    groundLine.setFillColor(sf::Color(0, 255, 255));
    groundLine.setPosition({0.0f, groundLevel - 2.0f});
}

void world::draw(sf::RenderWindow& window) const {
    window.draw(groundShape);
    window.draw(groundLine);
}

bool world::resolveGroundCollision(DroneChassis& drone) const {
    bool currently_grounded = false;
    float realGroundY = 4.0f; // Y=0 is the physical ground level

    // 1. Resolve penetration and bounce
    if (drone.position.y < realGroundY) {
        drone.position.y = realGroundY;

        if (drone.velocity.y < 0) {
            // Threshold for resting contact
            if (std::abs(drone.velocity.y) < 0.2f) {
                drone.velocity.y = 0.0f;
                currently_grounded = true;
            } else {
                drone.velocity.y = -drone.velocity.y * ENERGY_RESTITUTION;
                currently_grounded = false; // We are bouncing!
            }
        }
    } else if (std::abs(drone.position.y - realGroundY) <= COLLISION_EPSILON && std::abs(drone.velocity.y) < 0.01f) {
        // Already on the ground level and still
        currently_grounded = true;
    }

    return currently_grounded;
}
