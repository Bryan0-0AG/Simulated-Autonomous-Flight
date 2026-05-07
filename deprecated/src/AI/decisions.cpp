#include "AI/decisions.h"
#include "utils/math_utils.h"
#include "global_config.h"
#include <cmath>

#include <cmath>
#include <iostream>

void update_ai_decisions(DroneChassis& drone, const world& virtualWorld, const ProceduralCity& city) {
    DroneState state = toState(drone.current_state);

    // 1. RECHARGE LOGIC
    if (state == DroneState::LANDED) {
        drone.battery += BATTERY_CHARGE_SPEED;
        if (drone.battery >= drone.max_battery) drone.battery = drone.max_battery;
        
        // Wake up if the matrix moves us to a high target and we have energy
        if (drone.target.y > 10.0f && drone.battery > 20.0f) {
            drone.current_state = toInt(DroneState::FLYING);
        }
        return;
    }

    // 2. PHYSICAL & SAFETY CHECKS
    // Ground collision check
    if (drone.position.y <= 3.0f) {
        drone.current_state = toInt(DroneState::LANDED);
      // Individual drones no longer decide to RTB or Transit.
    // They just follow the target given by the Matrix AI.
}

bool has_reached_target(const DroneChassis& drone, Vector2 target, float tolerance) {
    float dx = drone.position.x - target.x;
    float dy = drone.position.y - target.y;
    return (std::sqrt(dx*dx + dy*dy) <= tolerance);
}
