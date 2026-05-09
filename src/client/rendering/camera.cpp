#include "rendering/camera.h"
#include "global_config.h"

#include <algorithm>

camera::camera(Vector2 windowSize) : windowSize(windowSize) {
    view.setSize({(float)windowSize.x, (float)windowSize.y});
    
    // Inicializamos el offset en el centro inicial deseado (X: Mitad de pantalla, Y: Sobre el suelo)
    manualOffset.x = windowSize.x / 2.0f;
    manualOffset.y = windowSize.y / 2.0f;
    
    currentHeight = (float)windowSize.y;
    view.setCenter({manualOffset.x, WORLD_SIZE.y - manualOffset.y});
}

void camera::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    // 1. Zoom with Scroll
    if (const auto* scroll = event.getIf<sf::Event::MouseWheelScrolled>()) {
        if (scroll->delta > 0) zoomMultiplier *= 0.9f; // Zoom In (zooming in)
        else zoomMultiplier *= 1.1f;                  // Zoom Out (zooming out)
    }

    // 2. Start of the Drag with Right Click
    if (const auto* mouseButton = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseButton->button == sf::Mouse::Button::Right) {
            isDragging = true;
            lastMousePos = mouseButton->position;
        }
    }

    // 3. End of the Drag
    if (const auto* mouseButton = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseButton->button == sf::Mouse::Button::Right) {
            isDragging = false;
        }
    }

    // 4. Movement during the Drag (Panning)
    if (const auto* mouseMove = event.getIf<sf::Event::MouseMoved>()) {
        if (isDragging) {
            sf::Vector2i currentMousePos = mouseMove->position;
            
            // Convert the difference of pixels to world coordinates
            sf::Vector2f worldLast = window.mapPixelToCoords(lastMousePos, view);
            sf::Vector2f worldCurrent = window.mapPixelToCoords(currentMousePos, view);
            sf::Vector2f delta = worldLast - worldCurrent;

            // Adjust the manual offset (compensating our Y inversion)
            manualOffset.x += delta.x;
            manualOffset.y -= delta.y; 

            lastMousePos = currentMousePos;
        }
    }
}

void camera::update(const std::vector<DroneChassis>& drones) {
    if (drones.empty()) return;

    // 1. Calculate the Bounding Box (Limits) of the swarm
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

    // 3. Manual Zoom
    float baseHeight = windowSize.y;
    float targetHeight = baseHeight * zoomMultiplier;

    currentHeight += (targetHeight - currentHeight) * zoomLerpFactor;
    float currentWidth = currentHeight * (windowSize.x / windowSize.y);

    view.setSize({currentWidth, currentHeight});

    // 4. Final Position (Based only on manualOffset)
    // Note: manualOffset now acts as the camera center in the world
    sf::Vector2f targetCenter = {manualOffset.x, WORLD_SIZE.y - manualOffset.y};
    
    // 5. Smart Clamping (Camera limits)
    float halfWidth = view.getSize().x / 2.0f;
    float halfHeight = view.getSize().y / 2.0f;

    // X limit (Do not allow the view to leave [0, WORLD_SIZE.x])
    if (targetCenter.x < halfWidth) targetCenter.x = halfWidth;
    if (targetCenter.x > WORLD_SIZE.x - halfWidth) targetCenter.x = WORLD_SIZE.x - halfWidth;

    // Y limit (Ground and Sky)
    float groundBottom = WORLD_SIZE.y + 1000.0f; // Margin for the ground
    if (targetCenter.y > groundBottom - halfHeight) targetCenter.y = groundBottom - halfHeight;
    if (targetCenter.y < halfHeight) targetCenter.y = halfHeight;

    // Apply to the View
    view.setCenter(targetCenter);

    // 6. Synchronize the manual Offset with the real limited center
    // This prevents you from continuing to drag "in the void" beyond the limits
    manualOffset.x = targetCenter.x;
    manualOffset.y = WORLD_SIZE.y - targetCenter.y;
}
