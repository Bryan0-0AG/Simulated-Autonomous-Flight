#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#include <SFML/Graphics.hpp>
#include <vector>
#include "body.h"
#include "utils/vector2.h"

class camera {
public:
    camera(Vector2 windowSize);

    // Procesa eventos de mouse (scroll y drag)
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);

    // Actualiza la cámara (mezcla auto-tracking con control manual)
    void update(const std::vector<Body>& bodies);

    const sf::View& getView() const { return view; }

private:
    sf::View view;
    Vector2 windowSize;
    
    // Auto-tracking
    Vector2 currentCenter;
    float currentHeight;

    // Control Manual
    Vector2 manualOffset = {0, 0};
    float zoomMultiplier = 1.0f;
    
    // Estado del Mouse para el Drag
    bool isDragging = false;
    sf::Vector2i lastMousePos;

    // Configuración
    float lerpFactor = 0.05f;
    float zoomLerpFactor = 0.02f;
    float margin = 1.15f;
};

#endif // CAMERA_CLASS_H
