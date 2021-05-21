//
//  vector2.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2016-08-24.
//  Copyright © 2016-2020 Daniel Cliche. All rights reserved.
//

#include "vector2.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
Vector2 MDStudio::operator+(float a, const Vector2& b) { return Vector2(a + b.x(), a + b.y()); }

// ---------------------------------------------------------------------------------------------------------------------
Vector2 MDStudio::operator*(float a, const Vector2& b) { return Vector2(a * b.x(), a * b.y()); }

// ---------------------------------------------------------------------------------------------------------------------
Vector2 MDStudio::operator/(float a, const Vector2& b) { return Vector2(a / b.x(), a / b.y()); }

// ---------------------------------------------------------------------------------------------------------------------
Vector2 MDStudio::operator+(const Vector2& a, const Vector2& b) { return Vector2(a.x() + b.x(), a.y() + b.y()); }

// ---------------------------------------------------------------------------------------------------------------------
Vector2 MDStudio::operator-(const Vector2& a, const Vector2& b) { return Vector2(a.x() - b.x(), a.y() - b.y()); }

// ---------------------------------------------------------------------------------------------------------------------
bool MDStudio::operator==(const Vector2& a, const Vector2& b) { return a.x() == b.x() && a.y() == b.y(); }

// ---------------------------------------------------------------------------------------------------------------------
Point MDStudio::makePoint(const Vector2& v) { return makePoint(v.x(), v.y()); }
