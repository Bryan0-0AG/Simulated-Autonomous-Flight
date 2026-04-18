#include "rendering/BasicRenderer.h"

// El constructor inicializa la ventana. No se puede llamar dentro de otra función como run().
BasicRenderer::BasicRenderer(Vector2 windowSize)
    : window(sf::VideoMode({(unsigned int)windowSize.x, (unsigned int)windowSize.y}), "Flight Simulation"),
      windowSize(windowSize)   // ← faltaba esto
{
    window.setFramerateLimit(60); // <-- ESTO ES CLAVE PARA QUE VAYA EN TIEMPO REAL
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

void BasicRenderer::clear() {
    window.clear(sf::Color::Black);
}

void BasicRenderer::display() {
    window.display();
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

void BasicRenderer::run(const Body& body) {
    while (isOpen()) {
        handleEvents();
        clear();
        updateBody(body);
        display();
    }
}