#ifndef BASIC_RENDERER_H
#define BASIC_RENDERER_H

#include <SFML/Graphics.hpp>
#include <optional>
#include "body.h"
#include "utils/vector2.h"

class BasicRenderer {
public:
    // Sugerencia: pasar el tamaño en el constructor es más limpio
    BasicRenderer(Vector2 windowSize = {800, 600});
    
    bool isOpen() const;
    void handleEvents();
    void clear();
    void display();
    
    // Método para actualizar y dibujar un cuerpo específico
    void updateBody(const Body& body);
    
    // Bucle para compatibilidad
    void run(const Body& body);

private:
    sf::RenderWindow window;
    sf::CircleShape shape;
    Vector2 windowSize;
};

#endif // BASIC_RENDERER_H
