#include "vehicle/pid.h"

PID::PID(float kp, float ki, float kd)
    : kp(kp), ki(ki), kd(kd), integral(0), previous_error(0) {}

float PID::update(float error, float dt) {
    integral += error * dt;
    float derivative = (error - previous_error) / dt;

    previous_error = error;

    return kp * error + ki * integral + kd * derivative;
}