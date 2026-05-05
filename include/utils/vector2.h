#ifndef VECTOR2_H
#define VECTOR2_H

// Si estamos compilando para GPU, activamos los marcadores de AMD
#ifdef __HIPCC__
    #define HD __host__ __device__
#else
    #define HD
#endif

struct Vector2 {
    float x, y;

    // Operadores marcados con HD para que funcionen en CPU y GPU
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