#pragma once
#include "AI/matrix/matrix_group.h"

namespace MatrixAI {
namespace States {

    enum class MatrixAction {
        TAKEOFF,
        FOLLOW_MATRIX,
        WAITING,
        LANDING
    };

    enum class MatrixState {
        STAGING,
        MISSION_ACTIVE,
        HOLDING
    };

    int toInt(MatrixAction a);
    int toInt(MatrixState s);
    MatrixAction toAction(int a);
    MatrixState toState(int s);

    // Actualiza el estado global de la matriz (ej. actualizar timer)
    void updateState(MatrixGroup& self, float dt);

    // Transicion manual de estado
    void transitionState(MatrixGroup& self, int new_action);

}
}
