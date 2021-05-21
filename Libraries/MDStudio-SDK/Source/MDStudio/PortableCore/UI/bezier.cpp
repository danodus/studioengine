//
//  bezier.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2017-08-20.
//  Copyright © 2017-2020 Daniel Cliche. All rights reserved.
//

// Ref.: http://devmag.org.za/2011/04/05/bzier-curves-a-tutorial/

#include "bezier.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
Vector2 MDStudio::calculateQuadraticBezierPoint(float t, Vector2 p0, Vector2 p1, Vector2 p2) {
    float u = 1 - t;
    float tt = t * t;
    float uu = u * u;

    Vector2 p = uu * p0;  // first term
    p += 2 * u * t * p1;  // second term
    p += tt * p2;

    return p;
}

// ---------------------------------------------------------------------------------------------------------------------
Vector2 MDStudio::calculateCubicBezierPoint(float t, Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3) {
    float u = 1 - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    Vector2 p = uuu * p0;  // first term
    p += 3 * uu * t * p1;  // second term
    p += 3 * u * tt * p2;  // third term
    p += ttt * p3;         // fourth term

    return p;
}
