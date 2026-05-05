#include "rendering/camera.h"
#include "global_config.h"

#include <algorithm>

camera::camera(Vector2 windowSize) : windowSize(windowSize) {
    view.setSize({(float)windowSize.x, (float)windowSize.y});
    currentCenter = {windowSize.x / 2.f, windowSize.y / 2.f};
    currentHeight = (float)windowSize.y;
    view.setCenter({currentCenter.x, WORLD_SIZE.y - currentCenter.y});
}

void camera::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    // 1. Zoom con Scroll
    if (const auto* scroll = event.getIf<sf::Event::MouseWheelScrolled>()) {
        if (scroll->delta > 0) zoomMultiplier *= 0.9f; // Zoom In (acercar)
        else zoomMultiplier *= 1.1f;                  // Zoom Out (alejar)
    }

    // 2. Inicio del Drag con Click Derecho
    if (const auto* mouseButton = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseButton->button == sf::Mouse::Button::Right) {
            isDragging = true;
            lastMousePos = mouseButton->position;
        }
    }

    // 3. Fin del Drag
    if (const auto* mouseButton = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseButton->button == sf::Mouse::Button::Right) {
            isDragging = false;
        }
    }

    // 4. Movimiento durante el Drag (Panning)
    if (const auto* mouseMove = event.getIf<sf::Event::MouseMoved>()) {
        if (isDragging) {
            sf::Vector2i currentMousePos = mouseMove->position;
            
            // Convertimos la diferencia de pÃƒÂ­xeles a coordenadas del mundo
            sf::Vector2f worldLast = window.mapPixelToCoords(lastMousePos, view);
            sf::Vector2f worldCurrent = window.mapPixelToCoords(currentMousePos, view);
            sf::Vector2f delta = worldLast - worldCurrent;

            // Ajustamos el offset manual (compensando nuestra inversiÃƒÂ³n de Y)
            manualOffset.x += delta.x;
            manualOffset.y -= delta.y; 

            lastMousePos = currentMousePos;
        }
    }
}

void camera::update(const std::vector<DroneChassis>& drones) {
    if (drones.empty()) return;

    // 1. Calcular el Bounding Box (LÃƒÂ­mites) del enjambre
    Vector2 swarm_center = {0, 0};
    float minX = drones[0].position.x, maxX = drones[0].position.x;
    float minY = drones[0].position.y, maxY = drones[0].position.y;

    for (const auto& b : drones) {
        swarm_center.x += b.position.x;
        swarm_center.y += b.position.y;
        if (b.position.x < minX) minX = b.position.x;
        if (b.position.x > maxX) maxX = b.position.x;
        if (b.position.y < minY) minY = b.position.y;
        if (b.position.y > maxY) maxY = b.position.y;
    }
    swarm_center.x /= (float)drones.size();
    swarm_center.y /= (float)drones.size();

    float swarmWidth = maxX - minX;
    float swarmHeight = maxY - minY;

    // 2. InterpolaciÃƒÂ³n del Centro Auto-tracking
    currentCenter.x += (swarm_center.x - currentCenter.x) * lerpFactor;
    currentCenter.y += (swarm_center.y - currentCenter.y) * lerpFactor;

    // 3. CÃƒÂ¡lculo de Zoom DinÃƒÂ¡mico + Multiplicador Manual
    float reqH_V = swarmHeight * margin;
    float reqH_H = (swarmWidth * margin) / (windowSize.x / windowSize.y);
    float reqH_G = currentCenter.y * 2.05f; 

    float targetHeight = std::max({(float)windowSize.y, reqH_V, reqH_H, reqH_G});
    
    // Aplicamos el zoom manual al target
    targetHeight *= zoomMultiplier;

    currentHeight += (targetHeight - currentHeight) * zoomLerpFactor;
    float currentWidth = currentHeight * (windowSize.x / windowSize.y);

    view.setSize({currentWidth, currentHeight});

    // 4. PosiciÃƒÂ³n Final (Auto-center + Manual Offset)
    sf::Vector2f targetCenter = {currentCenter.x + manualOffset.x, WORLD_SIZE.y - (currentCenter.y + manualOffset.y)};
    
    // 5. CLAMPING INTELIGENTE (LÃƒÂ­mites de cÃƒÂ¡mara)
    float halfWidth = view.getSize().x / 2.0f;
    float halfHeight = view.getSize().y / 2.0f;

    // LÃƒÂ­mite lateral X (0 a WORLD_SIZE.x)
    if (targetCenter.x - halfWidth < 0.0f) targetCenter.x = halfWidth;
    if (targetCenter.x + halfWidth > WORLD_SIZE.x) targetCenter.x = WORLD_SIZE.x - halfWidth;

    // LÃƒÂ­mite inferior Y (Permitir ver 1000px de suelo negro)
    float groundBottom = WORLD_SIZE.y + 1000.0f;
    if (targetCenter.y + halfHeight > groundBottom) {
        targetCenter.y = groundBottom - halfHeight;
    }

    // LÃƒÂ­mite superior Y (Opcional: No subir al infinito si quieres)
    if (targetCenter.y - halfHeight < 0.0f) targetCenter.y = halfHeight;

    // Aplicar al View
    view.setCenter(targetCenter);

    // 6. SINCRONIZAR OFFSETS (Evita el bloqueo de la cÃƒÂ¡mara)
    // Si la cÃƒÂ¡mara fue limitada, ajustamos el offset manual para que no se "acumule" error
    manualOffset.x = targetCenter.x - currentCenter.x;
    manualOffset.y = WORLD_SIZE.y - targetCenter.y - currentCenter.y;
}
