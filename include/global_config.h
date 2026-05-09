#ifndef GLOBAL_CONFIG_H
#define GLOBAL_CONFIG_H

#include "utils/vector2.h"
#include <string>

// ==============================
// WORLD & SIMULATION
// ==============================
constexpr bool HEADLESS_MODE = true; // Servidor Nube: Si es true, calcula físicas pero NO dibuja nada.
constexpr bool VIEWER_MODE = false;  // Cliente PC: Si es true, NO calcula físicas, solo recibe datos y dibuja.

constexpr Vector2 WINDOW_SIZE = {1280.0f, 720.0f};      
constexpr Vector2 WORLD_SIZE  = {100000.0f, 100000.0f}; 
constexpr float DT = 0.016f;

// ==============================
// DRONE SPAWN & SWARM
// ==============================

// General
constexpr int DRONE_COUNT = 10000;
constexpr int SPAWN_INTERVAL = 1;
constexpr float DRONE_SIZE = 4.0f;
constexpr int BUILDINGS_TO_USE = -1; // -1 = all
constexpr float TIME_SCALE = 1.0f;    // 1.0 = Normal, 0.2 = Slow Motion

// Flocking
constexpr float SEPARATION_RADIUS = 25.0f;
constexpr float SEPARATION_FORCE = 5.0f; 

// Matrix Formation
constexpr int DRONES_PER_ROW = 10;     
constexpr float FORMATION_SPACING_X = 35.0f;
constexpr float FORMATION_SPACING_Y = 35.0f;
constexpr float MATRIX_ERROR_TOLERANCE = 10.0f;
constexpr float MATRIX_AI_INTERVALE = 5.0f;

// ==============================
// CITY GENERATION
// ==============================
constexpr int   BUILDING_PROBABILITY = 85;  // % de tener un edificio vs espacio vacío
constexpr float BUILDING_MIN_WIDTH = 80.0f;
constexpr float BUILDING_MAX_WIDTH = 280.0f;
constexpr float BUILDING_MIN_HEIGHT = 150.0f;
constexpr float BUILDING_MAX_HEIGHT = 750.0f;

// Probabilidades de Tipos (Suma acumulada de 0 a 100)
constexpr int PROB_CHARGER   = 10; // 0-15 (15%)
constexpr int PROB_COLLECT   = 15; // 15-30 (15%)
constexpr int PROB_DEPLOY    = 15;
constexpr int PROB_SPAWN     = 20; // 30-55 (25%)
// Obstacles = 100 - (PROB_CHARGER + PROB_COLLECT + PROB_SPAWN)

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

// X Control (Angle)
constexpr float PID_KP_ANGLE = 0.4f;
constexpr float PID_KI_ANGLE = 0.0f;
constexpr float PID_KD_ANGLE = 2.1f;
constexpr float PID_MAX_I_ANGLE = 10.0f;

// Y Control (Thrust)
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
constexpr float BATTERY_DRAIN = 0.5f;
constexpr float BATTERY_CHARGE_SPEED = 2.0f;

// ==============================
// NETWORK
// ==============================
const std::string SERVER_IP = "134.199.196.239";
const unsigned short SERVER_PORT = 9999;

// ==============================
// AI
// ==============================
constexpr int LLM_QUERY_INTERVAL = 120;

#endif // GLOBAL_CONFIG_H