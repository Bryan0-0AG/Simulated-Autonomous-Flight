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

// Aerial Space - Two-tier altitude system
// Tier 1: STAGING ZONE - matrices form here (above buildings, below highway)
constexpr float STAGING_ALTITUDE_BASE = 1000.0f;   // Base altitude for staging (well above tallest buildings ~750)
constexpr float STAGING_LANE_HEIGHT = 500.0f;       // Vertical separation between staging slots (exceeds max matrix height)
constexpr int   MAX_STAGING_LANES = 6;              // Parallel staging bands

// Tier 2: HIGHWAY ZONE - matrices travel here (well above staging)
constexpr int   MAX_AERIAL_LANES = 6;              // Total number of highway lanes
constexpr float CRUISE_ALTITUDE_BASE = 4500.0f;    // Base altitude for lowest highway lane (above highest staging slot)
constexpr float LANE_HEIGHT = 400.0f;              // Vertical separation between highway lanes (must exceed tallest matrix)
constexpr float MATRIX_SAFETY_MARGIN = 80.0f;      // Extra padding around each matrix bounding box
constexpr float COLLISION_REPULSION_FORCE = 40.0f;  // Strength of repulsion when bounds overlap
constexpr float SAME_LANE_LOOKAHEAD = 800.0f;       // Distance to start braking for same-lane matrices
constexpr float MATRIX_MOVE_SPEED = 80.0f;          // Units per second the matrix center advances toward nav_target
constexpr float FORWARD_VISION_DISTANCE = 1200.0f;  // How far ahead the matrix "sees" obstacles

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
const std::string SERVER_IP = "127.0.0.1";
const unsigned short SERVER_PORT = 9999;

// ==============================
// AI
// ==============================
constexpr int LLM_QUERY_INTERVAL = 200;

// ==============================
// MISSIONS
// ==============================

constexpr int MISSION_SPAWN_AMOUNT = 50;
constexpr int PACKAGES_PER_MISSION_MIN = 300;
constexpr int PACKAGES_PER_MISSION_MAX = 700;
// Seconds to wait between launching each queued mission.
// Gives the previous mission's matrices time to clear the spawn zone before the next arrives.
constexpr float MISSION_DISPATCH_DELAY = 8.0f;


#endif // GLOBAL_CONFIG_H