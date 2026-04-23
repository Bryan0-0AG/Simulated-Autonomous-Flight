// Physics
#include "physics/motion.h"
#include "physics/forces.h"
// Control
#include "control/controller.h"
// Rendering
#include "rendering/BasicRenderer.h"
#include "world/world.h"
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
// Network
#include "network/bridge.h"

int main() {
    world virtualWorld(WINDOW_SIZE);
    BasicRenderer renderer(WINDOW_SIZE);
    TelemetryLogger logger;
    SpatialGrid grid(WINDOW_SIZE.x, WINDOW_SIZE.y, SEPARATION_RADIUS);
    NetworkBridge bridge;
    sf::Clock clock;

    bridge.connect(SERVER_IP, SERVER_PORT);

    std::vector<Body> bodies;
    bodies.reserve(DRONE_COUNT); // Esto asegura que el vector no se mueva de sitio

    float accumulator    = 0.0f;
    float seconds_passed    = 0.0f;
    float total_time        = 0.0f;
    float last_brain_update = 0.0f; // Rastrear el último envío a la IA

    while (renderer.isOpen()) {
        float frameTime = clock.restart().asSeconds();
        total_time += frameTime;                

        renderer.handleEvents();        

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
            body.grounded = virtualWorld.resolveGroundCollision(body);
            apply_forces(body, thrust_force, sepForce);
            update_motion(body, DT);                                                 
        }

        // --- RENDERIZADO OPTIMIZADO (GPU) ---
        renderer.clear(total_time);
        renderer.drawWorld(virtualWorld);
        renderer.drawSwarm(bodies); // Dibujamos todos de un solo golpe
        renderer.display();

        // --- CÁMARA MODULAR ---
        renderer.updateCamera(bodies);

        // Logic per second
        accumulator += frameTime;        
        while (accumulator >= 1.0f) {
            accumulator -= 1.0f;
            seconds_passed++;
            std::cout << "\n" << std::string(100, '-') << "\n" << std::endl;
            std::cout << "Real Second: " << seconds_passed << " | Drones: " << bodies.size() << std::endl;

            // --- BATCH INTERVAL SPAWNING ---
            if (static_cast<int>(seconds_passed) % SPAWN_INTERVAL == 0 && bodies.size() < DRONE_COUNT) {             
                int current_batch = GRID_COLS;
                
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
                    float x_pos = i * (SEPARATION_RADIUS * 1.2f) + body.size;
                    body.position = {x_pos, body.size};
        
                    // --- TARGET EN FORMACIÓN (GRID NAVIGATION) ---
                    // Los primeros drones (IDs bajos) van a las filas más altas
                    int drones_per_row = GRID_COLS;
                    int total_formation_rows = DRONE_COUNT / drones_per_row;
                    
                    int current_row_in_formation = body.id / drones_per_row;
                    int target_col = body.id % drones_per_row;
                    
                    // Invertimos: (Total - Fila Actual) para que el ID 0 sea la fila más alta
                    int target_row = (total_formation_rows - current_row_in_formation - 1) + GRID_ROWS_OFFSET;
                    
                    body.target = grid.getCellByCoord(target_col, target_row);
                    
                    body.original_target = body.target;
                    body.controller = Controller();
        
                    bodies.push_back(body);
                }
                std::cout << ">>> Despegando lote de " << current_batch << " drones. Total: " << bodies.size() << std::endl;
            }
                 
            // --- STATISTICS GENERATION ---
            int critical_drones = 0;
            int drones_in_mission = 0;

            float total_battery = 0.0f;       
            float total_speed = 0.0f;
            float total_dist_to_target = 0.0f;            
            for(const auto& b : bodies) {
                total_battery += b.battery;
                if(b.battery < 20.0f) critical_drones++;

                float speed = std::sqrt(b.velocity.x * b.velocity.x + b.velocity.y * b.velocity.y);
                total_speed += speed;

                if(b.current_action == DroneAction::FLYING_TO_TARGET) {
                    drones_in_mission++;
                    float dx = b.target.x - b.position.x;
                    float dy = b.target.y - b.position.y;
                    total_dist_to_target += std::sqrt(dx*dx + dy*dy);
                }
            }

            float avg_battery = (bodies.empty()) ? 0.0f : (total_battery / bodies.size());
            float avg_speed   = (bodies.empty()) ? 0.0f : (total_speed / bodies.size());
            float avg_dist    = (drones_in_mission == 0) ? 0.0f : (total_dist_to_target / drones_in_mission);

            // --- ENVIAR ESTADÍSTICAS AL CEREBRO (PYTHON) ---
            if (total_time - last_brain_update >= 10.0f) {                
                bridge.sendSwarmStatus(
                    bodies.size(), 
                    critical_drones, 
                    drones_in_mission, 
                    avg_battery, 
                    avg_speed, 
                    avg_dist);
                last_brain_update = total_time; // Reiniciar el cronómetro
            }

            // AI
            for(auto& body : bodies) {                
                update_ai_decisions(body, virtualWorld);
            }

            // --- SELECTIVE TELEMETRY ---                       
            for(auto& body : bodies) {                
                if (body.id == 0) {
                    std::cout << " Body " << body.id
                            << "\n\t | pos: " << body.position.x << ", " << body.position.y
                            << "\n\t | vel: " << body.velocity.x << ", " << body.velocity.y
                            << "\n\t | target: " << body.target.x << ", " << body.target.y
                            << "\n\t | action: " << toString(body.current_action)
                            << "\n\t | state: " << toString(body.current_state)
                            << "\n\t | battery: " << body.battery << "/" << body.max_battery
                        << std::endl;
                }
                
                if (body.id < 9) {
                    logger.log(total_time, bodies.size(), body);
                }
            } 
        }        
    }
    return 0;
}