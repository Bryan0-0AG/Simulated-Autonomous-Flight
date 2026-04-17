#ifndef PID_H
#define PID_H

class PID {
public:
    PID(float kp, float ki, float kd);

    float update(float error, float dt);

private:
    float kp, ki, kd;
    float integral;
    float previous_error;
};

#endif