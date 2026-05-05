#include "AI/decisions.h"
#include "utils/math_utils.h"
#include "global_config.h"
#include <cmath>

void update_ai_decisions(DroneChassis& DroneChassis, const world& virtualWorld) {
    // 1. STATES CONTROL
    switch (DroneChassis.current_state) {
        case DroneState::FLYING:
            // Si el dron toca el suelo (caída o aterrizaje), cambia a LANDED
            if (DroneChassis.position.y <= 4.1f) {
                DroneChassis.current_state = DroneState::LANDED;
                DroneChassis.velocity = {0.0f, 0.0f};
            }
            // Batería baja -> Volver a casa
            if (DroneChassis.battery < 20.0f && DroneChassis.current_action != DroneAction::RETURNING_TO_BASE) {
                DroneChassis.original_target = DroneChassis.target;
                DroneChassis.current_action = DroneAction::RETURNING_TO_BASE;
            }
            break;
            
        case DroneState::LANDED:
            DroneChassis.battery += BATTERY_CHARGE_SPEED;
            if (DroneChassis.battery >= DroneChassis.max_battery) {
                DroneChassis.battery = DroneChassis.max_battery;
                DroneChassis.current_action = DroneAction::FLYING_TO_TARGET;
                DroneChassis.current_state = DroneState::FLYING;
            }
            break;         
    }
    
    // 2. ACTIONS CONTROL
    switch (DroneChassis.current_action) {
        case DroneAction::RETURNING_TO_BASE:
            // Volver al punto de carga (X=4)
            DroneChassis.target.x = 4.0f;
            
            // Si estamos cerca de la vertical de la base, bajar
            if (std::abs(DroneChassis.position.x - DroneChassis.target.x) < 10.0f) {
                DroneChassis.target.y = 4.0f;
            }
            break;
            
        case DroneAction::FLYING_TO_TARGET:
            DroneChassis.target = DroneChassis.original_target;
            DroneChassis.current_state = DroneState::FLYING;
            break;
    }
}
