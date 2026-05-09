#ifndef SWARM_DYNAMICS_H
#define SWARM_DYNAMICS_H

#include "utils/vector2.h"
#include "AI/drone_states.h"

// 1. Shared structures between CPU and GPU
struct PIDOutput {
    float p, i, d;
};

struct ControlOutput {
    float thrust;
    float angle;
};

struct DroneChassis {
    // Physics
    Vector2 position = {0.0f, 0.0f};
    Vector2 velocity = {0.0f, 0.0f};
    Vector2 force    = {0.0f, 0.0f};
    Vector2 acceleration = {0.0f, 0.0f};
    
    // Breakdown of forces for telemetry
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

    // AI & Grouping
    int id;
    int group_id = -1; // -1 means no group
    int group_row = 0;
    int group_col = 0;

    int current_action = 0; // Maps to DroneAction
    int current_state = 0;  // Maps to DroneState
    Vector2 original_target = {0.0f, 0.0f};
    float battery = 100.0f;
    float max_battery = 100.0f;
};

// Structure for obstacles in GPU
struct GPUObstacle {
    float x, y, w, h;
};

// 2. Interface for Host function (the bridge)
#ifdef __cplusplus
extern "C" {
#endif

void launch_physics_kernel(DroneChassis* drones, int n, float dt);

#ifdef __cplusplus
}
#endif

#endif

