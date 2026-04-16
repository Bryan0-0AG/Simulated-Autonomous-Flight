#ifndef WORLD_H
#define WORLD_H

#include "utils/vector2.h"

struct World {
    float groundY = 0.0f;
};

bool check_ground_collision(Vector2& position, Vector2& velocity, World& world);

#endif