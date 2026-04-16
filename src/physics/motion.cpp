#include <iostream>
#include "physics/motion.h"
#include "physics/physics_config.h"
#include "utils/vector2.h"
#include "body.h"

void apply_impulse(Body& body, Vector2 impulse) {
    // DeltaV = Impulse / Mass
    Vector2 delta_velocity;
    delta_velocity.x = impulse.x / body.mass;
    delta_velocity.y = impulse.y / body.mass;
    
    body.velocity.x += delta_velocity.x;
    body.velocity.y += delta_velocity.y;
}

void apply_forces(Body& body, bool is_grounded) {
    // Reset forces for current step
    body.force = {0.0f, 0.0f};

    // 1. Air Resistance - Opposes all motion
    body.force.x -= AIR_FRICTION * body.velocity.x;
    body.force.y -= AIR_FRICTION * body.velocity.y;

    // 2. Gravity (only if in the air)
    if (!is_grounded) {
        body.force.y -= body.mass * GRAVITY;
    }
}

void update_motion(Body& body, float dt) {
    // 1. Calculate acceleration = Force / Mass
    Vector2 acceleration;
    acceleration.x = body.force.x / body.mass;
    acceleration.y = body.force.y / body.mass;

    // 2. Integrate velocity
    body.velocity.x += acceleration.x * dt;
    body.velocity.y += acceleration.y * dt;

    // 3. Apply Ground Friction
    // Using a more robust method: if grounded, apply friction that can't reverse direction
    if (std::abs(body.position.y - 0.0f) < GROUND_EPSILON && std::abs(body.velocity.y) < GROUND_EPSILON) {
        float friction_impulse = GROUND_FRICTION * GRAVITY * dt;
        
        if (std::abs(body.velocity.x) > 0.0f) {
            float speed = std::abs(body.velocity.x);
            float new_speed = speed - friction_impulse;
            
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