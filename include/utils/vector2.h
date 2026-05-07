#ifndef VECTOR2_H
#define VECTOR2_H

// If we are compiling for GPU, we activate AMD markers
#ifdef __HIPCC__
    #define HD __host__ __device__
#else
    #define HD
#endif

struct Vector2 {
    float x, y;

    // Operators marked with HD to work on both CPU and GPU
    HD Vector2 operator+(const Vector2& other) const { return {x + other.x, y + other.y}; }
    HD Vector2 operator-(const Vector2& other) const { return {x - other.x, y - other.y}; }
    HD Vector2 operator*(float scalar) const { return {x * scalar, y * scalar}; }
    HD Vector2 operator/(float scalar) const { return {x / scalar, y / scalar}; }

    HD Vector2& operator+=(const Vector2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }
};

#endif