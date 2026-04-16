#ifndef MOTION_H
#define MOTION_H

#include "body.h"

void apply_impulse(Body& body, Vector2 impulse);
void apply_forces(Body& body, bool is_grounded);
void update_motion(Body& body, float dt);

#endif