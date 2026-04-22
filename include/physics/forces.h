#ifndef FORCES_H
#define FORCES_H

#include <vector>
#include "utils/vector2.h"
#include "global_config.h"
#include "body.h"

// Declaraciones
Vector2 compute_gravity(const Body& body);
Vector2 compute_drag(const Body& body);
Vector2 compute_thrust(const ActuatorOutput& output);
Vector2 compute_separation(const Body& body, const std::vector<Body*>& neighbors);

#endif