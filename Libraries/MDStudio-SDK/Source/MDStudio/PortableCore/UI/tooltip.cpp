//
//  tooltip.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2018-09-07.
//  Copyright (c) 2018-2020 Daniel Cliche. All rights reserved.
//

#include "tooltip.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
Tooltip::Tooltip(const std::string& text, Point pt) : _text(text) {
    _window = new Window();
    _labelView = std::make_shared<LabelView>("tooltipLabelView", this, _text);
    auto size = _labelView->contentSize();
    size.width += 10.0f;
    size.height += 6.0f;
    _labelView->setTextAlign(LabelView::CenterTextAlign);
    _labelView->setTextColor(MDStudio::blackColor);
    _labelView->setBorderColor(MDStudio::blackColor);
    _labelView->setFillColor(MDStudio::wheatColor);
    _window->contentView()->addSubview(_labelView);
    _window->contentView()->setLayoutFn(
        [=](View* sender, Rect frame) { _labelView->setFrame(makeRect(0.0f, 0.0f, size.width, size.height)); });
    _window->open(makeRect(pt.x, pt.y, size.width, size.height), false);
}

// ---------------------------------------------------------------------------------------------------------------------
Tooltip::~Tooltip() {
    if (_window) delete _window;
}
