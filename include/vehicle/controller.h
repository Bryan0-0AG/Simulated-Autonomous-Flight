#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "vehicle/pid.h"
#include "vehicle/control_config.h"
#include "utils/vector2.h"

class Controller {
public:
    Controller();

    ActuatorOutput update(Vector2 target, Vector2 body_position, float dt);

private:
    PID pid_x; 
    PID pid_y;
};

#endif