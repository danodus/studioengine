//
//  uievent.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-05-23.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#include "uievent.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
bool MDStudio::isMouseEvent(const UIEvent* event) {
    return ((event->type == MOUSE_DOWN_UIEVENT) || (event->type == MOUSE_UP_UIEVENT) ||
            (event->type == RIGHT_MOUSE_DOWN_UIEVENT) || (event->type == RIGHT_MOUSE_UP_UIEVENT) ||
            (event->type == MOUSE_MOVED_UIEVENT) || (event->type == SCROLL_UIEVENT));
}

// ---------------------------------------------------------------------------------------------------------------------
bool MDStudio::isKeyEvent(const UIEvent* event) {
    return ((event->type == KEY_UIEVENT) || (event->type == KEY_UP_UIEVENT));
}
