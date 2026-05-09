#pragma once
#include "AI/matrix/matrix_group.h"
#include "utils/vector2.h"

namespace MatrixAI {
namespace Highways {

    // Calcula la altitud (capa) asignada para la matriz dependiendo de su direccion de viaje
    float getAssignedAltitude(const MatrixGroup& self);

    // Asigna el carril y la altitud inicial basada en el destino final
    void assignLane(MatrixGroup& self);

    // Devuelve el proximo "Waypoint" basado en el movimiento de 3 Fases (Ascenso, Crucero, Descenso)
    Vector2 calculateNextWaypoint(const MatrixGroup& self);

}
}
