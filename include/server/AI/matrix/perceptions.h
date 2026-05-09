#pragma once
#include "AI/matrix/matrix_group.h"
#include <vector>
#include <string>

namespace MatrixAI {
namespace Perceptions {

    enum class Direction { 
        UP, DOWN, LEFT, RIGHT,
        UP_RIGHT, UP_LEFT, DOWN_RIGHT, DOWN_LEFT 
    };

    // Devuelve true si hay algun obstaculo (otra matriz) en la direccion especificada
    bool Vision(const MatrixGroup& self, Direction dir, float distance, const std::vector<MatrixGroup>& all_matrices, const std::vector<DroneChassis>& drones);

    // 2. Radar: Detecta globalmente si hay otra matriz en un radio usando posiciones dinamicas (separado por ejes)
    bool Radar(const MatrixGroup& self, float radiusX, float radiusY, const std::vector<MatrixGroup>& all_matrices, const std::vector<DroneChassis>& drones);

    // 3. Communication: Negociación de prioridad entre dos matrices en conflicto
    bool ShouldIWait(const MatrixGroup& self, const MatrixGroup& other);

}
}
