#ifndef BASIC_RENDERER_H
#define BASIC_RENDERER_H

#include <SFML/Graphics.hpp>
#include "HPC/swarm_dynamics.h"
#include "utils/vector2.h"
#include "rendering/camera.h"

// Forward declaration
class world;
class ProceduralCity;

class BasicRenderer {
public:
    // Sugerencia: pasar el tamaÃƒÂ±o en el constructor es mÃƒÂ¡s limpio
    BasicRenderer(Vector2 windowSize = {800, 600});
    
    bool isOpen() const;
    void handleEvents();
    void clear(float totalTime);
    void display();
    
    void drawCity(const ProceduralCity& city);
    
    // MÃƒÂ©todo para actualizar parÃƒÂ¡metros del Shader
    void updateShader(float totalTime);
    
    // MÃƒÂ©todo para actualizar y dibujar un cuerpo especÃƒÂ­fico
    void updateBody(const DroneChassis& DroneChassis);
    
    // Bucle para compatibilidad
    void run(const DroneChassis& DroneChassis);

    void drawSwarm(const std::vector<DroneChassis>& drones);
    void drawWorld(const world& virtualWorld);

    // CÃƒÂ¡mara
    // CÃƒÂ¡mara inteligente modular
    void updateCamera(const std::vector<DroneChassis>& drones);
    
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
