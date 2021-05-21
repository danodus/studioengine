//
//  progressindicator.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2015-08-29.
//  Copyright (c) 2015-2020 Daniel Cliche. All rights reserved.
//

#include "progressindicator.h"

#include "draw.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
ProgressIndicator::ProgressIndicator(const std::string& name, void* owner, float max) : View(name, owner), _max(max) {
    _pos = 0;
}

// ---------------------------------------------------------------------------------------------------------------------
void ProgressIndicator::draw() {
    DrawContext* dc = drawContext();

    dc->pushStates();
    dc->setFillColor(veryDimGrayColor);
    dc->setStrokeColor(lightGrayColor);
    dc->drawRect(bounds());
    dc->popStates();

    // If horizontal scroll bar
    Rect progressRect;
    float f = frame().size.width / _max;
    progressRect = makeRect(0.0f, 0.0f, f * _pos, frame().size.height);
    progressRect = makeInsetRect(progressRect, 2.0f, 2.0f);
    if (progressRect.size.width < 0.0f) progressRect.size.width = 0.0f;

    dc->pushStates();
    dc->setFillColor(lightGrayColor);
    dc->drawRect(progressRect);
    dc->popStates();
}

// ---------------------------------------------------------------------------------------------------------------------
void ProgressIndicator::setPos(float pos) {
    _pos = pos;
    setDirty();
}
