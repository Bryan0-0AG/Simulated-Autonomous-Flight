#include "physics/motion.h"
#include "physics/physics_config.h"
#include "utils/vector2.h"
#include "utils/math_utils.h"
#include "environment/world.h"
#include "body.h"
#include "rendering/BasicRenderer.h"
#include <iostream>
#include <vector>

int main() {
    World world;
    BasicRenderer renderer({800, 600});

    std::vector<Body> bodies;

    float accumulator    = 0.0f;
    float seconds_passed = 0.0f;

    for(int i = 0; i < 10; i++) {    
        Body body;
        body.mass     = 1.0f;
        body.position = {0.0f, 0.0f};
        body.size     = 10.0f;
        body.color[0] = randint(0, 255); // R
        body.color[1] = randint(0, 255); // G
        body.color[2] = randint(0, 255); // B
        body.id       = i;
        bodies.push_back(body);

        Vector2 impulse = {
            static_cast<float>(randint(30, 70)),
            static_cast<float>(randint(30, 70))
        };

        apply_impulse(bodies.back(), impulse);
    }

    while (renderer.isOpen()) {
        renderer.handleEvents(); 
        
        // Física
        for(auto& body : bodies) {            
            body.grounded = check_ground_collision(body, world);
            apply_forces(body, body.grounded);
            update_motion(body, DT);            
        }
        
        // Render
        renderer.clear();
        for(auto& body : bodies) {
            renderer.updateBody(body);
        }
        renderer.display();

        // Log por segundo
        accumulator += DT;
        if (accumulator >= 1.0f) {
            std::cout << "\n" << std::string(100, '-') << "\n" << std::endl;
            std::cout << "Second " << seconds_passed << "\n" << std::endl;
            for(auto& body : bodies) {
                std::cout << " Body " << body.id
                        << "\n\t | pos: " << body.position.x << ", " << body.position.y
                        << "\n\t | vel: " << body.velocity.x << ", " << body.velocity.y
                        << std::endl;
            }
            accumulator    -= 1.0f;  // ← restar, no resetear a 0 (más preciso)
            seconds_passed += 1.0f;        
        }        
    }
    return 0;
}