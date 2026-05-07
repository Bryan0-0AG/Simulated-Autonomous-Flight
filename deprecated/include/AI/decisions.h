#ifndef DECISIONS_H
#define DECISIONS_H

// We need to know the DroneChassis structure to be able to modify it
#include "swarm/swarm_dynamics.h"
#include "world/world.h"
#include "world/procedural_city.h"

// High-level mission orchestration and state logic
void update_ai_decisions(DroneChassis& drone, const world& virtualWorld, const ProceduralCity& city);

// Helper to check if a target has been reached
bool has_reached_target(const DroneChassis& drone, Vector2 target, float tolerance = 10.0f);

#endif
