#ifndef BODY_H
#define BODY_H

#include "utils/vector2.h"
#include "control/controller.h"
#include "control/control_config.h"
#include "AI/states.h"

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

    // AI
    DroneAction current_action = DroneAction::FLYING_TO_TARGET;
    DroneState current_state = DroneState::LANDED;
    Vector2 original_target = {0.0f, 0.0f}; // Para no olvidar su misión mientras carga
    float battery = 100.0f;
    float max_battery = 100.0f;
};

#endif