#include "AI/matrix_ai.h"
#include "AI/states.h"
#include "utils/math_utils.h"
#include "global_config.h"
#include <iostream>

void update_matrix_ai(MatrixGroup& matrix, const world& vWorld, const std::vector<DroneChassis>& drones) {
    float error = matrix.getGlobalError(drones);
    
    // FASE 1: Esperar Estabilidad (TAKEOFF / REGROUPING)
    if (matrix.current_action == toInt(DroneAction::TAKEOFF)) {
        // DEBUG: Solo para la primera matriz para no inundar la consola
        if (matrix.id == -1) {
            std::cout << "[DEBUG MATRIX " << matrix.id << "] isFull: " << (matrix.isFull()?"YES":"NO") 
                      << " | Drones: " << matrix.children.size() << "/" << matrix.target_count
                      << " | Error: " << error << " (Tol: " << MATRIX_ERROR_TOLERANCE << ")" << std::endl;
        }

        // Solo verificamos estabilidad si la matriz ya tiene todos sus drones reales
        if (matrix.isFull() && error < MATRIX_ERROR_TOLERANCE) {
            std::cout << "[MATRIX " << matrix.id << "] Formation full and stable. Transitioning to MISSION_ACTIVE." << std::endl;
            matrix.current_action = toInt(DroneAction::FOLLOW_MATRIX);
            matrix.mission_timer = 0.0f; 
        }
    }

    // FASE 2: Misión Activa / Navegación
    if (matrix.current_action == toInt(DroneAction::FOLLOW_MATRIX)) {
        matrix.mission_timer += 0.016f;

        // Cada matriz tiene un tiempo de espera ligeramente distinto según su ID (ej: entre 8 y 13 segundos)
        float jitter_interval = MATRIX_AI_INTERVALE + static_cast<float>(matrix.id % 6);

        if (error < MATRIX_ERROR_TOLERANCE && matrix.mission_timer > jitter_interval) {
            // Límites del mundo relativos a la posición actual
            float xmin = std::max(100.0f, matrix.center.x - 3000.0f);
            float xmax = std::min(4900.0f, matrix.center.x + 3000.0f);
            
            // Garantizar que min <= max por si la matrix está fuera de límites
            if (xmin > xmax) std::swap(xmin, xmax);

            float ymin = 1200.0f; 
            float ymax = 3800.0f; 

            float randX = static_cast<float>(randint(xmin, xmax));
            float randY = static_cast<float>(randint(ymin, ymax));
            
            std::cout << "[MATRIX " << matrix.id << "] Patrol complete. Jumping to {" << (int)randX << ", " << (int)randY << "}." << std::endl;
            
            matrix.moveTo({randX, randY});
            
            // VOLVEMOS A TAKEOFF: Ahora cada matriz esperará a su propio ritmo
            matrix.current_action = toInt(DroneAction::TAKEOFF);
            matrix.mission_timer = 0.0f;
        }
    }
}
