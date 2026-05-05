#include <algorithm>
#include "control/pid.h"

PID::PID(float kp, float ki, float kd, float max_integral)
    : kp(kp), ki(ki), kd(kd),
      integral(0.0f), previous_error(0.0f),
      max_integral(max_integral) {}

PIDOutput PID::update(float error, float dt) {
    PIDOutput output;
    
    // Acumular integral con anti-windup: saturar en ±max_integral
    integral += error * dt;
    integral = std::clamp(integral, -max_integral, max_integral);

    float derivative = (error - previous_error) / dt;
    previous_error = error;

    output.p = kp * error;
    output.i = ki * integral;
    output.d = kd * derivative;
    
    return output;
}

void PID::reset() {
    integral       = 0.0f;
    previous_error = 0.0f;
}