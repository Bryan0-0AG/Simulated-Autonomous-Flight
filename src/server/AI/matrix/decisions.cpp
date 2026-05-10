#include "AI/matrix/decisions.h"
#include "AI/matrix/actions.h"
#include "AI/matrix/states.h"
#include <iostream>

#include "AI/matrix/properties.h"
#include "AI/matrix/highways.h"
#include "global_config.h"

#include "AI/matrix/perceptions.h"

namespace MatrixAI {
namespace Decisions {

    void evaluateAndDecide(MatrixGroup& self, const std::vector<MatrixGroup>& all_matrices, const std::vector<DroneChassis>& drones, float dt) {
        
        // 1. Actualizar Timer y Estados Generales
        States::updateState(self, dt);
        
        // 2. Comportamiento según el Estado Actual
        if (self.current_action == States::toInt(States::MatrixAction::TAKEOFF)) {
            
            // VERIFICACIÓN VERTICAL: ¿Hay alguien encima?
            // Miramos hacia arriba (UP) en un rango de 600 unidades
            if (Perceptions::Vision(self, Perceptions::Direction::UP, 600.0f, all_matrices, drones)) {
                Actions::executeWait(self, dt, drones);
                return;
            }

            float error = Properties::getGlobalError(self, drones);
            
            // Ya no usamos radar, la repulsión dinámica se encarga de los espacios
            if (self.isFull() && error < MATRIX_ERROR_TOLERANCE) {
                std::cout << "[MATRIX " << self.id << "] Formation full and stable. Transitioning to FOLLOW_MATRIX." << std::endl;
                States::transitionState(self, States::toInt(States::MatrixAction::FOLLOW_MATRIX));
                Actions::executeMove(self, self.final_target);
            }
        } 
        else if (self.current_action == States::toInt(States::MatrixAction::FOLLOW_MATRIX)) {
            Vector2 next_waypoint = Highways::calculateNextWaypoint(self);
            // Movimiento directo, la repulsión dinámica en SwarmManager desvía el centro si hay conflicto
            Actions::executeMove(self, next_waypoint);
        }
        else if (self.current_action == States::toInt(States::MatrixAction::WAITING)) {
            // Solo reanudamos si el camino hacia arriba está despejado
            if (self.current_state == States::toInt(States::MatrixState::STAGING)) {
                if (!Perceptions::Vision(self, Perceptions::Direction::UP, 800.0f, all_matrices, drones)) {
                    self.current_action = States::toInt(States::MatrixAction::TAKEOFF);
                }
            } else {
                // Para el seguimiento normal en autopista, reanudamos
                self.current_action = States::toInt(States::MatrixAction::FOLLOW_MATRIX);
            }
        }
    }

}
}
