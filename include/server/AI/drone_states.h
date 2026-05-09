#ifndef STATES_H
#define STATES_H

#include <string>

enum class DroneAction {
    TAKEOFF,
    FOLLOW_MATRIX,
    GOING_TO_RECHARGE,
    RECHARGING,
    REJOINING_MATRIX,
    LANDING
};

enum class DroneState {
    FLYING,
    LANDED,
    EMERGENCY
};


std::string toString(DroneAction a);
std::string toString(DroneState s);

// GPU Compatibility: Explicit conversion functions
int toInt(DroneAction a);
int toInt(DroneState s);
DroneAction toAction(int a);
DroneState toState(int s);

#endif
