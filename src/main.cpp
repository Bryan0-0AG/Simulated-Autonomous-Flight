#include <iostream>
#include "physics/motion.h"
#include "physics/physics_config.h"
#include "utils/vector2.h"
#include "environment/world.h"
#include "body.h"

int main() {
    World world;
    Body body;   
    body.mass = 1.0f;    

    float steps_per_second = 1.0f / DT;
    int total_seconds = 30;

    apply_impulse(body, {10.0f, 50.0f});
    
    std::cout << "Initial Position: " << body.position.x << ", " << body.position.y << std::endl;
    std::cout << "Initial Velocity: " << body.velocity.x << ", " << body.velocity.y << std::endl;
    std::cout << "Delta Time: " << DT << std::endl;
    std::cout << "Gravity: " << GRAVITY << std::endl;
    std::cout << "Total seconds: " << total_seconds << std::endl;
    std::cout << "----------------------------------------" << std::endl;       
    
    int steps = total_seconds * steps_per_second;    
    float accumulator = 0.0f;
    float seconds_passed = 0.0f;
    bool grounded = false;

    for (int i = 0; i < steps; i++) {
        apply_forces(body, grounded);
        update_motion(body, DT);
        grounded = check_ground_collision(body, world);

        accumulator += DT;
        if (accumulator >= 1.0f) {
            std::cout << " Second " << seconds_passed << ": " <<
                      " | Position: " << body.position.x << ", " << body.position.y << 
                      " | Velocity: " << body.velocity.x << ", " << body.velocity.y << std::endl;
            accumulator = 0.0f;
            seconds_passed += 1.0f;
        }
    }

    return 0;
}