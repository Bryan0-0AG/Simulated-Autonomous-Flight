#include "AI/drone_states.h"

std::string toString(DroneAction a) {
    switch(a) {
        case DroneAction::TAKEOFF:          return "TAKEOFF";
        case DroneAction::FOLLOW_MATRIX:    return "FOLLOWING_MATRIX";
        case DroneAction::GOING_TO_RECHARGE: return "GOING_TO_RECHARGE";
        case DroneAction::RECHARGING:       return "RECHARGING";
        case DroneAction::REJOINING_MATRIX: return "REJOINING_MATRIX";
        case DroneAction::LANDING:          return "LANDING";
    }
    return "UNKNOWN_ACTION";
}

std::string toString(DroneState s) {
    switch(s) {
        case DroneState::FLYING: return "FLYING";
        case DroneState::LANDED: return "LANDED";
        case DroneState::EMERGENCY: return "EMERGENCY";
    }
    return "UNKNOWN_STATE";
}

int toInt(DroneAction a) { return static_cast<int>(a); }
int toInt(DroneState s)  { return static_cast<int>(s); }

DroneAction toAction(int a) {
    switch(a) {
        case 0: return DroneAction::TAKEOFF;
        case 1: return DroneAction::FOLLOW_MATRIX;
        case 2: return DroneAction::GOING_TO_RECHARGE;
        case 3: return DroneAction::RECHARGING;
        case 4: return DroneAction::REJOINING_MATRIX;
        case 5: return DroneAction::LANDING;
        default: return DroneAction::FOLLOW_MATRIX;
    }
}

DroneState toState(int s) {
    switch(s) {
        case 0: return DroneState::FLYING;
        case 1: return DroneState::LANDED;
        case 2: return DroneState::EMERGENCY;
        default: return DroneState::LANDED;
    }
}
