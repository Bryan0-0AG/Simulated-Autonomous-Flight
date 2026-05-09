#ifndef RENDERER_H
#define RENDERER_H

#include <SFML/Graphics.hpp>
#include "drone_dynamics.h"
#include "utils/vector2.h"
#include "rendering/camera.h"

// Forward declaration
class world;
class ProceduralCity;

class Renderer {
public:
    Renderer(Vector2 windowSize = {800, 600});
    
    bool isOpen() const;
    void handleEvents();
    void clear(float totalTime);
    void display();
    
    void drawCity(const ProceduralCity& city);
    void updateShader(float totalTime);
    void updateBody(const DroneChassis& DroneChassis);
    void run(const DroneChassis& DroneChassis);

    void drawSwarm(const std::vector<DroneChassis>& drones);
    void drawWorld(const world& virtualWorld);

    void updateCamera(const std::vector<DroneChassis>& drones);
    
private:
    sf::RenderWindow window;
    camera cam;
    sf::CircleShape shape;
    Vector2 windowSize;

    sf::Shader shader;
    sf::RectangleShape backgroundRect;
};

#endif // RENDERER_H
