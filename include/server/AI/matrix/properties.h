#pragma once
#include "AI/matrix/matrix_group.h"
#include <vector>

namespace MatrixAI {
namespace Properties {
    
    // Obtener el error global de la matriz (qué tan desordenados están los drones)
    float getGlobalError(const MatrixGroup& matrix, const std::vector<DroneChassis>& drones);
    
    struct Rect { Vector2 min, max; };
    
    // Obtener la caja de colisión (AABB) de la matriz completa
    Rect getBounds(const MatrixGroup& matrix);

    // Calcular posicion de un slot en la matriz
    Vector2 getSlotPosition(const MatrixGroup& matrix, int row, int col);

    // Registrar dron en la matriz
    void insertChild(MatrixGroup& matrix, int droneId, int r, int c);

}
}
