//
//  point.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2013-09-09.
//  Copyright (c) 2013-2020 Daniel Cliche. All rights reserved.
//

#include "point.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
Point MDStudio::makePoint(float x, float y) {
    Point point;
    point.x = x;
    point.y = y;
    return point;
}

// ---------------------------------------------------------------------------------------------------------------------
Point MDStudio::makeZeroPoint() { return {0.0f, 0.0f}; }