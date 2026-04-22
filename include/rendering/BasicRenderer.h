#ifndef BASIC_RENDERER_H
#define BASIC_RENDERER_H

#include <SFML/Graphics.hpp>
#include "body.h"
#include "utils/vector2.h"
#include "rendering/camera.h"

// Forward declaration
class world;

class BasicRenderer {
public:
    // Sugerencia: pasar el tamaño en el constructor es más limpio
    BasicRenderer(Vector2 windowSize = {800, 600});
    
    bool isOpen() const;
    void handleEvents();
    void clear(float totalTime);
    void display();
    
    // Método para actualizar parámetros del Shader
    void updateShader(float totalTime);
    
    // Método para actualizar y dibujar un cuerpo específico
    void updateBody(const Body& body);
    
    // Bucle para compatibilidad
    void run(const Body& body);

    void drawSwarm(const std::vector<Body>& bodies);
    void drawWorld(const world& virtualWorld);

    // Cámara
    // Cámara inteligente modular
    void updateCamera(const std::vector<Body>& bodies);
    
private:
    sf::RenderWindow window;
    camera camera;
    sf::CircleShape shape;
    Vector2 windowSize;

    // Elementos para el Shader de la GPU
    sf::Shader shader;
    sf::RectangleShape backgroundRect;
};

#endif // BASIC_RENDERER_H
