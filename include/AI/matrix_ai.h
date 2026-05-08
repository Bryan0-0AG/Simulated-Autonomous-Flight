#ifndef MATRIX_AI_H
#define MATRIX_AI_H

#include "swarm/spawn_logic/matrix_group.h"
#include "world/world.h"
#include "swarm/swarm_dynamics.h" // For DroneChassis

void update_matrix_ai(MatrixGroup& matrix, const world& vWorld, std::vector<DroneChassis>& drones);

#endif
