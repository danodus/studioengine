//
//  svg.h
//  MDStudio
//
//  Created by Daniel Cliche on 2018-01-15.
//  Copyright © 2016-2020 Daniel Cliche. All rights reserved.
//

#ifndef SVG_H
#define SVG_H

#include <string>

#include "drawcontext.h"

namespace MDStudio {

class SVG {
    const std::string _s;

    DrawContext* _drawContext;
    Size _size;

    bool parse();

   public:
    SVG(const std::string& s);
    ~SVG();

    enum ParserStates {
        RootState,
        SVGState,
        SVGRectState,
        SVGCircleState,
        SVGEllipseState,
        SVGLineState,
        SVGPolygonState,
        SVGPolylineState,
        SVGPathState
    };
    ParserStates _parserState;

    bool draw(DrawContext* drawContext, float scaleX = 1.0f, float scaleY = 1.0f, Point translation = {0.0f, 0.0f});

    DrawContext* drawContext() { return _drawContext; }

    Size size() { return _size; }
    void setSize(Size size) { _size = size; }
};

}  // namespace MDStudio

#endif  // SVG_H
