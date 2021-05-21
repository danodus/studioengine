//
//  rect.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2013-09-09.
//  Copyright (c) 2013-2020 Daniel Cliche. All rights reserved.
//

#include "rect.h"

#include <math.h>

#include <algorithm>

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
Rect MDStudio::makeRect(float x, float y, float width, float height) {
    Rect rect;
    rect.origin.x = x;
    rect.origin.y = y;
    rect.size.width = width;
    rect.size.height = height;
    return rect;
}

// ---------------------------------------------------------------------------------------------------------------------
Rect MDStudio::makeZeroRect() {
    Rect rect;
    rect.origin.x = 0.0f;
    rect.origin.y = 0.0f;
    rect.size.width = 0.0f;
    rect.size.height = 0.0f;
    return rect;
}

// ---------------------------------------------------------------------------------------------------------------------
Rect MDStudio::makeIntersectRect(Rect r1, Rect r2) {
    Rect rect;
    rect.origin.x = r1.origin.x > r2.origin.x ? r1.origin.x : r2.origin.x;
    rect.origin.y = r1.origin.y > r2.origin.y ? r1.origin.y : r2.origin.y;
    float r1x = r1.origin.x + r1.size.width;
    float r1y = r1.origin.y + r1.size.height;
    float r2x = r2.origin.x + r2.size.width;
    float r2y = r2.origin.y + r2.size.height;
    float rx = r1x < r2x ? r1x : r2x;
    float ry = r1y < r2y ? r1y : r2y;
    rect.size.width = rx - rect.origin.x;
    rect.size.height = ry - rect.origin.y;

    if (rect.size.width < 0) rect.size.width = 0;

    if (rect.size.height < 0) rect.size.height = 0;

    return rect;
}

// ---------------------------------------------------------------------------------------------------------------------
Rect MDStudio::makeCenteredRectInRect(Rect r, float width, float height) {
    float cx = r.origin.x + r.size.width / 2.0f;
    float cy = r.origin.y + r.size.height / 2.0f;
    return makeRect(floorf(cx - width / 2.0f), floorf(cy - height / 2.0f), width, height);
}

// ---------------------------------------------------------------------------------------------------------------------
Rect MDStudio::makeCenteredRectInRectLimited(Rect r, float width, float height) {
    float cx = r.origin.x + r.size.width / 2.0f;
    float cy = r.origin.y + r.size.height / 2.0f;

    if (width > r.size.width) {
        width = r.size.width;
    }

    if (height > r.size.height) {
        height = r.size.height;
    }

    return makeRect(floorf(cx - width / 2.0f), floorf(cy - height / 2.0f), width, height);
}

// ---------------------------------------------------------------------------------------------------------------------
Rect MDStudio::makeCenteredLeftRectInRect(Rect r, float width, float height, float leftMargin) {
    float cy = r.origin.y + r.size.height / 2.0f;
    return makeRect(floorf(r.origin.x + leftMargin), floorf(cy - height / 2.0f), width, height);
}

// ---------------------------------------------------------------------------------------------------------------------
Rect MDStudio::makeCenteredRightRectInRect(Rect r, float width, float height, float rightMargin) {
    float cy = r.origin.y + r.size.height / 2.0f;
    return makeRect(floorf(r.size.width - width - rightMargin), floorf(cy - height / 2.0f), width, height);
}

// ---------------------------------------------------------------------------------------------------------------------
Rect MDStudio::makeInsetRect(Rect r, float hMargin, float vMargin) {
    return makeRect(r.origin.x + hMargin, r.origin.y + vMargin, r.size.width - hMargin * 2.0f,
                    r.size.height - vMargin * 2.0f);
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::normalizeRect(Rect* r) {
    if (r->size.width < 0) {
        r->size.width = -r->size.width;
        r->origin.x -= r->size.width;
    }
    if (r->size.height < 0) {
        r->size.height = -r->size.height;
        r->origin.y -= r->size.height;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
float MDStudio::midRectX(Rect r) { return r.origin.x + r.size.width / 2.0f; }

// ---------------------------------------------------------------------------------------------------------------------
float MDStudio::midRectY(Rect r) { return r.origin.y + r.size.height / 2.0f; }

// ---------------------------------------------------------------------------------------------------------------------
Point MDStudio::midRect(Rect r) { return makePoint(midRectX(r), midRectY(r)); }

// ---------------------------------------------------------------------------------------------------------------------
Rect MDStudio::makeUnionRect(Rect r1, Rect r2) {
    float blx1 = r1.origin.x;
    float bly1 = r1.origin.y;
    float trx1 = r1.origin.x + r1.size.width;
    float try1 = r1.origin.y + r1.size.height;

    float blx2 = r2.origin.x;
    float bly2 = r2.origin.y;
    float trx2 = r2.origin.x + r2.size.width;
    float try2 = r2.origin.y + r2.size.height;

    float rblx = std::min(blx1, blx2);
    float rbly = std::min(bly1, bly2);
    float rtrx = std::max(trx1, trx2);
    float rtry = std::max(try1, try2);

    return makeRect(rblx, rbly, rtrx - rblx, rtry - rbly);
}
