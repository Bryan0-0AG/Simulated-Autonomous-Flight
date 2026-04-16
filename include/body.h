#ifndef BODY_H
#define BODY_H

#include "utils/vector2.h"

struct Body {
    Vector2 position = {0.0f, 0.0f};
    Vector2 velocity = {0.0f, 0.0f};
    Vector2 force    = {0.0f, 0.0f};
    float mass = 1.0f;
};

#endif