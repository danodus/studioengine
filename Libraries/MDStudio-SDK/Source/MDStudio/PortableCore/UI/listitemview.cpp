//
//  listitemview.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-07-12.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#include "listitemview.h"

#include "draw.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
ListItemView::ListItemView(const std::string& name, void* owner, const std::string& title)
    : _title(title), View(name, owner) {
    _font = SystemFonts::sharedInstance()->semiboldFont();
    _textColor = whiteColor;
    _isHovering = false;
    _isHighlighted = false;
    _hasFocus = false;
}

// ---------------------------------------------------------------------------------------------------------------------
void ListItemView::draw() {
    DrawContext* dc = drawContext();

    dc->pushStates();
    dc->setFillColor(_isHighlighted ? (_hasFocus ? blueColor : (_isHovering ? dimGrayColor : veryDimGrayColor))
                                    : (_isHovering ? dimGrayColor : blackColor));
    dc->drawRect(bounds());
    dc->popStates();

    dc->pushStates();
    dc->setStrokeColor(_textColor);
    dc->drawLeftText(_font, bounds(), _title);
    dc->popStates();
}

// ---------------------------------------------------------------------------------------------------------------------
void ListItemView::setIsHighlighted(bool isHighlighted) {
    if (isHighlighted != _isHighlighted) {
        _isHighlighted = isHighlighted;
        setDirty();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ListItemView::setIsHovering(bool isHovering) {
    if (isHovering != _isHovering) {
        _isHovering = isHovering;
        setDirty();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ListItemView::setFocusState(bool focusState) {
    if (_hasFocus != focusState) {
        _hasFocus = focusState;
        setDirty();
    }
}
