#pragma once
#include "AI/matrix/matrix_group.h"
#include <vector>

namespace MatrixAI {
namespace Decisions {

    // Evalua las percepciones y decide qué accion tomar (Reemplaza a update_matrix_ai)
    void evaluateAndDecide(MatrixGroup& self, const std::vector<MatrixGroup>& all_matrices, const std::vector<DroneChassis>& drones, float dt);

}
}
