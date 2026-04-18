// Physics
#include "physics/motion.h"
#include "physics/physics_config.h"
#include "physics/forces.h"
// Control
#include "control/control_config.h"
#include "control/controller.h"
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
#include <string>
// AI
#include "AI/decisions.h"

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
        body.id       = i;
        body.target   = {
            static_cast<float>(randint(200, 700)),
            static_cast<float>(randint(100, 500))
        };
        body.original_target = body.target;
        body.controller = Controller();
        bodies.push_back(body);
    }

    while (renderer.isOpen()) {
        renderer.handleEvents();

        // Control -> Physics
        for(auto& body : bodies) {  
            ActuatorOutput control_output = body.controller.update(body, DT);
            Vector2 thrust_force = compute_thrust(control_output);         
          
            body.grounded = check_ground_collision(body, world);
            apply_forces(body, thrust_force);
            update_motion(body, DT);            
        }

        // Render
        renderer.clear();
        for(auto& body : bodies) {
            // Color de batería: Verde (100%) a Rojo (0%)
            float battery_pct = body.battery / body.max_battery;
            if (battery_pct < 0.0f) battery_pct = 0.0f;
            if (battery_pct > 1.0f) battery_pct = 1.0f;
            
            body.color[0] = static_cast<int>(255.0f * (1.0f - battery_pct)); // R
            body.color[1] = static_cast<int>(255.0f * battery_pct);          // G
            body.color[2] = 0;                                               // B

            renderer.updateBody(body);
        }
        renderer.display();

        // Logic per second
        accumulator += DT;        
        if(accumulator >= 1.0f) {
            accumulator -= 1.0f;
            std::cout << "\n" << std::string(100, '-') << "\n" << std::endl;
            std::cout << "Second " << seconds_passed << "\n" << std::endl;

            // Telemetry
            for(auto& body : bodies) {
                // Console
                std::cout << " Body " << body.id
                        << "\n\t | pos: " << body.position.x << ", " << body.position.y
                        << "\n\t | vel: " << body.velocity.x << ", " << body.velocity.y
                        << "\n\t | target: " << body.target.x << ", " << body.target.y
                        << "\n\t | action: " << toString(body.current_action)
                        << "\n\t | state: " << toString(body.current_state)
                        << "\n\t | battery: " << body.battery << "/" << body.max_battery
                        << std::endl;

                // Log
                logger.log(
                    seconds_passed,
                    body.id,
                    body.position.x,
                    body.position.y,
                    body.velocity.x,
                    body.velocity.y,
                    body.target.x,
                    body.target.y,
                    body.actuator_output.thrust,
                    body.actuator_output.angle,
                    body.error_x,
                    body.error_y,
                    body.angle_pid,
                    body.thrust_pid,
                    toString(body.current_action),
                    toString(body.current_state),
                    body.battery,
                    body.max_battery,
                    body.original_target.x,
                    body.original_target.y
                );
            }

            // AI
            for(auto& body : bodies) {                
                update_ai_decisions(body, world);
            }

            // Update time
            seconds_passed += 1.0f;  
        }        
    }
    return 0;
}