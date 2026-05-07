#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#include <SFML/Graphics.hpp>
#include <vector>
#include "swarm/swarm_dynamics.h"
#include "utils/vector2.h"

class camera {
public:
    camera(Vector2 windowSize);

    // Processes mouse events (scroll and drag)
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);

    // Updates camera (mixes auto-tracking with manual control)
    void update(const std::vector<DroneChassis>& drones);

    const sf::View& getView() const { return view; }

private:
    sf::View view;
    Vector2 windowSize;
    
    // Auto-tracking
    Vector2 currentCenter;
    float currentHeight;

    // Manual Control
    Vector2 manualOffset = {0, 0};
    float zoomMultiplier = 1.0f;
    
    // Mouse State for Drag
    bool isDragging = false;
    sf::Vector2i lastMousePos;

    // Configuration
    float lerpFactor = 0.05f;
    float zoomLerpFactor = 0.02f;
    float margin = 1.15f;
};

#endif // CAMERA_CLASS_H
