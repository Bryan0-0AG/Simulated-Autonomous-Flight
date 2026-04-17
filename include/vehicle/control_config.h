#ifndef CONTROL_CONFIG_H
#define CONTROL_CONFIG_H

// ==============================
// PID PARAMETERS
// ==============================

struct PIDOutput {
    float p;
    float i;
    float d;
};

// Control en X (ángulo)
constexpr float PID_X_KP = 0.1f;
constexpr float PID_X_KI = 0.0f;    // Sin integral en X: evita windup lateral
constexpr float PID_X_KD = 0.2f;

// Control en Y (thrust)
constexpr float PID_Y_KP = 0.2f;
constexpr float PID_Y_KI = 0.005f;  // Reducido: era 0.02, causa del overshoot en targets lejanos
constexpr float PID_Y_KD = 0.4f;    // Aumentado: más amortiguación al aproximarse

// Límite de saturación del integrador (anti-windup)
// Impide que el término integral acumule más allá de este valor durante
// trayectos largos donde el error persiste mucho tiempo.
constexpr float PID_X_MAX_INTEGRAL = 10.0f;
constexpr float PID_Y_MAX_INTEGRAL = 20.0f;


// ==============================
// ACTUATOR
// ==============================

struct ActuatorOutput {
    float thrust;
    float angle;
};

// Máximo empuje
constexpr float MAX_THRUST = 50.0f;

// Mínimo empuje
constexpr float MIN_THRUST = 0.0f;

// Ángulo máximo (radianes)
constexpr float MAX_ANGLE = 0.5f;   // ~28 grados

// Ángulo mínimo
constexpr float MIN_ANGLE = -0.5f;


// ==============================
// PHYSICAL DAMPING
// ==============================

// Factor de amortiguación (simula resistencia del aire)
constexpr float LINEAR_DAMPING = 0.98f;


// ==============================
// SAFETY / STABILITY
// ==============================

// Límite de velocidad (opcional)
constexpr float MAX_VELOCITY = 100.0f;

// Deadzone para evitar micro-oscilaciones
constexpr float POSITION_EPSILON = 0.01f;

#endif