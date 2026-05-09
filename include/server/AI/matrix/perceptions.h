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

    // 1. Vision: Verifica en una direccion especifica hasta cierta distancia
    bool Vision(const MatrixGroup& self, Direction dir, float distance, const std::vector<MatrixGroup>& all_matrices);

    // 2. Radar: Detecta globalmente si hay otra matriz en un radio
    bool Radar(const MatrixGroup& self, float radius, const std::vector<MatrixGroup>& all_matrices);

    // 3. Communication: Escucha y procesa ordenes externas
    bool Communication(const MatrixGroup& self, std::string& out_message);

}
}
