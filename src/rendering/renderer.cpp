#include "rendering/renderer.h"
#include "world/world.h"
#include "world/procedural_city.h"

#include "global_config.h"
#include <algorithm>

// The constructor initializes the window.
Renderer::Renderer(Vector2 windowSize)
    : window(sf::VideoMode({(unsigned int)windowSize.x, (unsigned int)windowSize.y}), "Flight Simulation"),
      cam(windowSize),
      windowSize(windowSize)
{
    window.setFramerateLimit(60); 
    
    // 1. Load the GPU program (Shader)
    if (!shader.loadFromFile("assets/shaders/entorno.frag", sf::Shader::Type::Fragment)) {
        // If it fails, SFML will print an error in the console
    }

    // 2. Configure the background rectangle that will occupy the entire window
    backgroundRect.setSize({(float)windowSize.x, (float)windowSize.y});
    
    // 3. Pass the initial resolution to the GPU
    shader.setUniform("u_resolution", sf::Vector2f(windowSize.x, windowSize.y));

    shape.setRadius(10.f);
    shape.setFillColor(sf::Color::White);
}

bool Renderer::isOpen() const {
    return window.isOpen();
}

void Renderer::handleEvents() {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }
        
        // Pass the event to the camera for manual control
        cam.handleEvent(*event, window);
    }
}

void Renderer::clear(float totalTime) {
    window.clear(sf::Color::Black);
    
    // Static background (UI/Shader)
    window.setView(window.getDefaultView());
    window.draw(backgroundRect, &shader);

    // Activate camera for drones and world (from the modular camera object)
    window.setView(cam.getView());
}

void Renderer::updateCamera(const std::vector<DroneChassis>& drones) {
    cam.update(drones);
}

void Renderer::display() {
    window.display();
}

void Renderer::drawCity(const ProceduralCity& city) {
    const auto& buildings = city.getBuildings();
    sf::RectangleShape rect;
    
    for (const auto& b : buildings) {
        rect.setSize({b.bounds.size.x, b.bounds.size.y});
        // IMPORTANT: Invert Y using the real world height, not the window height
        rect.setPosition({b.bounds.position.x, city.getHeight() - b.bounds.position.y - b.bounds.size.y});
        
        // Neon Colors according to the type
        if (b.type == BuildingType::CHARGER) {
            rect.setFillColor(sf::Color(0, 255, 255, 180)); // Cyan neon
            rect.setOutlineColor(sf::Color::White);
            rect.setOutlineThickness(2.0f);
        } else if (b.type == BuildingType::COLLECTION) {
            rect.setFillColor(sf::Color(255, 0, 255, 180)); // Magenta neon
            rect.setOutlineColor(sf::Color::White);
            rect.setOutlineThickness(4.0f);
        } else if (b.type == BuildingType::DEPLOY) {
            rect.setFillColor(sf::Color(128, 0, 128, 180)); // Purple neon
            rect.setOutlineColor(sf::Color::White);
            rect.setOutlineThickness(4.0f);
        } else if (b.type == BuildingType::SPAWN) {
            rect.setFillColor(sf::Color(255, 165, 0, 180)); // Orange neon
            rect.setOutlineColor(sf::Color::White);
            rect.setOutlineThickness(2.0f);
        } else {
            rect.setFillColor(sf::Color(50, 50, 50, 200)); // Dark grey
            rect.setOutlineColor(sf::Color(100, 100, 100));
            rect.setOutlineThickness(1.0f);
        }

        if (b.is_mission_active) {
            rect.setOutlineColor(sf::Color(0, 255, 0)); // Bright Green for active mission
            rect.setOutlineThickness(8.0f);
        }
        
        window.draw(rect);
    }
}

void Renderer::updateShader(float totalTime) {
    // Send the current time to the "u_time" variable in the GPU
    shader.setUniform("u_time", totalTime);
}

// This method updates the visual shape with the data from the DroneChassis and draws it
void Renderer::updateBody(const DroneChassis& DroneChassis) {
    shape.setRadius(4.0f);
    shape.setOrigin({4.0f, 4.0f}); // For the position to be the center
    
    shape.setFillColor(sf::Color(255, 255, 255));
    
    // Invert Y for the individual shape
    shape.setPosition({DroneChassis.position.x, WORLD_SIZE.y - DroneChassis.position.y});
    window.draw(shape);
}

void Renderer::drawWorld(const world& virtualWorld) {
    virtualWorld.draw(window);
}

void Renderer::drawSwarm(const std::vector<DroneChassis>& drones) {
    // SFML 3.0 uses Triangles (2 triangles per drone square = 6 vertices)
    sf::VertexArray swarm(sf::PrimitiveType::Triangles, drones.size() * 6);
    
    for(size_t i = 0; i < drones.size(); ++i) {
        const auto& b = drones[i];
        float s = DRONE_SIZE; 
        float x = b.position.x;
        float y = WORLD_SIZE.y - b.position.y;  
        
        sf::Color color(255, 255, 255);
        
        size_t idx = i * 6;
        
        // Define the two triangles that form the drone square
        // Triangle 1
        swarm[idx + 0].position = sf::Vector2f(x - s, y - s);
        swarm[idx + 1].position = sf::Vector2f(x + s, y - s);
        swarm[idx + 2].position = sf::Vector2f(x - s, y + s);
        
        // Triangle 2
        swarm[idx + 3].position = sf::Vector2f(x + s, y - s);
        swarm[idx + 4].position = sf::Vector2f(x + s, y + s);
        swarm[idx + 5].position = sf::Vector2f(x - s, y + s);
        
        // Color by battery for each drone        
        for(int j = 0; j < 6; ++j) {
            float bp = b.battery / b.max_battery;
            if (bp < 0.0f) bp = 0.0f;
            if (bp > 1.0f) bp = 1.0f;
            
            uint8_t cr, cg, cb;
            if (bp > 0.5f) {
                // Upper half: navy blue -> orange
                float t = (bp - 0.5f) * 2.0f; // 1.0 at full, 0.0 at half
                cr = static_cast<uint8_t>(220.0f - t * 200.0f); // 220 -> 20
                cg = static_cast<uint8_t>(120.0f - t * 90.0f);  // 120 -> 30
                cb = static_cast<uint8_t>(20.0f  + t * 60.0f);  //  20 -> 80
            } else {
                // Lower half: orange -> dark red
                float t = bp * 2.0f; // 1.0 at half, 0.0 at empty
                cr = static_cast<uint8_t>(180.0f + t * 40.0f);  // 180 -> 220
                cg = static_cast<uint8_t>(20.0f  + t * 100.0f); //  20 -> 120
                cb = static_cast<uint8_t>(20.0f);                //  20 -> 20
            }
            
            swarm[idx + j].color = sf::Color(cr, cg, cb);
        }
    }
    
    window.draw(swarm);
}

void Renderer::run(const DroneChassis& DroneChassis) {
    while (isOpen()) {
        handleEvents();
        clear(0.0f); // We pass 0 because this method is no longer used for the real simulation
        updateBody(DroneChassis);
        display();
    }
}
