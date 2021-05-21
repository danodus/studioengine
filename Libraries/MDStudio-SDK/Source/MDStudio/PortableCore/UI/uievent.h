//
//  uievent.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-05-23.
//  Copyright (c) 2014-2018 Daniel Cliche. All rights reserved.
//

#ifndef UIEVENT_H
#define UIEVENT_H

#include <iostream>
#include <string>

#include "point.h"

#define KEY_CONTROL 59

#define KEY_DELETE 117
#define KEY_UP 126
#define KEY_DOWN 125
#define KEY_LEFT 123
#define KEY_RIGHT 124

#define KEY_BACKSPACE 51
#define KEY_ESCAPE 53

#define KEY_ENTER 36
#define KEY_TAB 48

#define KEY_HOME 115
#define KEY_END 119
#define KEY_PAGE_UP 116
#define KEY_PAGE_DOWN 121

#define MODIFIER_FLAG_SHIFT 0x1
#define MODIFIER_FLAG_CONTROL 0x2
#define MODIFIER_FLAG_ALTERNATE 0x4
#define MODIFIER_FLAG_COMMAND 0x8

namespace MDStudio {

typedef enum {
    MOUSE_DOWN_UIEVENT,
    RIGHT_MOUSE_DOWN_UIEVENT,
    MOUSE_UP_UIEVENT,
    RIGHT_MOUSE_UP_UIEVENT,
    MOUSE_MOVED_UIEVENT,
    KEY_UIEVENT,
    KEY_UP_UIEVENT,
    SCROLL_UIEVENT,
    CUT_UIEVENT,
    COPY_UIEVENT,
    PASTE_UIEVENT
} UIEventType;
typedef enum { PHASE_NONE_UIEVENT, PHASE_BEGAN_UIEVENT, PHASE_CHANGED_UIEVENT, PHASE_ENDED_UIEVENT } UIEventPhase;

struct UIEvent {
    UIEventType type;
    Point pt;
    unsigned int key;
    std::string characters;
    bool isARepeat;
    float deltaX, deltaY, deltaZ;
    UIEventPhase phase;
    unsigned int modifierFlags;
};

bool isMouseEvent(const UIEvent* event);
bool isKeyEvent(const UIEvent* event);
}  // namespace MDStudio

#endif  // uievent_h
