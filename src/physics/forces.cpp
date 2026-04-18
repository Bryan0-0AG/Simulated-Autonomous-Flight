#include <iostream>
#include <cmath>
#include "physics/physics_config.h"
#include "control/control_config.h"
#include "utils/vector2.h"
#include "body.h"

Vector2 compute_gravity(const Body& body) {
    if (body.grounded) return {0.0f, 0.0f};

    return {0.0f, -body.mass * GRAVITY};
}

Vector2 compute_drag(const Body& body) {
    return {
        -AIR_FRICTION * body.velocity.x, 
        -AIR_FRICTION * body.velocity.y
    };
}

Vector2 compute_thrust(const ActuatorOutput& output) {
    return {
        output.thrust * std::sin(output.angle), 
        output.thrust * std::cos(output.angle)
    };
}