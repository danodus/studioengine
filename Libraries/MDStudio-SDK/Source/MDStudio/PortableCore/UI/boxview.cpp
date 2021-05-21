//
//  boxview.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-07-26.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#include "boxview.h"

#include "draw.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
BoxView::BoxView(const std::string& name, void* owner) : View(name, owner) {
    _bottomFillColor = blackColor;
    _topFillColor = blackColor;
    _borderColor = lightGrayColor;
    _cornerRadius = 0.0f;
}

// ---------------------------------------------------------------------------------------------------------------------
void BoxView::draw() {
    DrawContext* dc = drawContext();

    dc->pushStates();
    dc->setStrokeColor(_borderColor);
    if (_cornerRadius > 0) {
        dc->setFillColor(_bottomFillColor);
        dc->drawRoundRect(bounds(), _cornerRadius);
    } else {
        dc->drawRectGradientV(bounds(), _bottomFillColor, _topFillColor);
    }
    dc->popStates();
}

// ---------------------------------------------------------------------------------------------------------------------
void BoxView::setFillColors(Color bottomFillColor, Color topFillColor) {
    _bottomFillColor = bottomFillColor;
    _topFillColor = topFillColor;
    setDirty();
}

// ---------------------------------------------------------------------------------------------------------------------
void BoxView::setBorderColor(Color borderColor) {
    _borderColor = borderColor;
    setDirty();
}
