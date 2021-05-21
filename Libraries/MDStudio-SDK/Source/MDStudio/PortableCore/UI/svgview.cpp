//
//  svgview.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2016-08-23.
//  Copyright (c) 2016-2020 Daniel Cliche. All rights reserved.
//

#include "svgview.h"

#include "draw.h"

#define BUFFSIZE 8192

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
SVGView::SVGView(const std::string& name, void* owner, SVG* svg) : View(name, owner), _svg(svg) {}

// ---------------------------------------------------------------------------------------------------------------------
void SVGView::draw() {
    DrawContext* dc = drawContext();

    dc->pushStates();
    dc->setFillColor(whiteColor);
    dc->drawRect(bounds());
    dc->popStates();

    _svg->draw(dc, rect().size.width / _svg->size().width, -rect().size.height / _svg->size().height,
               makePoint(0.0f, rect().size.height));
}

// ---------------------------------------------------------------------------------------------------------------------
void SVGView::setFrame(MDStudio::Rect frame) { View::setFrame(frame); }
