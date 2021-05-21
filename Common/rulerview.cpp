//
//  rulerview.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-12-15.
//  Copyright © 2015-2019 Daniel Cliche. All rights reserved.
//

#include "rulerview.h"

#include "helpers.h"

#include <draw.h>

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
RulerView::RulerView(std::string name, void *owner, float margin, int min, unsigned int nbDivisions, unsigned int divStep) : _margin(margin), _min(min), _nbDivisions(nbDivisions), _divStep(divStep), View(name, owner)
{
    _hasFocus = false;
}

// ---------------------------------------------------------------------------------------------------------------------
void RulerView::draw()
{
    DrawContext *dc = drawContext();
    
    Rect r = bounds();
    dc->pushStates();
    dc->setFillColor(_hasFocus ? dimGrayColor : veryDimGrayColor);
    dc->setStrokeColor(lightGrayColor);
    dc->drawRect(r);
    dc->popStates();
    
    float divHeight = (r.size.height - 2.0f * _margin) / static_cast<float>(_nbDivisions - 1);
    dc->pushStates();
    dc->setStrokeColor(_hasFocus ? whiteColor : grayColor);
    for (int div = 0; div < _nbDivisions; ++div) {
        float y = r.origin.y + static_cast<float>(div) * divHeight + _margin;
        dc->drawLine(makePoint(r.size.width - 3.0f, y), makePoint(r.size.width, y));
        std::string s = std::to_string(_min + div * static_cast<int>(_divStep));
        Rect textRect = makeRect(0.0f, y - divHeight / 2.0f, r.size.width - 5.0f, divHeight);
        textRect.origin.y++;
        dc->drawRightText(SystemFonts::sharedInstance()->semiboldFontSmall(), textRect, s);
    }
    dc->popStates();
}

// ---------------------------------------------------------------------------------------------------------------------
void RulerView::setRange(unsigned int min, unsigned int nbDivisions, unsigned int divStep)
{
    _min = min;
    _nbDivisions = nbDivisions;
    _divStep = divStep;
    setDirty();
}
