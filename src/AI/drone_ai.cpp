#include "AI/drone_ai.h"
#include "AI/states.h"
#include "global_config.h"
#include <cmath>

void update_drone_ai(DroneChassis& drone, const world& vWorld, const ProceduralCity& city) {
    DroneState state = toState(drone.current_state);
    DroneAction action = toAction(drone.current_action);

    // 1. Survival Logic: Battery Check
    // Random extra drain during flight to create variance
    if (drone.battery > 0.0f && state == DroneState::FLYING) {
        float random_extra_drain = static_cast<float>(rand() % 100) / 10000.0f; // 0.0 to 0.01 extra per frame
        drone.battery -= random_extra_drain;
    }

    if (drone.battery < 20.0f && action == DroneAction::FOLLOW_MATRIX) {
        drone.current_action = toInt(DroneAction::GOING_TO_RECHARGE);
        // Landing spot (for now just a fixed spot or ground)
        drone.original_target = drone.target; 
        drone.target = { drone.position.x, 4.0f }; // Emergency landing right below
        return;
    }

    // 2. State Machine
    switch (action) {
        case DroneAction::GOING_TO_RECHARGE:
            if (drone.position.y <= 5.0f) {
                drone.current_action = toInt(DroneAction::RECHARGING);
                drone.current_state = toInt(DroneState::LANDED);
            }
            break;

        case DroneAction::RECHARGING:
            drone.battery += BATTERY_CHARGE_SPEED;
            if (drone.battery >= drone.max_battery) {
                drone.battery = drone.max_battery;
                drone.current_action = toInt(DroneAction::REJOINING_MATRIX);
                drone.current_state = toInt(DroneState::FLYING);
            }
            break;

        case DroneAction::REJOINING_MATRIX:
            // The SwarmManager will automatically update the target to the matrix slot
            // once we are in FLYING/FOLLOWING mode.
            // For now, if we are high enough, we go back to nominal.
            if (drone.position.y > 50.0f) {
                drone.current_action = toInt(DroneAction::FOLLOW_MATRIX);
            }
            break;

        default:
            // Ground collision safety
            if (drone.position.y <= 3.0f && state != DroneState::LANDED) {
                drone.current_state = toInt(DroneState::LANDED);
                drone.velocity = {0.0f, 0.0f};
            }
            break;
    }
}
