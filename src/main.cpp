#include <iostream>
#include "physics/motion.h"

int main() {
    float position = 0.0f;
    float velocity = 10.0f;
    float dt = 1.0f; // delta time

    for (int i = 0; i < 50; i++) {
        update_position(position, velocity, dt);

        std::cout << "Step " << i 
                  << " | Position: " << position << std::endl;
    }

    return 0;
}