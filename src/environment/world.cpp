#include <iostream>
#include "environment/world.h"
#include "utils/vector2.h"
#include "global_config.h"
#include "body.h"

bool check_ground_collision(Body& body, World& world) {    
    bool currently_grounded = false;
    float realgroundY = world.groundY + body.size;

    // 1. Resolve penetration and bounce
    if (body.position.y < realgroundY) {
        body.position.y = realgroundY;

        if (body.velocity.y < 0) {
            // Resting contact threshold
            if (std::abs(body.velocity.y) < 0.2f) {
                body.velocity.y = 0.0f;
                currently_grounded = true;
            } else {
                body.velocity.y = -body.velocity.y * ENERGY_RESTITUTION;
                currently_grounded = false; // We are bouncing!
            }
        }
    } else if (std::abs(body.position.y - world.groundY) <= COLLISION_EPSILON && std::abs(body.velocity.y) < 0.01f) {
        // Already at ground level and stationary
        currently_grounded = true;
    }

    return currently_grounded;
}