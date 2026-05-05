#ifndef SWARM_DYNAMICS_H
#define SWARM_DYNAMICS_H

#include "../utils/vector2.h"
#include "../AI/states.h"

// 1. Estructuras compartidas entre CPU y GPU
struct PIDOutput {
    float p, i, d;
};

struct ControlOutput {
    float thrust;
    float angle;
};

struct DroneChassis {
    // Física
    Vector2 position = {0.0f, 0.0f};
    Vector2 velocity = {0.0f, 0.0f};
    Vector2 force    = {0.0f, 0.0f};
    Vector2 acceleration = {0.0f, 0.0f};
    
    // Desglose de fuerzas para telemetría
    Vector2 f_thrust     = {0.0f, 0.0f};
    Vector2 f_separation = {0.0f, 0.0f};
    Vector2 f_drag       = {0.0f, 0.0f};
    
    float mass = 1.0f;
    bool grounded = false;

    // Control
    Vector2 target = {0.0f, 0.0f};
    Vector2 error = {0.0f, 0.0f};
    Vector2 prev_error = {0.0f, 0.0f};
    ControlOutput prev_integral;
    ControlOutput control_output;
    Vector2 computed_sep_force = {0.0f, 0.0f};

    // AI
    int id;
    DroneAction current_action = DroneAction::FLYING_TO_TARGET;
    DroneState current_state = DroneState::LANDED;
    Vector2 original_target = {0.0f, 0.0f};
    float battery = 100.0f;
    float max_battery = 100.0f;
};

// 2. Interfaz de la función de Host (el puente)
#ifdef __cplusplus
extern "C" {
#endif

void launch_physics_kernel(DroneChassis* drones, int n, float dt);

#ifdef __cplusplus
}
#endif

#endif

