#include "rendering/BasicRenderer.h"

// El constructor inicializa la ventana. No se puede llamar dentro de otra función como run().
BasicRenderer::BasicRenderer(Vector2 windowSize)
    : window(sf::VideoMode({(unsigned int)windowSize.x, (unsigned int)windowSize.y}), "Flight Simulation"),
      windowSize(windowSize)
{
    window.setFramerateLimit(60); 
    
    // 1. Cargar el programa de la GPU (Shader)
    if (!shader.loadFromFile("assets/shaders/entorno.frag", sf::Shader::Type::Fragment)) {
        // Si falla, SFML imprimirá un error en la consola
    }

    // 2. Configurar el rectángulo de fondo que ocupará toda la ventana
    backgroundRect.setSize({(float)windowSize.x, (float)windowSize.y});
    
    // 3. Pasar la resolución inicial a la GPU
    shader.setUniform("u_resolution", sf::Vector2f(windowSize.x, windowSize.y));

    shape.setRadius(10.f);
    shape.setFillColor(sf::Color::White);
}

bool BasicRenderer::isOpen() const {
    return window.isOpen();
}

void BasicRenderer::handleEvents() {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            window.close();
    }
}

void BasicRenderer::clear(float totalTime) {
    window.clear(sf::Color::Black);
    
    // Actualizamos y dibujamos el fondo con el Shader
    updateShader(totalTime);
    window.draw(backgroundRect, &shader);
}

void BasicRenderer::display() {
    window.display();
}

void BasicRenderer::updateShader(float totalTime) {
    // Enviamos el tiempo actual a la variable "u_time" en la GPU
    shader.setUniform("u_time", totalTime);
}

// Este método actualiza la forma visual con los datos del Body y la dibuja
void BasicRenderer::updateBody(const Body& body) {
    shape.setRadius(body.size);
    shape.setOrigin({body.size, body.size}); // Para que la posición sea el centro
    
    shape.setFillColor(sf::Color(body.color[0], body.color[1], body.color[2]));
    
    Vector2 worldPosition = {body.position.x, windowSize.y - body.position.y};
    shape.setPosition({worldPosition.x, worldPosition.y});
    window.draw(shape);
}

void BasicRenderer::drawSwarm(const std::vector<Body>& bodies) {
    // SFML 3.0 usa Triángulos (2 triángulos por cada cuadrado del dron = 6 vértices)
    sf::VertexArray swarm(sf::PrimitiveType::Triangles, bodies.size() * 6);
    
    for(size_t i = 0; i < bodies.size(); ++i) {
        const auto& b = bodies[i];
        float s = b.size; 
        float x = b.position.x;
        float y = windowSize.y - b.position.y;  
        
        sf::Color color(b.color[0], b.color[1], b.color[2]);
        
        size_t idx = i * 6;
        
        // Definimos los dos triángulos que forman el cuadrado del dron
        // Triángulo 1
        swarm[idx + 0].position = sf::Vector2f(x - s, y - s);
        swarm[idx + 1].position = sf::Vector2f(x + s, y - s);
        swarm[idx + 2].position = sf::Vector2f(x - s, y + s);
        
        // Triángulo 2
        swarm[idx + 3].position = sf::Vector2f(x + s, y - s);
        swarm[idx + 4].position = sf::Vector2f(x + s, y + s);
        swarm[idx + 5].position = sf::Vector2f(x - s, y + s);
        
        // Color por batería para cada dron        
        for(int j = 0; j < 6; ++j) {
            // Color de batería: Verde (100%) a Rojo (0%)
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

void BasicRenderer::run(const Body& body) {
    while (isOpen()) {
        handleEvents();
        clear(0.0f); // Le pasamos 0 porque este método ya no lo usamos para la simulación real
        updateBody(body);
        display();
    }
}