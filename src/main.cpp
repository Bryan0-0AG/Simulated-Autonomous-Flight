// Physics
#include "physics/motion.h"
#include "physics/physics_config.h"
#include "physics/forces.h"
// Control
#include "vehicle/control_config.h"
#include "vehicle/controller.h"
// Rendering
#include "rendering/BasicRenderer.h"
#include "environment/world.h"
#include "body.h"
// Telemetry
#include "telemetry/telemetry_logger.h"
// Libraries / Utils
#include "utils/vector2.h"
#include "utils/math_utils.h"
#include <iostream>
#include <vector>

int main() {
    World world;
    BasicRenderer renderer({800, 600});
    TelemetryLogger logger;

    std::vector<Body> bodies;

    float accumulator    = 0.0f;
    float seconds_passed = 0.0f;

    for(int i = 0; i < 10; i++) {    
        Body body;
        body.mass     = 1.0f;
        body.position = {0.0f, 0.0f};
        body.size     = 10.0f;
        body.color[0] = randint(50, 255); // R
        body.color[1] = randint(50, 255); // G
        body.color[2] = randint(50, 255); // B
        body.id       = i;
        body.target   = {
            static_cast<float>(randint(40, 500)),
            static_cast<float>(randint(40, 500))
        };
        body.controller = Controller();
        bodies.push_back(body);
    }

    while (renderer.isOpen()) {
        renderer.handleEvents();

        // Control -> Física
        for(auto& body : bodies) {  
            ActuatorOutput control_output = body.controller.update(body.target, body.position, DT);
            Vector2 thrust_force = compute_thrust(control_output);         
          
            body.grounded = check_ground_collision(body, world);
            apply_forces(body, thrust_force);
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
                // Console
                std::cout << " Body " << body.id
                        << "\n\t | pos: " << body.position.x << ", " << body.position.y
                        << "\n\t | vel: " << body.velocity.x << ", " << body.velocity.y
                        << "\n\t | target: " << body.target.x << ", " << body.target.y
                        << std::endl;

                // Telemetry
                logger.log(
                    seconds_passed,
                    body.id,
                    body.position.x,
                    body.position.y,
                    body.velocity.x,
                    body.velocity.y
                );
            }

            // Update time
            accumulator    -= 1.0f;  // ← restar, no resetear a 0 (más preciso)
            seconds_passed += 1.0f;  
        }        
    }
    return 0;
}