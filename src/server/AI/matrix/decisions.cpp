#include "AI/matrix/decisions.h"
#include "AI/matrix/perceptions.h"
#include "AI/matrix/actions.h"
#include "AI/matrix/states.h"
#include <iostream>

#include "AI/matrix/properties.h"
#include "AI/matrix/highways.h"
#include "global_config.h"

namespace MatrixAI {
namespace Decisions {

    void evaluateAndDecide(MatrixGroup& self, const std::vector<MatrixGroup>& all_matrices, const std::vector<DroneChassis>& drones, float dt) {
        
        // 1. Actualizar Timer y Estados Generales
        States::updateState(self, dt);
        
        // 2. Comportamiento según el Estado Actual
        if (self.current_action == States::toInt(States::MatrixAction::TAKEOFF)) {
            float error = Properties::getGlobalError(self, drones);
            if (self.isFull() && error < MATRIX_ERROR_TOLERANCE) {
                std::cout << "[MATRIX " << self.id << "] Formation full and stable. Transitioning to FOLLOW_MATRIX." << std::endl;
                States::transitionState(self, States::toInt(States::MatrixAction::FOLLOW_MATRIX));
                Actions::executeMove(self, self.final_target);
            }
        } 
        else if (self.current_action == States::toInt(States::MatrixAction::FOLLOW_MATRIX)) {
            // Usar Highway Manager para obtener el siguiente paso (Ascenso, Crucero, Descenso)
            Vector2 next_waypoint = Highways::calculateNextWaypoint(self);

            // Elegimos la dirección de visión base dependiendo de si vamos de subida, crucero o bajada
            Perceptions::Direction vision_dir;
            if (next_waypoint.x > self.center.x) vision_dir = Perceptions::Direction::RIGHT;
            else if (next_waypoint.x < self.center.x) vision_dir = Perceptions::Direction::LEFT;
            else if (next_waypoint.y > self.center.y) vision_dir = Perceptions::Direction::UP;
            else vision_dir = Perceptions::Direction::DOWN;

            // Evaluamos percepciones en nuestro carril
            bool danger = Perceptions::Vision(self, vision_dir, 150.0f, all_matrices);
            if (danger) {
                Actions::executeWait(self, dt);
            } else {
                Actions::executeMove(self, next_waypoint);
            }
        }
    }

}
}
