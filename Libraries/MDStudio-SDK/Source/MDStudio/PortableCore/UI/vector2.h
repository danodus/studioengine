//
//  vector2.h
//  MDStudio
//
//  Created by Daniel Cliche on 2016-08-24.
//  Copyright (c) 2016-2020 Daniel Cliche. All rights reserved.
//

#ifndef VECTOR2_H
#define VECTOR2_H

#include <point.h>

#include <cmath>

namespace MDStudio {

class Vector2 {
    float _x, _y;

   public:
    float x() const { return _x; }
    float y() const { return _y; }

    Vector2(float x, float y) : _x(x), _y(y) {}
    Vector2 operator+(const Vector2& v) { return Vector2(_x + v.x(), _y + v.y()); }
    Vector2 operator-(const Vector2& v) { return Vector2(_x - v.x(), _y - v.y()); }
    float operator*(const Vector2& v) { return _x * v.x() + _y * v.y(); }
    bool operator==(const Vector2& v) { return _x == v.x() && _y == v.y(); }
    void operator+=(const Vector2& v) {
        _x += v.x();
        _y += v.y();
    }
    float magnitude() { return sqrtf(_x * _x + _y * _y); }
    float dot(const Vector2& v) { return _x * v.x() + _y * v.y(); }

    Vector2 normalized() {
        auto m = magnitude();
        return Vector2(_x / m, _y / m);
    }
};

Vector2 operator+(float a, const Vector2& b);
Vector2 operator*(float a, const Vector2& b);
Vector2 operator/(float a, const Vector2& b);
Vector2 operator+(const Vector2& a, const Vector2& b);
Vector2 operator-(const Vector2& a, const Vector2& b);
bool operator==(const Vector2& a, const Vector2& b);

Point makePoint(const Vector2& v);

}  // namespace MDStudio

#endif  // VECTOR2_H
