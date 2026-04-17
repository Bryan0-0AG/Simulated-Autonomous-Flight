#include <iostream>
#include <algorithm>
#include "vehicle/controller.h"
#include "utils/vector2.h"
#include "vehicle/control_config.h"

Controller::Controller()
    : pid_x(PID_X_KP, PID_X_KI, PID_X_KD),
      pid_y(PID_Y_KP, PID_Y_KI, PID_Y_KD) {}

ActuatorOutput Controller::update(Vector2 target, Vector2 body_position, float dt) {    
    float error_x = target.x - body_position.x;
    float error_y = target.y - body_position.y;

    ActuatorOutput output;
    output.angle = pid_x.update(error_x, dt);
    output.thrust = pid_y.update(error_y, dt);

    output.thrust = std::clamp(output.thrust, MIN_THRUST, MAX_THRUST);
    output.angle  = std::clamp(output.angle, MIN_ANGLE, MAX_ANGLE);

    return output;
}