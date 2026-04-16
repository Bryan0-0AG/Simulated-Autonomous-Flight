#ifndef WORLD_H
#define WORLD_H

#include "utils/vector2.h"
#include "body.h"

struct World {
    float groundY = 0.0f;
};

bool check_ground_collision(Body& body, World& world);

#endif