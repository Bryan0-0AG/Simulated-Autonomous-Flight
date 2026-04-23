#ifndef GLOBAL_CONFIG_H
#define GLOBAL_CONFIG_H

#include "utils/vector2.h"
#include <string>

// ==============================
// BASIC STRUCTURES
// ==============================
struct PIDOutput {
    float p;
    float i;
    float d;
};

struct ActuatorOutput {
    float thrust;
    float angle;
};

// ==============================
// WORLD & SIMULATION
// ==============================
const Vector2 WINDOW_SIZE = {1280.0f, 720.0f};      // Tamaño físico
const Vector2 WORLD_SIZE  = {100000.0f, 100000.0f}; // Espacio virtual
const float DT = 0.016f;

// ==============================
// DRONE SPAWN
// ==============================
const int DRONE_COUNT = 5000;
const int SPAWN_INTERVAL = 1;

// Flocking (collision avoidance)
const float SEPARATION_RADIUS = 25.0f;
const float SEPARATION_FORCE = 5.0f; // Reduced drastically to avoid explosions

// Formation configuration
const int GRID_ROWS_OFFSET = 8; // Offset of rows to raise the formation
const int GRID_COLS = 250;

// ==============================
// PHYSICS
// ==============================
const float GRAVITY = 9.81f;
const float AIR_FRICTION = 0.1f;
const float GROUND_FRICTION = 0.5f;
const float ENERGY_RESTITUTION = 0.7f;
const float COLLISION_EPSILON = 0.001f;
const float GROUND_EPSILON = 0.01f;

// ==============================
// PID CONTROL PARAMETERS
// ==============================

// Control en X (Ángulo)
const float PID_X_KP = 0.7f;
const float PID_X_KI = 0.0f;
const float PID_X_KD = 2.1f;
const float PID_X_MAX_INTEGRAL = 10.0f;

// Control en Y (Empuje/Thrust)
const float PID_Y_KP = 0.4f;
const float PID_Y_KI = 0.005f;
const float PID_Y_KD = 1.3f;
const float PID_Y_MAX_INTEGRAL = 20.0f;

// ==============================
// ACTUATORS & LIMITS
// ==============================
const float MAX_THRUST = 50.0f;
const float MIN_THRUST = 0.0f;
const float MAX_ANGLE  = 0.5f;
const float MIN_ANGLE  = -0.5f;
const float MAX_VELOCITY = 100.0f;

// ==============================
// AI & BATTERY
// ==============================
const float BATTERY_DRAIN_MIN = 0.1f;
const float BATTERY_DRAIN_MAX = 0.5f;
const float BATTERY_CHARGE_SPEED = 2.0f;

// ==============================
// NETWORK
// ==============================
const std::string SERVER_IP = "127.0.0.1";
const unsigned short SERVER_PORT = 9999;

#endif // GLOBAL_CONFIG_H