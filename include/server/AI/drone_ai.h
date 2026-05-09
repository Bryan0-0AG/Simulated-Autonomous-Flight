#ifndef DRONE_AI_H
#define DRONE_AI_H

#include "drone_dynamics.h"
#include "world/world.h"
#include "world/procedural_city.h"

void update_drone_ai(DroneChassis& drone, const world& vWorld, const ProceduralCity& city);

#endif
