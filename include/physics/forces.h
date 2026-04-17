#ifndef FORCES_H
#define FORCES_H

#include "utils/vector2.h"
#include "vehicle/control_config.h"
#include "body.h"

// Declaraciones
Vector2 compute_gravity(const Body& body);
Vector2 compute_drag(const Body& body);
Vector2 compute_thrust(const ActuatorOutput& output);

#endif