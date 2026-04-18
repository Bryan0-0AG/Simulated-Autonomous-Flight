#ifndef STATES_H
#define STATES_H

#include <string>

enum class DroneAction {
    FLYING_TO_TARGET,  // Modo de vuelo normal hacia su objetivo
    RETURNING_TO_BASE  // Volviendo a la base por batería baja (Y = 0.0f)
};

enum class DroneState {
    FLYING,
    LANDED
};

std::string toString(DroneAction a);
std::string toString(DroneState s);

#endif
