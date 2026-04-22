// Physics
#include "physics/motion.h"
#include "physics/forces.h"
// Control
#include "control/controller.h"
// Rendering
#include "rendering/BasicRenderer.h"
#include "environment/world.h"
#include "body.h"
// Telemetry
#include "telemetry/telemetry_logger.h"
// Libraries / Utils
#include "global_config.h"
#include "utils/vector2.h"
#include "utils/math_utils.h"
#include "utils/spatial_grid.h"
#include <iostream>
#include <vector>
#include <string>
// AI
#include "AI/decisions.h"

int main() {
    World world;
    BasicRenderer renderer(WORLD_SIZE);
    TelemetryLogger logger;
    SpatialGrid grid(WORLD_SIZE.x, WORLD_SIZE.y, SEPARATION_RADIUS); // Ancho, Alto, Tamaño de celda

    std::vector<Body> bodies;
    bodies.reserve(DRONE_COUNT); // Esto asegura que el vector no se mueva de sitio

    float accumulator    = 0.0f;
    float seconds_passed = 0.0f;
    float total_time     = 0.0f;


    sf::Clock clock;

    while (renderer.isOpen()) {
        renderer.handleEvents();
        
        float frameTime = clock.restart().asSeconds();

        // --- CONTROL ESPACIAL ---
        grid.clear();
        for(auto& body : bodies) {
            grid.addBody(&body);
        }

        // --- FÍSICA E IA ---
        for(auto& body : bodies) {  
            // Control espacial
            auto neighbors = grid.getNeighbors(body);
            Vector2 sepForce = compute_separation(body, neighbors);            
            
            // Control PID
            ActuatorOutput control_output = body.controller.update(body, DT);
            Vector2 thrust_force = compute_thrust(control_output);         
          
            // Física
            body.grounded = check_ground_collision(body, world);
            apply_forces(body, thrust_force, sepForce);
            update_motion(body, DT);                                                 
        }

        // --- RENDERIZADO OPTIMIZADO (GPU) ---
        renderer.clear(total_time);
        renderer.drawSwarm(bodies); // Dibujamos todos de un solo golpe
        renderer.display();

        total_time += frameTime;

        // Logic per second
        accumulator += frameTime;        
        if(accumulator >= 1.0f) {
            accumulator -= 1.0f;
            seconds_passed++;
            std::cout << "\n" << std::string(100, '-') << "\n" << std::endl;
            std::cout << "Real Second: " << seconds_passed << " | Drones: " << bodies.size() << std::endl;

            // SPAWN N DRONES EVERY SPAWN_INTERVAL SECONDS         
            if ((int)seconds_passed % SPAWN_INTERVAL == 0 && bodies.size() < DRONE_COUNT) {             
                int current_batch = DRONE_BATCH_SIZE;
                float INITIAL_DRONE_SPACING = WORLD_SIZE.x / current_batch; 
                
                // Ensure we don't exceed the maximum
                if (bodies.size() + current_batch > DRONE_COUNT) {
                    current_batch = DRONE_COUNT - bodies.size();
                }

                // Calculate horizontal gap to spread drones across the entire screen width                
                for (int i = 0; i < current_batch; i++) {
                    Body body;
                    body.id = static_cast<int>(bodies.size()); 
                    body.mass = 1.0f;
                    body.size = 4.0f;
        
                    // Position drones evenly on the ground
                    float x_pos = (i * INITIAL_DRONE_SPACING) + (INITIAL_DRONE_SPACING / 2.0f);
                    body.position = {x_pos, body.size};
        
                    // --- TARGET EN FORMACIÓN (GRID NAVIGATION) ---
                    // Cada dron tiene asignada una celda única basada en su ID
                    int target_col = body.id % GRID_COLS;
                    int target_row = (body.id / GRID_COLS) + GRID_ROWS_OFFSET; // Offset de 8 filas para elevar la formación
                    
                    body.target = grid.getCellByCoord(target_col, target_row);
                    
                    body.original_target = body.target;
                    body.controller = Controller();
        
                    bodies.push_back(body);
                }
                std::cout << ">>> Despegando lote de " << current_batch << " drones. Total: " << bodies.size() << std::endl;
            }
                                                                                                                                                
                                                                                                                                                                                                                                                                                                            
            // AI
            for(auto& body : bodies) {                
                update_ai_decisions(body, world);
            }

            // --- SELECTIVE TELEMETRY ---
            for(auto& body : bodies) {
                if (body.id == 0) { // Solo mostramos 1 dron en consola para no saturar la pantalla
                    std::cout << " Body " << body.id
                            << "\n\t | pos: " << body.position.x << ", " << body.position.y
                            << "\n\t | vel: " << body.velocity.x << ", " << body.velocity.y
                            << "\n\t | target: " << body.target.x << ", " << body.target.y
                            << "\n\t | action: " << toString(body.current_action)
                            << "\n\t | state: " << toString(body.current_state)
                            << "\n\t | battery: " << body.battery << "/" << body.max_battery
                        << std::endl;
                }
                if (body.id < 9) { // Guardamos registros de los primeros 10 drones                                                                                                                                                                                                                                                                                                                                  // Log
                    logger.log(total_time, bodies.size(), body);
                }
            }

            // Update time
            seconds_passed += 1.0f;  
        }        
    }
    return 0;
}