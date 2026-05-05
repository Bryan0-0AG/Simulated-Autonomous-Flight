#ifndef DECISIONS_H
#define DECISIONS_H

// Necesitamos conocer la estructura DroneChassis para poder modificarla
#include "HPC/swarm_dynamics.h"
#include "world/world.h"

// FunciÃ³n principal que evalÃºa el entorno y actualiza el estado de la IA
void update_ai_decisions(DroneChassis& DroneChassis, const world& virtualWorld);

#endif
