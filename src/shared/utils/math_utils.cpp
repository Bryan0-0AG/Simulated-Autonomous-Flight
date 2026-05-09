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

int findClosestExact(int n, int target) {
    // Finds a value that is either a divisor or a multiple of n
    // which is closest to the target. This ensures perfect alignment.
    int closest = n;
    int min_diff = std::abs(target - n);

    // 1. Check divisors of n
    for (int d = 1; d <= n; ++d) {
        if (n % d == 0) {
            int diff = std::abs(target - d);
            if (diff < min_diff) {
                min_diff = diff;
                closest = d;
            }
        }
    }

    // 2. Check multiples of n
    // We check up to target + n to find the closest possible multiple
    for (int m = n; m <= target + n; m += n) {
        int diff = std::abs(target - m);
        if (diff < min_diff) {
            min_diff = diff;
            closest = m;
        }
        // In case of tie (e.g., target 10, batch 4, multiples 8 and 12),
        // we keep the first one found (the smaller one) for stability.
    }

    return closest;
}
