#include <iostream>
#include "physics/motion.h"
#include "physics/physics_config.h"
#include "utils/vector2.h"
#include "environment/world.h"
#include "body.h"
#include "rendering/BasicRenderer.h"

int main() {
    World world;

    Body body;
    body.mass     = 1.0f;
    body.position = {0.0f, 0.0f};
    body.size     = 10.0f;
    body.color[0] = 0;    // R
    body.color[1] = 255;  // G
    body.color[2] = 0;    // B

    apply_impulse(body, {50.0f, 50.0f});

    BasicRenderer renderer({800, 600});

    bool  grounded       = false;
    float accumulator    = 0.0f;
    float seconds_passed = 0.0f;  // ← fuera del bucle

    while (renderer.isOpen()) {
        renderer.handleEvents();

        // Física
        apply_forces(body, grounded);
        update_motion(body, DT);
        grounded = check_ground_collision(body, world);

        // Log por segundo
        accumulator += DT;
        if (accumulator >= 1.0f) {
            std::cout << "Second " << seconds_passed
                      << " | pos: " << body.position.x << ", " << body.position.y
                      << " | vel: " << body.velocity.x << ", " << body.velocity.y
                      << std::endl;
            accumulator    -= 1.0f;  // ← restar, no resetear a 0 (más preciso)
            seconds_passed += 1.0f;
        }

        // Render
        renderer.clear();
        renderer.updateBody(body);
        renderer.display();
    }

    return 0;
}