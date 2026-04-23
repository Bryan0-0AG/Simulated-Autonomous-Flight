#ifndef STATES_H
#define STATES_H

#include <string>

enum class DroneAction {
    FLYING_TO_TARGET,  
    RETURNING_TO_BASE  
};

enum class DroneState {
    FLYING,
    LANDED
};

std::string toString(DroneAction a);
std::string toString(DroneState s);

#endif
