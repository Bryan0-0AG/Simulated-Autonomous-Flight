#include <iostream>
#include "physics/motion.h"
#include "physics/physics_config.h"
#include "utils/vector2.h"
#include "environment/world.h"

int main() {
    Vector2 position = {0.0f, 0.0f};
    Vector2 velocity = {10.0f, 50.0f};
    World world;

    std::cout << "Initial Position: " << position.x << ", " << position.y << std::endl;
    std::cout << "Initial Velocity: " << velocity.x << ", " << velocity.y << std::endl;
    std::cout << "Delta Time: " << DT << std::endl;
    std::cout << "Gravity: " << GRAVITY << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    float steps_per_second = 1.0f / DT;
    int total_seconds = 20;
    int steps = total_seconds * steps_per_second;
    
    float accumulator = 0.0f;
    float seconds_passed = 0.0f;
    for (int i = 0; i < steps; i++) {
        update_physics(position, velocity, DT);
        check_ground_collision(position, velocity, world);

        accumulator += DT;
        if (accumulator >= 1.0f) {
            std::cout << " Second " << seconds_passed << ": " <<
                      " | Position: " << position.x << ", " << position.y << 
                      " | Velocity: " << velocity.x << ", " << velocity.y << std::endl;
            accumulator = 0.0f;
            seconds_passed += 1.0f;
        }
    }

    return 0;
}