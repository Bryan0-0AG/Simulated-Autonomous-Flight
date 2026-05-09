#pragma once
#include "AI/matrix/matrix_group.h"

namespace MatrixAI {
namespace Actions {

    // Cambia el objetivo de la matriz y ajusta los drones
    void executeMove(MatrixGroup& self, Vector2 new_target);

    // Mantiene la matriz en su lugar
    void executeWait(MatrixGroup& self, float dt);

    // Reformatea la matriz dinámicamente
    void reshape(MatrixGroup& self, int newCols, std::vector<DroneChassis>& drones);

}
}
