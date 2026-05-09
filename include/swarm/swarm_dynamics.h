#ifndef SWARM_DYNAMICS_H
#define SWARM_DYNAMICS_H

#include "../utils/vector2.h"
#include "../AI/states.h"
#include <cstdint>

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

    // Visual (used by renderer for battery color)
    struct { uint8_t r, g, b, a; } color = {0, 255, 0, 255};
};

// Structure for GPU obstacle representation
struct GPUObstacle {
    float x, y, w, h;
};

// Flat GPU-friendly representation of a MatrixGroup for the AI kernel.
// No std::vector - all fields are plain data types for GPU compatibility.
struct GPUMatrix {
    // Identity & geometry
    int   id;
    float center_x,     center_y;
    float nav_target_x, nav_target_y;
    float final_target_x, final_target_y;
    float col_spacing,  row_spacing;
    int   cols,         rows;
    int   child_count;  // Number of drones currently assigned

    // Navigation state
    int   current_action;
    int   assigned_lane;
    float lane_change_cooldown;
    int   is_braking;   // bool stored as int for GPU compatibility

    // Output fields (written by the kernel, applied back by CPU)
    float out_center_x,     out_center_y;
    float out_nav_target_x, out_nav_target_y;
    int   out_assigned_lane;
    float out_lane_change_cooldown;
    int   out_is_braking;
    float out_threat_distance; // Distance to closest forward threat (-1 if clear)
    float out_repulsion_x;     // Cross-lane repulsion force computed by GPU
};

// 2. Interface for Host functions (the bridge)
#ifdef __cplusplus
extern "C" {
#endif

void launch_physics_kernel(DroneChassis* drones, int n, float dt, const GPUObstacle* obstacles, int num_obstacles);
void launch_matrix_ai_kernel(GPUMatrix* matrices, int n, float dt);

#ifdef __cplusplus
}
#endif

#endif
