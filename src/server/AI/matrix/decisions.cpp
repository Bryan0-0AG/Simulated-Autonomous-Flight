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
            
            // Usamos radar durante el despegue con rangos X/Y separados
            bool danger = false;
            for (const auto& other : all_matrices) {
                if (other.id == self.id) continue;
                if (Perceptions::Radar(self, 400.0f, 250.0f, {other}, drones)) {
                    if (Perceptions::ShouldIWait(self, other)) {
                        danger = true;
                        break;
                    }
                }
            }

            if (danger) {
                Actions::executeWait(self, dt, drones);
            } else if (self.isFull() && error < MATRIX_ERROR_TOLERANCE) {
                std::cout << "[MATRIX " << self.id << "] Formation full, stable and clear. Transitioning to FOLLOW_MATRIX." << std::endl;
                States::transitionState(self, States::toInt(States::MatrixAction::FOLLOW_MATRIX));
                Actions::executeMove(self, self.final_target);
            }
        } 
        else if (self.current_action == States::toInt(States::MatrixAction::FOLLOW_MATRIX)) {
            Vector2 next_waypoint = Highways::calculateNextWaypoint(self);

            Perceptions::Direction vision_dir;
            if (next_waypoint.x > self.center.x) vision_dir = Perceptions::Direction::RIGHT;
            else if (next_waypoint.x < self.center.x) vision_dir = Perceptions::Direction::LEFT;
            else if (next_waypoint.y > self.center.y) vision_dir = Perceptions::Direction::UP;
            else vision_dir = Perceptions::Direction::DOWN;

            // Visión con negociación
            bool danger = false;
            if (Perceptions::Vision(self, vision_dir, 200.0f, all_matrices, drones)) {
                // Si vemos a alguien, negociamos
                for (const auto& other : all_matrices) {
                    if (other.id == self.id) continue;
                    if (Perceptions::Vision(self, vision_dir, 200.0f, {other}, drones)) {
                        if (Perceptions::ShouldIWait(self, other)) {
                            danger = true;
                            break;
                        }
                    }
                }
            }

            if (danger) {
                Actions::executeWait(self, dt, drones);
            } else {
                Actions::executeMove(self, next_waypoint);
            }
        }
        else if (self.current_action == States::toInt(States::MatrixAction::WAITING)) {
            // Re-evaluar peligro con negociación para reanudar
            bool danger = false;
            
            // Radar check
            for (const auto& other : all_matrices) {
                if (other.id == self.id) continue;
                if (Perceptions::Radar(self, 400.0f, 250.0f, {other}, drones)) {
                    if (Perceptions::ShouldIWait(self, other)) {
                        danger = true;
                        break;
                    }
                }
            }

            // Vision check si ya estábamos en misión
            if (!danger && self.current_state == States::toInt(States::MatrixState::MISSION_ACTIVE)) {
                Vector2 next_waypoint = Highways::calculateNextWaypoint(self);
                Perceptions::Direction vision_dir;
                if (next_waypoint.x > self.center.x) vision_dir = Perceptions::Direction::RIGHT;
                else if (next_waypoint.x < self.center.x) vision_dir = Perceptions::Direction::LEFT;
                else if (next_waypoint.y > self.center.y) vision_dir = Perceptions::Direction::UP;
                else vision_dir = Perceptions::Direction::DOWN;

                if (Perceptions::Vision(self, vision_dir, 200.0f, all_matrices, drones)) {
                    for (const auto& other : all_matrices) {
                        if (other.id == self.id) continue;
                        if (Perceptions::Vision(self, vision_dir, 200.0f, {other}, drones)) {
                            if (Perceptions::ShouldIWait(self, other)) {
                                danger = true;
                                break;
                            }
                        }
                    }
                }
            }

            if (!danger) {
                self.center = self.current_target; 
                if (self.current_state == States::toInt(States::MatrixState::STAGING)) {
                    self.current_action = States::toInt(States::MatrixAction::TAKEOFF);
                } else {
                    self.current_action = States::toInt(States::MatrixAction::FOLLOW_MATRIX);
                }
            }
        }
    }

}
}
