#ifndef MOTION_H
#define MOTION_H

#include "body.h"
#include "utils/vector2.h"

void apply_impulse(Body& body, Vector2 impulse);
void apply_forces(Body& body, Vector2 thrust, Vector2 separation);
void update_motion(Body& body, float dt);

#endif