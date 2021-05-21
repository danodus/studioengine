//
//  responderchain.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-05-23.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#ifndef RUNLOOP_H
#define RUNLOOP_H

#include <deque>
#include <list>
#include <vector>

#include "../platform.h"
#include "rect.h"
#include "responder.h"
#include "uievent.h"

namespace MDStudio {
class ResponderChain {
   public:
    typedef std::function<void(ResponderChain* sender, const UIEvent* event)> WillSendEventFnType;

   private:
    std::list<Responder*> _responders;
    std::deque<Responder*> _capturedResponders;

    UIEvent _lastEvent;
    Point _lastMousePos;

    Rect _cursorRect;
    Responder* _cursorResponder;
    Responder* _firstResponder;

    void updateCursor();

    WillSendEventFnType _willSendEventFn = nullptr;

   public:
    ResponderChain();

    void addResponder(Responder* responder);
    void removeResponder(Responder* responder);
    void removeAllResponders(bool isFirstResponderKept = false);
    bool sendEvent(const UIEvent* event);
    bool captureResponder(Responder* responder);
    std::deque<Responder*> capturedResponders() { return _capturedResponders; }
    void setCapturedResponders(const std::deque<Responder*>& capturedResponders) {
        _capturedResponders = capturedResponders;
    }
    void releaseResponder(Responder* responder);

    // First responder
    void makeFirstResponder(Responder* responder);

    // First responder edition actions
    void selectAll();

    void setCursorInRect(Responder* responser, Platform::CursorEnumType cursor, Rect rect);
    void releaseCursor();

    UIEvent lastEvent() { return _lastEvent; }
    Point lastMousePos() { return _lastMousePos; }

    void setWillSendEventFn(WillSendEventFnType willSendEventFn) { _willSendEventFn = willSendEventFn; }
};
}  // namespace MDStudio

#endif  // RUNLOOP_H
