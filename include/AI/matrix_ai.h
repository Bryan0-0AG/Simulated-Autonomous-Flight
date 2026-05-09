#ifndef MATRIX_AI_H
#define MATRIX_AI_H

#include "swarm/spawn_logic/matrix_group.h"
#include "swarm/swarm_dynamics.h"  // DroneChassis, GPUMatrix
#include <vector>

// CPU adapter: packs matrix data into GPU-friendly structs,
// dispatches to the matrix_ai.hip kernel, and applies results back.
void update_matrix_ai(std::vector<MatrixGroup>& all_matrices,
                      std::vector<DroneChassis>& drones,
                      float dt);

#endif
