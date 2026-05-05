#include "rendering/BasicRenderer.h"
#include "world/world.h"
#include "world/procedural_city.h"

#include "global_config.h"
#include <algorithm>

// El constructor inicializa la ventana. No se puede llamar dentro de otra funciÃƒÆ’Ã‚Â³n como run().
BasicRenderer::BasicRenderer(Vector2 windowSize)
    : window(sf::VideoMode({(unsigned int)windowSize.x, (unsigned int)windowSize.y}), "Flight Simulation"),
      camera(windowSize),
      windowSize(windowSize)
{
    window.setFramerateLimit(60); 
    
    // 1. Cargar el programa de la GPU (Shader)
    if (!shader.loadFromFile("assets/shaders/entorno.frag", sf::Shader::Type::Fragment)) {
        // Si falla, SFML imprimirÃƒÆ’Ã‚Â¡ un error en la consola
    }

    // 2. Configurar el rectÃƒÆ’Ã‚Â¡ngulo de fondo que ocuparÃƒÆ’Ã‚Â¡ toda la ventana
    backgroundRect.setSize({(float)windowSize.x, (float)windowSize.y});
    
    // 3. Pasar la resoluciÃƒÆ’Ã‚Â³n inicial a la GPU
    shader.setUniform("u_resolution", sf::Vector2f(windowSize.x, windowSize.y));

    shape.setRadius(10.f);
    shape.setFillColor(sf::Color::White);
}

bool BasicRenderer::isOpen() const {
    return window.isOpen();
}

void BasicRenderer::handleEvents() {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }
        
        // Pasar el evento a la cÃƒÆ’Ã‚Â¡mara para el control manual
        camera.handleEvent(*event, window);
    }
}

void BasicRenderer::clear(float totalTime) {
    window.clear(sf::Color::Black);
    
    // Fondo estÃƒÆ’Ã‚Â¡tico (UI/Shader)
    window.setView(window.getDefaultView());
    window.draw(backgroundRect, &shader);

    // Activar cÃƒÆ’Ã‚Â¡mara para los drones y el mundo (desde el objeto cÃƒÆ’Ã‚Â¡mara modular)
    window.setView(camera.getView());
}

void BasicRenderer::updateCamera(const std::vector<DroneChassis>& drones) {
    camera.update(drones);
}

void BasicRenderer::display() {
    window.display();
}

void BasicRenderer::drawCity(const ProceduralCity& city) {
    const auto& buildings = city.getBuildings();
    sf::RectangleShape rect;
    
    for (const auto& b : buildings) {
        rect.setSize({b.bounds.size.x, b.bounds.size.y});
        // IMPORTANTE: Invertimos Y usando la altura real del mundo, no de la ventana
        rect.setPosition({b.bounds.position.x, city.getHeight() - b.bounds.position.y - b.bounds.size.y});
        
        // Colores NeÃƒÆ’Ã‚Â³n segÃƒÆ’Ã‚Âºn el tipo
        if (b.type == BuildingType::CHARGER) {
            rect.setFillColor(sf::Color(0, 255, 255, 180)); // Cyan neÃƒÆ’Ã‚Â³n
            rect.setOutlineColor(sf::Color::White);
            rect.setOutlineThickness(2.0f);
        } else if (b.type == BuildingType::COLLECTION) {
            rect.setFillColor(sf::Color(50, 255, 50, 180)); // Verde neÃƒÆ’Ã‚Â³n
            rect.setOutlineColor(sf::Color::White);
            rect.setOutlineThickness(2.0f);
        } else {
            rect.setFillColor(sf::Color(50, 50, 50, 200)); // Gris oscuro
            rect.setOutlineColor(sf::Color(100, 100, 100));
            rect.setOutlineThickness(1.0f);
        }
        
        window.draw(rect);
    }
}

void BasicRenderer::updateShader(float totalTime) {
    // Enviamos el tiempo actual a la variable "u_time" en la GPU
    shader.setUniform("u_time", totalTime);
}

// Este mÃƒÆ’Ã‚Â©todo actualiza la forma visual con los datos del DroneChassis y la dibuja
void BasicRenderer::updateBody(const DroneChassis& DroneChassis) {
    shape.setRadius(4.0f);
    shape.setOrigin({4.0f, 4.0f}); // Para que la posiciÃƒÆ’Ã‚Â³n sea el centro
    
    shape.setFillColor(sf::Color(255, 255, 255));
    
    // Invertimos Y para la forma individual
    shape.setPosition({DroneChassis.position.x, WORLD_SIZE.y - DroneChassis.position.y});
    window.draw(shape);
}

void BasicRenderer::drawWorld(const world& virtualWorld) {
    virtualWorld.draw(window);
}

void BasicRenderer::drawSwarm(const std::vector<DroneChassis>& drones) {
    // SFML 3.0 usa TriÃƒÆ’Ã‚Â¡ngulos (2 triÃƒÆ’Ã‚Â¡ngulos por cada cuadrado del dron = 6 vÃƒÆ’Ã‚Â©rtices)
    sf::VertexArray swarm(sf::PrimitiveType::Triangles, drones.size() * 6);
    
    for(size_t i = 0; i < drones.size(); ++i) {
        const auto& b = drones[i];
        float s = 4.0f; 
        float x = b.position.x;
        float y = WORLD_SIZE.y - b.position.y;  
        
        sf::Color color(255, 255, 255);
        
        size_t idx = i * 6;
        
        // Definimos los dos triÃƒÆ’Ã‚Â¡ngulos que forman el cuadrado del dron
        // TriÃƒÆ’Ã‚Â¡ngulo 1
        swarm[idx + 0].position = sf::Vector2f(x - s, y - s);
        swarm[idx + 1].position = sf::Vector2f(x + s, y - s);
        swarm[idx + 2].position = sf::Vector2f(x - s, y + s);
        
        // TriÃƒÆ’Ã‚Â¡ngulo 2
        swarm[idx + 3].position = sf::Vector2f(x + s, y - s);
        swarm[idx + 4].position = sf::Vector2f(x + s, y + s);
        swarm[idx + 5].position = sf::Vector2f(x - s, y + s);
        
        // Color por baterÃƒÆ’Ã‚Â­a para cada dron        
        for(int j = 0; j < 6; ++j) {
            // Color de baterÃƒÆ’Ã‚Â­a: Verde (100%) a Rojo (0%)
            float battery_pct = b.battery / b.max_battery;
            if (battery_pct < 0.0f) battery_pct = 0.0f;
            if (battery_pct > 1.0f) battery_pct = 1.0f;
            
            swarm[idx + j].color.r = static_cast<uint8_t>(255.0f * (1.0f - battery_pct)); // R
            swarm[idx + j].color.g = static_cast<uint8_t>(255.0f * battery_pct);          // G
            swarm[idx + j].color.b = 0;                                                   // B
        }
    }
    
    window.draw(swarm);
}

void BasicRenderer::run(const DroneChassis& DroneChassis) {
    while (isOpen()) {
        handleEvents();
        clear(0.0f); // Le pasamos 0 porque este mÃƒÆ’Ã‚Â©todo ya no lo usamos para la simulaciÃƒÆ’Ã‚Â³n real
        updateBody(DroneChassis);
        display();
    }
}