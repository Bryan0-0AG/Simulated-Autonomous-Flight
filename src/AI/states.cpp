#include "AI/states.h"

std::string toString(DroneAction a) {
    switch(a) {
        case DroneAction::FLYING_TO_TARGET: return "FLYING_TO_TARGET";
        case DroneAction::RETURNING_TO_BASE: return "RETURNING_TO_BASE";
    }
    return "UNKNOWN";
}

std::string toString(DroneState s) {
    switch(s) {
        case DroneState::FLYING: return "FLYING";
        case DroneState::LANDED: return "LANDED";
    }
    return "UNKNOWN";
}