// Physics
#include "HPC/swarm_dynamics.h"
// Rendering
#include "rendering/BasicRenderer.h"
#include "world/procedural_city.h"
#include "world/world.h"
#include "HPC/swarm_dynamics.h"
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

    ProceduralCity city(WORLD_SIZE.x, WORLD_SIZE.y);
    city.generate(static_cast<unsigned int>(std::time(nullptr)));

    bridge.connect(SERVER_IP, SERVER_PORT);

    std::vector<DroneChassis> drones;
    drones.reserve(DRONE_COUNT); 

    float accumulator    = 0.0f;
    float seconds_passed    = 0.0f;
    float total_time        = 0.0f;
    float last_brain_update = 0.0f;

    // --- BUFFER PARA GPU ---
    std::vector<DroneChassis> gpu_data;

    while (renderer.isOpen()) {
        float frameTime = clock.restart().asSeconds();
        total_time += frameTime;                

        renderer.handleEvents();        

        // --- CONTROL ESPACIAL (Cálculo de Separación en CPU) ---
        grid.clear();
        for(auto& drone : drones) {
            grid.addBody(&drone);
        }

        for(auto& drone : drones) {
            Vector2 sep_force = {0.0f, 0.0f};
            auto neighbors = grid.getNeighbors(drone);
            for(auto* neighbor : neighbors) {
                if(drone.id == neighbor->id) continue;
                
                float dx = drone.position.x - neighbor->position.x;
                float dy = drone.position.y - neighbor->position.y;
                float distSq = dx*dx + dy*dy;
                
                if(distSq < SEPARATION_RADIUS * SEPARATION_RADIUS && distSq > 0.01f) {
                    float dist = std::sqrt(distSq);
                    // Fuerza inversamente proporcional a la distancia
                    float strength = (SEPARATION_RADIUS - dist) / SEPARATION_RADIUS;
                    sep_force.x += (dx / dist) * SEPARATION_FORCE * strength;
                    sep_force.y += (dy / dist) * SEPARATION_FORCE * strength;
                }
            }
            drone.f_separation = sep_force;
        }

        // --- GPU PHYSICS (AMD HIP) or CPU FALLBACK ---
        if (!drones.empty()) {
            launch_physics_kernel(drones.data(), drones.size(), DT);
        }

        // --- RENDERIZADO OPTIMIZADO (GPU) ---
        renderer.clear(total_time);
        renderer.drawCity(city); // Dibujamos los edificios primero
        renderer.drawWorld(virtualWorld);
        renderer.drawSwarm(drones); // Dibujamos todos de un solo golpe
        renderer.display();

        // --- MODULAR CAMERA ---
        renderer.updateCamera(drones);

        // Logic per second
        accumulator += frameTime;        
        while (accumulator >= 1.0f) {
            accumulator -= 1.0f;
            seconds_passed++;
            std::cout << "\n" << std::string(100, '-') << "\n" << std::endl;
            std::cout << "Second: " << seconds_passed << " | Drones: " << drones.size() << std::endl;

            // --- BATCH INTERVAL SPAWNING ---
            if (static_cast<int>(seconds_passed) % SPAWN_INTERVAL == 0 && drones.size() < DRONE_COUNT) {             
                int current_batch = GRID_COLS;
                
                // Ensure we don't exceed the maximum
                if (drones.size() + current_batch > DRONE_COUNT) {
                    current_batch = DRONE_COUNT - drones.size();
                }

                // Calculate horizontal gap to spread drones across the entire screen width                
                for (int i = 0; i < current_batch; i++) {
                    // Inicialización limpia para evitar basura en la telemetría
                    DroneChassis drone = {}; 
                    drone.id = static_cast<int>(drones.size()); 
                    drone.mass = 1.0f;
                    drone.battery = 100.0f; // Iniciar con batería llena
                    
                    float size = 4.0f; 
        
                    // Position drones evenly on the ground
                    float x_pos = i * (SEPARATION_RADIUS * 1.5f) + 500.0f + size;
                    drone.position = {x_pos, size};
        
                    // --- GRID NAVIGATION ---
                    int drones_per_row = GRID_COLS;
                    int total_formation_rows = DRONE_COUNT / drones_per_row;
                    
                    int current_row_in_formation = drone.id / drones_per_row;
                    int target_col = drone.id % drones_per_row;
                    
                    int target_row = (total_formation_rows - current_row_in_formation - 1) + GRID_ROWS_OFFSET;
                    
                    drone.target = grid.getCellByCoord(target_col, target_row);
                    drone.original_target = drone.target;
                    drone.current_state = DroneState::FLYING;
        
                    drones.push_back(drone);
                }
                std::cout << ">>> Launching batch of " << current_batch << " drones. Total: " << drones.size() << std::endl;
            }
                 
            // --- STATISTICS GENERATION ---
            int critical_drones = 0;
            int drones_in_mission = 0;

            float total_battery = 0.0f;       
            float total_speed = 0.0f;
            float total_dist_to_target = 0.0f;            
            for(const auto& b : drones) {
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

            float avg_battery = (drones.empty()) ? 0.0f : (total_battery / drones.size());
            float avg_speed   = (drones.empty()) ? 0.0f : (total_speed / drones.size());
            float avg_dist    = (drones_in_mission == 0) ? 0.0f : (total_dist_to_target / drones_in_mission);

            // --- SEND STATISTICS TO BRAIN (PYTHON) ---
            if (total_time - last_brain_update >= LLM_QUERY_INTERVAL) {                
                bridge.sendSwarmStatus(
                    drones.size(), 
                    critical_drones, 
                    drones_in_mission, 
                    avg_battery, 
                    avg_speed, 
                    avg_dist);
                last_brain_update = total_time;
            }

            // AI
            for(auto& drone : drones) {
                virtualWorld.resolveGroundCollision(drone);
                update_ai_decisions(drone, virtualWorld);
            }

            // --- SELECTIVE TELEMETRY ---                       
            for(auto& drone : drones) {                
                if (drone.id == 0) {
                    std::cout << " DroneChassis " << drone.id
                            << "\n\t | pos: " << drone.position.x << ", " << drone.position.y
                            << "\n\t | vel: " << drone.velocity.x << ", " << drone.velocity.y
                            << "\n\t | target: " << drone.target.x << ", " << drone.target.y
                            << "\n\t | action: " << toString(drone.current_action)
                            << "\n\t | state: " << toString(drone.current_state)
                            << "\n\t | battery: " << drone.battery << "/" << drone.max_battery
                        << std::endl;
                }
                
                if (drone.id < 9) {
                    logger.log(total_time, drones.size(), drone);
                }
            } 
        }        
    }
    return 0;
}