#include <iostream>
#include <cmath>
#include <vector>
#include "global_config.h"
#include "utils/vector2.h"
#include "utils/math_utils.h"
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

Vector2 compute_separation(const Body& body, const std::vector<Body*>& neighbors) {
    Vector2 separationForce = {0.0f, 0.0f};
    int count = 0;

    for (const auto& neighborPtr : neighbors) {
        if (&body == neighborPtr) continue;

        float d = distance(body.position, neighborPtr->position);
        if (d > 0 && d < SEPARATION_RADIUS) {
            Vector2 diff = body.position - neighborPtr->position;
            // La fuerza es inversamente proporcional al cuadrado de la distancia
            separationForce += diff / (d * d); 
            count++;
        }
    }

    if (count > 0) {
        separationForce = separationForce * SEPARATION_FORCE;
    }

    return separationForce;
}