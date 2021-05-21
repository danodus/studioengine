//
//  rect.h
//  MDStudio
//
//  Created by Daniel Cliche on 2013-09-09.
//  Copyright (c) 2013-2020 Daniel Cliche. All rights reserved.
//

#ifndef RECT_H
#define RECT_H

#include "point.h"
#include "size.h"

namespace MDStudio {

struct Rect {
    Point origin;
    Size size;
};

Rect makeRect(float x, float y, float width, float height);
Rect makeZeroRect();
Rect makeIntersectRect(Rect r1, Rect r2);
Rect makeCenteredRectInRect(Rect r, float width, float height);
Rect makeCenteredRectInRectLimited(Rect r, float width, float height);
Rect makeCenteredLeftRectInRect(Rect r, float width, float height, float leftMargin = 0.0f);
Rect makeCenteredRightRectInRect(Rect r, float width, float height, float rightMargin = 0.0f);
Rect makeInsetRect(Rect r, float hMargin, float vMargin);
Rect makeUnionRect(Rect r1, Rect r2);
void normalizeRect(Rect* r);

float midRectX(Rect r);
float midRectY(Rect r);
Point midRect(Rect r);

}  // namespace MDStudio

#endif  // RECT_H
