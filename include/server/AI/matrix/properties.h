#pragma once
#include "AI/matrix/matrix_group.h"
#include <vector>

namespace MatrixAI {
namespace Properties {
    
    // Obtener el error global de la matriz (qué tan desordenados están los drones)
    float getGlobalError(const MatrixGroup& matrix, const std::vector<DroneChassis>& drones);
    Vector2 getVectorError(const MatrixGroup& matrix, const std::vector<DroneChassis>& drones);
    
    // Funciones de Telemetría (Promedios)
    Vector2 getAveragePosition(const MatrixGroup& matrix, const std::vector<DroneChassis>& drones);
    Vector2 getAverageVelocity(const MatrixGroup& matrix, const std::vector<DroneChassis>& drones);
    float getAverageBattery(const MatrixGroup& matrix, const std::vector<DroneChassis>& drones);
    Vector2 getAverageRepulsionForce(const MatrixGroup& matrix, const std::vector<DroneChassis>& drones);
    float getAverageThrust(const MatrixGroup& matrix, const std::vector<DroneChassis>& drones);
    float getAverageAngle(const MatrixGroup& matrix, const std::vector<DroneChassis>& drones);
    
    struct Rect { Vector2 min, max; };
    
    // Obtener la caja de colisión (AABB) de la matriz completa
    Rect getBounds(const MatrixGroup& matrix);
    Rect getDynamicBounds(const MatrixGroup& matrix, const std::vector<DroneChassis>& drones);

    // Calcular posicion de un slot en la matriz
    Vector2 getSlotPosition(const MatrixGroup& matrix, int row, int col);

    // Aplicar fuerzas de repulsión entre matrices basadas en sus posiciones dinámicas (centroides reales)
    void applyDynamicRepulsion(MatrixGroup& self, const std::vector<MatrixGroup>& all_matrices, const std::vector<DroneChassis>& drones, float dt);

    // Integración física del movimiento de la matriz (Suavizado)
    void updateMatrixPhysics(MatrixGroup& self, float dt);

    // Registrar dron en la matriz
    void insertChild(MatrixGroup& matrix, int droneId, int r, int c);

}
}
