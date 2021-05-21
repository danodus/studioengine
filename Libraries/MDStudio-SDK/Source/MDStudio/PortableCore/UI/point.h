//
//  point.h
//  MDStudio
//
//  Created by Daniel Cliche on 2013-09-09.
//  Copyright (c) 2013-2020 Daniel Cliche. All rights reserved.
//

#ifndef POINT_H
#define POINT_H

namespace MDStudio {

struct Point {
    float x, y;
};

Point makePoint(float x, float y);
Point makeZeroPoint();

}  // namespace MDStudio

#endif  // POINT_H
