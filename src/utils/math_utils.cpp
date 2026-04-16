#include "utils/math_utils.h"
#include <cmath>
#include <iostream>

float fast_pow(float base, float exp) {
    float result = 1.0f;
    for (int i = 0; i < exp; i++) {
        result *= base;
    }
    return result;
}

void snap_zero(float& value, float epsilon) {
    if (std::abs(value) < epsilon) {
        value = 0.0f;
    }
}