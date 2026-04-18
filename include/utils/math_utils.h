#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include "vector2.h"

float fast_pow(float base, float exp);
void snap_zero(float& value, float epsilon = 1e-4f);
int randint(int min, int max);
float distance(Vector2 p1, Vector2 p2);
float magnitude(Vector2 v);

#endif