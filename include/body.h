#ifndef BODY_H
#define BODY_H

#include "utils/vector2.h"
#include "vehicle/controller.h"
#include "vehicle/control_config.h"

struct Body {
    // Physics
    Vector2 position = {0.0f, 0.0f};
    Vector2 velocity = {0.0f, 0.0f};
    Vector2 force    = {0.0f, 0.0f};
    Vector2 acceleration = {0.0f, 0.0f};

    float mass = 1.0f;
    bool grounded = false;

    // Render
    float size = 1.0f;
    int color[3] = {255, 255, 255};

    // Control
    int id = 0;
    float error_x = 0.0f;
    float error_y = 0.0f;
    
    PIDOutput angle_pid;
    PIDOutput thrust_pid;
    float angle;
    float thrust;

    ActuatorOutput actuator_output;

    Vector2 target = {0.0f, 0.0f};
    Controller controller;
};

#endif