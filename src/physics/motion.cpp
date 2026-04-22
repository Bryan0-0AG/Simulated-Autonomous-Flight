#include <iostream>
#include <cmath>
#include "physics/motion.h"
#include "global_config.h"
#include "physics/forces.h"
#include "utils/vector2.h"
#include "utils/math_utils.h"
#include "body.h"

void apply_impulse(Body& body, Vector2 impulse) {
    Vector2 delta_velocity;
    delta_velocity.x = impulse.x / body.mass;
    delta_velocity.y = impulse.y / body.mass;
    
    body.velocity.x += delta_velocity.x;
    body.velocity.y += delta_velocity.y;
}

void apply_forces(Body& body, Vector2 thrust, Vector2 separation) {
    body.force = {0.0f, 0.0f};

    body.f_gravity    = compute_gravity(body);
    body.f_drag       = compute_drag(body);
    body.f_thrust     = thrust;
    body.f_separation = separation;

    body.force = body.f_gravity + body.f_drag + body.f_thrust + body.f_separation;
}

void update_motion(Body& body, float dt) {
    // 1. Calculate acceleration = Force / Mass
    body.acceleration.x = body.force.x / body.mass;
    body.acceleration.y = body.force.y / body.mass;

    // 2. Integrate velocity
    body.velocity.x += body.acceleration.x * dt;
    body.velocity.y += body.acceleration.y * dt;

    // --- CAP VELOCITY (Optimization for 10k drones) ---
    float speedSq = body.velocity.x * body.velocity.x + body.velocity.y * body.velocity.y;
    if (speedSq > MAX_VELOCITY * MAX_VELOCITY) {
        float speed = std::sqrt(speedSq);
        body.velocity.x = (body.velocity.x / speed) * MAX_VELOCITY;
        body.velocity.y = (body.velocity.y / speed) * MAX_VELOCITY;
    }

    snap_zero(body.velocity.x);
    snap_zero(body.velocity.y);

    // 3. Apply Ground Friction
    if (std::abs(body.position.y - body.size) < GROUND_EPSILON && std::abs(body.velocity.y) < GROUND_EPSILON) {
        float friction_impulse = GROUND_FRICTION * GRAVITY * dt;
        
        if (std::abs(body.velocity.x) > 0.0f) {
            float speedX = std::abs(body.velocity.x);
            float new_speed = speedX - friction_impulse;
            
            if (new_speed < 0.0f) {
                body.velocity.x = 0.0f;
            } else {
                float direction = (body.velocity.x > 0) ? 1.0f : -1.0f;
                body.velocity.x = new_speed * direction;
            }
        }
    }

    // 4. Integrate position
    body.position.x += body.velocity.x * dt;
    body.position.y += body.velocity.y * dt;
}