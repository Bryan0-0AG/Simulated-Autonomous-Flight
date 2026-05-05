#ifndef GLOBAL_CONFIG_H
#define GLOBAL_CONFIG_H

#include "utils/vector2.h"
#include <string>

// ==============================
// WORLD & SIMULATION
// ==============================
constexpr Vector2 WINDOW_SIZE = {1280.0f, 720.0f};      
constexpr Vector2 WORLD_SIZE  = {100000.0f, 100000.0f}; 
constexpr float DT = 0.016f;

// ==============================
// DRONE SPAWN & SWARM
// ==============================
constexpr int DRONE_COUNT = 5000;
constexpr int SPAWN_INTERVAL = 1;
constexpr float SEPARATION_RADIUS = 25.0f;
constexpr float SEPARATION_FORCE = 5.0f; 

// Formation configuration
constexpr int GRID_ROWS_OFFSET = 8;
constexpr int GRID_COLS = 250;

// ==============================
// PHYSICS (AMD GPU Compatible)
// ==============================
constexpr float GRAVITY = 9.81f;
constexpr float AIR_FRICTION = 0.1f;
constexpr float GROUND_FRICTION = 0.5f;
constexpr float COLLISION_EPSILON = 0.001f;
constexpr float ENERGY_RESTITUTION = 0.7f;
constexpr float MAX_VELOCITY = 100.0f;

// ==============================
// PID CONTROL PARAMETERS (Names synchronized with HIP kernel)
// ==============================

// Control en X (Ángulo)
constexpr float PID_KP_ANGLE = 0.4f;
constexpr float PID_KI_ANGLE = 0.0f;
constexpr float PID_KD_ANGLE = 2.1f;
constexpr float PID_MAX_I_ANGLE = 10.0f;

// Control en Y (Empuje/Thrust)
constexpr float PID_KP_THRUST = 0.4f;
constexpr float PID_KI_THRUST = 0.005f;
constexpr float PID_KD_THRUST = 1.3f;
constexpr float PID_MAX_I_THRUST = 20.0f;

// ==============================
// ACTUATORS & LIMITS
// ==============================
constexpr float MAX_THRUST = 500.0f;
constexpr float MIN_THRUST = 0.0f;
constexpr float MAX_ANGLE  = 0.5f;
constexpr float MIN_ANGLE  = -0.5f;

// ==============================
// AI & BATTERY
// ==============================
constexpr float BATTERY_DRAIN = 0.05f;
constexpr float BATTERY_CHARGE_SPEED = 2.0f;

// ==============================
// NETWORK
// ==============================
const std::string SERVER_IP = "127.0.0.1";
const unsigned short SERVER_PORT = 9999;

// ==============================
// AI
// ==============================
constexpr int LLM_QUERY_INTERVAL = 120;

#endif // GLOBAL_CONFIG_H