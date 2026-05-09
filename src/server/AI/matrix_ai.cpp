#include "AI/matrix_ai.h"
#include "AI/states.h"
#include "utils/math_utils.h"
#include "global_config.h"
#include <iostream>

void update_matrix_ai(MatrixGroup& matrix, const world& vWorld, std::vector<DroneChassis>& drones) {
    float error = matrix.getGlobalError(drones);
    
    // FASE 1: Esperar Estabilidad (TAKEOFF / REGROUPING)
    if (matrix.current_action == toInt(DroneAction::TAKEOFF)) {
        if (matrix.isFull() && error < MATRIX_ERROR_TOLERANCE) {
            std::cout << "[MATRIX " << matrix.id << "] Formation full and stable. Transitioning to MISSION_ACTIVE." << std::endl;
            matrix.current_action = toInt(DroneAction::FOLLOW_MATRIX);
            matrix.mission_timer = 0.0f; 
            
            // Iniciar movimiento al destino final
            matrix.moveTo(matrix.final_target);
        }
    }

    // FASE 2: Mision Activa / Navegacion
    if (matrix.current_action == toInt(DroneAction::FOLLOW_MATRIX)) {
        // En esta fase, los drones simplemente siguen a la matriz hacia su final_target.
        // Podríamos agregar lógica aquí para detectar si llegaron al destino,
        // pero el wandering aleatorio ya no es necesario.
    }
}
