#ifndef PID_H
#define PID_H

#include "control_config.h"

// PID con anti-windup: el integral se satura en ±max_integral
// para evitar acumulación excesiva cuando el actuador está saturado
// o durante trayectos largos hacia el target.
class PID {
public:
    // max_integral: límite de saturación del término integral
    PID(float kp, float ki, float kd, float max_integral = 50.0f);

    PIDOutput update(float error, float dt);
    void reset();   // Reinicia integral y error previo (útil al cambiar target)

private:
    float kp, ki, kd;
    float integral;
    float previous_error;
    float max_integral;
};

#endif