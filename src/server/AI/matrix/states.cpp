#include "AI/matrix/states.h"
#include "utils/math_utils.h"
#include <cmath>

namespace MatrixAI {
namespace States {

    int toInt(MatrixAction a) { return static_cast<int>(a); }
    int toInt(MatrixState s) { return static_cast<int>(s); }

    MatrixAction toAction(int a) {
        switch(a) {
            case 0: return MatrixAction::TAKEOFF;
            case 1: return MatrixAction::FOLLOW_MATRIX;
            case 2: return MatrixAction::WAITING;
            case 3: return MatrixAction::LANDING;
            default: return MatrixAction::WAITING;
        }
    }

    MatrixState toState(int s) {
        switch(s) {
            case 0: return MatrixState::STAGING;
            case 1: return MatrixState::MISSION_ACTIVE;
            case 2: return MatrixState::HOLDING;
            default: return MatrixState::STAGING;
        }
    }

    std::string toString(MatrixAction a) {
        switch(a) {
            case MatrixAction::TAKEOFF: return "TAKEOFF";
            case MatrixAction::FOLLOW_MATRIX: return "FOLLOW_MATRIX";
            case MatrixAction::WAITING: return "WAITING";
            case MatrixAction::LANDING: return "LANDING";
            default: return "UNKNOWN";
        }
    }

    std::string toString(MatrixState s) {
        switch(s) {
            case MatrixState::STAGING: return "STAGING";
            case MatrixState::MISSION_ACTIVE: return "MISSION_ACTIVE";
            case MatrixState::HOLDING: return "HOLDING";
            default: return "UNKNOWN";
        }
    }

    void updateState(MatrixGroup& self, float dt) {
        self.mission_timer += dt;
        
        // Logic to change state if a time limit is met
        // Example: If waiting for more than 60s, force something else
        if (self.current_action == toInt(MatrixAction::WAITING) && self.mission_timer > 10.0f) {
            transitionState(self, toInt(MatrixAction::FOLLOW_MATRIX));
        }
    }

    void transitionState(MatrixGroup& self, int new_action) {
        self.current_action = new_action;
        self.mission_timer = 0.0f;
    }

}
}
