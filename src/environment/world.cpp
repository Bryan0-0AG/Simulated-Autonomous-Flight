#include <iostream>
#include "environment/world.h"
#include "utils/vector2.h"
#include "physics/physics_config.h"

bool check_ground_collision(Vector2& position, Vector2& velocity, World& world) {

    if (position.y <= world.groundY && velocity.y < 0) {
        std::cout << "Ground collision!" << std::endl;
        position.y = world.groundY;
        velocity.y = -velocity.y * ENERGY_RESTITUTION;
        return true;
    }

    return false;
}