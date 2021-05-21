//
//  svgview.h
//  MDStudio
//
//  Created by Daniel Cliche on 2016-08-23.
//  Copyright (c) 2016-2020 Daniel Cliche. All rights reserved.
//

#ifndef SVGVIEW_H
#define SVGVIEW_H

#include "svg.h"
#include "view.h"

namespace MDStudio {

class DrawContext;

class SVGView : public MDStudio::View {
    SVG* _svg;

   public:
    SVGView(const std::string& name, void* owner, SVG* svg);

    void draw() override;

    void setFrame(MDStudio::Rect frame) override;

    Size svgSize() { return _svg->size(); }
};

}  // namespace MDStudio

#endif  // svgview_H
