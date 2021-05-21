//
//  tooltip.h
//  MDStudio
//
//  Created by Daniel Cliche on 2018-09-07.
//  Copyright (c) 2018-2020 Daniel Cliche. All rights reserved.
//

#ifndef TOOLTIP_H
#define TOOLTIP_H

#include "labelview.h"
#include "window.h"

namespace MDStudio {

class Tooltip {
    Window* _window;
    std::string _text;
    std::shared_ptr<LabelView> _labelView;

   public:
    Tooltip(const std::string& text, Point pt);
    ~Tooltip();
};

}  // namespace MDStudio

#endif  // TOOLTIP_H
