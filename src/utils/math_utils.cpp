#include "utils/math_utils.h"

float pow(float base, float exp) {
    float result = 1.0f;
    for (int i = 0; i < exp; i++) {
        result *= base;
    }
    return result;
}