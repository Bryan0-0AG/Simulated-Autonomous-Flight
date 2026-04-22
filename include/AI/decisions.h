#ifndef DECISIONS_H
#define DECISIONS_H

// Necesitamos conocer la estructura Body para poder modificarla
#include "body.h"
#include "world/world.h"

// Función principal que evalúa el entorno y actualiza el estado de la IA
void update_ai_decisions(Body& body, const world& virtualWorld);

#endif
