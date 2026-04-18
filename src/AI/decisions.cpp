#include "AI/decisions.h"
#include "utils/math_utils.h"
#include <cmath>

void update_ai_decisions(Body& body, World& world) {
    // 1. STATES CONTROL
    switch (body.current_state) {
        case DroneState::FLYING:
            if(body.battery > 0.0f) {
                body.battery -= static_cast<float>(randint(5, 10));
            } else {
                body.battery = 0.0f;
            }
            if (body.battery < 20.0f && body.current_action != DroneAction::RETURNING_TO_BASE) {
                body.original_target = body.target;
                body.current_action = DroneAction::RETURNING_TO_BASE;
            }
            break;
            
        case DroneState::LANDED:
            body.battery += static_cast<float>(randint(1, 5));
            if (body.battery >= body.max_battery) {
                body.battery = body.max_battery;
                body.current_action = DroneAction::FLYING_TO_TARGET;
            }
            break;         
    }

    // 2. ACTIONS CONTROL
    switch (body.current_action) {
        case DroneAction::RETURNING_TO_BASE:
            // FASE 1: Volver a casa (X=0) manteniendo la altura actual
            body.target.x = 0.0f + body.size;
            
            // FASE 2: Si ya llegamos a casa en X, procedemos a aterrizar
            if (std::abs(body.position.x - body.target.x) < 5.0f) {
                float realgroundY = world.groundY + body.size;
                body.target.y = realgroundY;
                
                // Si ya estamos a nivel del suelo
                if (body.position.y < realgroundY + 5.0f) {
                    body.current_state = DroneState::LANDED;
                }
            }
            break;
            
        case DroneAction::FLYING_TO_TARGET:
            body.target = body.original_target;
            body.current_state = DroneState::FLYING;
            break;
    }
}
