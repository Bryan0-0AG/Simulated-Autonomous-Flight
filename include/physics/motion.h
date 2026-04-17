#ifndef MOTION_H
#define MOTION_H

#include "body.h"

void apply_impulse(Body& body, Vector2 impulse);
void apply_forces(Body& body, Vector2 thrust);
void update_motion(Body& body, float dt);

#endif