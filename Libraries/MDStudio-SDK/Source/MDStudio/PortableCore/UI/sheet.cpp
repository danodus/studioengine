//
//  sheet.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-07-26.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#include "sheet.h"

#include "draw.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
Sheet::Sheet(const std::string& name, void* owner, bool modal) : View(name, owner), _modal(modal) {
    _boxView = std::shared_ptr<BoxView>(new BoxView("boxView", this));
    _boxView->setCornerRadius(5.0f);
    addSubview(_boxView);
}

// ---------------------------------------------------------------------------------------------------------------------
Sheet::~Sheet() { removeSubview(_boxView); }

// ---------------------------------------------------------------------------------------------------------------------
bool Sheet::handleEvent(const UIEvent* event) {
    if (!_modal) return false;

    if ((event->type == KEY_UIEVENT) || isPointInRect(event->pt, rect())) {
        // Forward to our children
        return false;
    }
    // We absorb the event in order to be modal
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
void Sheet::setFrame(Rect rect) {
    View::setFrame(rect);
    _boxView->setFrame(bounds());
}
