#ifndef VECTOR2_H
#define VECTOR2_H

struct Vector2 {
    float x, y;

    // Operadores para simplificar cálculos de física y separación
    Vector2 operator+(const Vector2& other) const { return {x + other.x, y + other.y}; }
    Vector2 operator-(const Vector2& other) const { return {x - other.x, y - other.y}; }
    Vector2 operator*(float scalar) const { return {x * scalar, y * scalar}; }
    Vector2 operator/(float scalar) const { return {x / scalar, y / scalar}; }

    Vector2& operator+=(const Vector2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }
};

#endif