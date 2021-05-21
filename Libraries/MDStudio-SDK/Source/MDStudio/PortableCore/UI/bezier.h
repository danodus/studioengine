//
//  bezier.h
//  MDStudio
//
//  Created by Daniel Cliche on 2017-08-20.
//  Copyright © 2017-2020 Daniel Cliche. All rights reserved.
//

#ifndef BEZIER_H
#define BEZIER_H

#include "vector2.h"

namespace MDStudio {

Vector2 calculateQuadraticBezierPoint(float t, Vector2 p0, Vector2 p1, Vector2 p2);
Vector2 calculateCubicBezierPoint(float t, Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3);

}  // namespace MDStudio

#endif  // BEZIER_H
