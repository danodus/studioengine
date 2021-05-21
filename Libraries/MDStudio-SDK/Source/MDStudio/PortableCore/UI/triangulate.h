//
//  triangulate.h
//  MDStudio
//
//  Created by Daniel Cliche on 2016-09-01.
//  Copyright (c) 2016-2020 Daniel Cliche. All rights reserved.
//

// Ref.: http://flipcode.net/archives/Efficient_Polygon_Triangulation.shtml

#ifndef TRIANGULATE_H
#define TRIANGULATE_H

#include <vector>

#include "vector2.h"

namespace MDStudio {

typedef std::vector<Vector2> Vector2dVector;

class Triangulate {
   public:
    // triangulate a contour/polygon, places results in STL vector
    // as series of triangles.
    static bool Process(const Vector2dVector& contour, Vector2dVector& result);

    // compute area of a contour/polygon
    static float Area(const Vector2dVector& contour);

    // decide if point Px/Py is inside triangle defined by
    // (Ax,Ay) (Bx,By) (Cx,Cy)
    static bool InsideTriangle(float Ax, float Ay, float Bx, float By, float Cx, float Cy, float Px, float Py);

   private:
    static bool Snip(const Vector2dVector& contour, int u, int v, int w, int n, int* V);
};

}  // namespace MDStudio

#endif  // TRIANGULATE_H
