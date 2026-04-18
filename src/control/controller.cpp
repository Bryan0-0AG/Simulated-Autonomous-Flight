#include <iostream>
#include <algorithm>
#include "control/controller.h"
#include "control/control_config.h"
#include "physics/physics_config.h"
#include "body.h"

Controller::Controller()
    : pid_x(PID_X_KP, PID_X_KI, PID_X_KD, PID_X_MAX_INTEGRAL),
      pid_y(PID_Y_KP, PID_Y_KI, PID_Y_KD, PID_Y_MAX_INTEGRAL) {}

ActuatorOutput Controller::update(Body& body, float dt) {  
    body.error_x = body.target.x - body.position.x;
    body.error_y = body.target.y - body.position.y;  

    body.angle_pid = pid_x.update(body.error_x, dt);
    body.thrust_pid = pid_y.update(body.error_y, dt);
    
    body.actuator_output.angle = 
        body.angle_pid.p + 
        body.angle_pid.i + 
        body.angle_pid.d;
        
    body.actuator_output.thrust = 
        body.thrust_pid.p + 
        body.thrust_pid.i + 
        body.thrust_pid.d + 
        GRAVITY;

    body.actuator_output.thrust = std::clamp(body.actuator_output.thrust, MIN_THRUST, MAX_THRUST);
    body.actuator_output.angle  = std::clamp(body.actuator_output.angle, MIN_ANGLE, MAX_ANGLE);

    ActuatorOutput output = {
        body.actuator_output.thrust,
        body.actuator_output.angle        
    };

    return output;
}