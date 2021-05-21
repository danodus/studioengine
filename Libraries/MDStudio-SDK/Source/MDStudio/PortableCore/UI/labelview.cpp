//
//  labelview.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-07-15.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#include "labelview.h"

#include "../utf8.h"
#include "draw.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
LabelView::LabelView(const std::string& name, void* owner, std::string title) : View(name, owner) {
    _font = SystemFonts::sharedInstance()->semiboldFont();
    _borderColor = zeroColor;
    _fillColor = zeroColor;
    _textColor = whiteColor;
    _textAlign = LeftTextAlign;
    setTitle(title);
}

// ---------------------------------------------------------------------------------------------------------------------
void LabelView::draw() {
    DrawContext* dc = drawContext();

    // Draw background

    dc->pushStates();
    dc->setFillColor(_fillColor);
    dc->drawRect(bounds());
    dc->popStates();

    // Draw text

    dc->pushStates();
    dc->setStrokeColor(_textColor);

    if (_lines.size() == 1) {
        if (_textAlign == CenterTextAlign) {
            dc->drawCenteredText(_font, bounds(), _lines[0]);
        } else if (_textAlign == RightTextAlign) {
            dc->drawRightText(_font, bounds(), _lines[0]);
        } else {
            dc->drawLeftText(_font, bounds(), _lines[0]);
        }
    } else {
        float height = fontHeight(_font);
        Point pt = makePoint(0.0f, bounds().size.height - height);
        for (auto line : _lines) {
            if (isRectInRect(makeRect(pt.x + offset().x, pt.y + offset().y, bounds().size.width, height),
                             clippedBounds()))
                dc->drawText(_font, pt, line);
            pt.y -= height;
        }
    }
    dc->popStates();

    // Draw border

    dc->pushStates();
    dc->setStrokeColor(_borderColor);
    dc->drawRect(bounds());
    dc->popStates();
}

// ---------------------------------------------------------------------------------------------------------------------
void LabelView::setTitle(std::string title) {
    _lines.clear();

    std::string line;
    for (auto c : title) {
        if (c == '\n') {
            _lines.push_back(line);
            line.clear();
        } else {
            line += c;
        }
    }
    _lines.push_back(line);

    _title = title;
    setDirty();
}

// ---------------------------------------------------------------------------------------------------------------------
Size LabelView::contentSize() {
    float height = fontHeight(_font);
    float maxWidth = 0.0f;
    for (auto line : _lines) {
        float width = getTextWidth(_font, line);
        if (width > maxWidth) maxWidth = width;
    }

    return makeSize(maxWidth, _lines.size() * height);
}
