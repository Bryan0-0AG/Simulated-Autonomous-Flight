#include "utils/math_utils.h"
#include <cmath>
#include <iostream>
#include <random>

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

int randint(int min, int max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen);
}

float distance(Vector2 p1, Vector2 p2) {
    return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
}

float magnitude(Vector2 v) {
    return std::sqrt(std::pow(v.x, 2) + std::pow(v.y, 2));
}