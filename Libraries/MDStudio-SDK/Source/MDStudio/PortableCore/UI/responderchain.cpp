//
//  responderchain.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-05-23.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#include "responderchain.h"

#include <assert.h>

#include "draw.h"

#define RESPONDER_CHAIN_DEBUG 0
#define RESPONDER_CHAIN_DEBUG_EVENTS 0

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
ResponderChain::ResponderChain() {
    _cursorRect = makeZeroRect();
    _cursorResponder = nullptr;
    _lastMousePos = makeZeroPoint();
    _firstResponder = nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------
void ResponderChain::addResponder(Responder* responder) {
    _responders.push_back(responder);
    auto it = _responders.end();
    --it;
    responder->addToChain(it);
}

// ---------------------------------------------------------------------------------------------------------------------
void ResponderChain::removeResponder(Responder* responder) {
    if (!responder->isInChain()) return;

    if (_firstResponder == responder) makeFirstResponder(nullptr);

    _responders.erase(responder->it());
    responder->removeFromChain();

    std::deque<Responder*>::iterator it2;
    for (it2 = _capturedResponders.begin(); it2 < _capturedResponders.end(); it2++) {
        if (*it2 == responder) {
            _capturedResponders.erase(it2);
            break;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ResponderChain::removeAllResponders(bool isFirstResponderKept) {
    if (!isFirstResponderKept) makeFirstResponder(nullptr);

    for (auto responder : _responders) responder->removeFromChain();

    _capturedResponders.clear();
    _responders.clear();
}

// ---------------------------------------------------------------------------------------------------------------------
void ResponderChain::releaseCursor() {
    _cursorResponder = nullptr;
    Platform::sharedInstance()->setCursor(this, Platform::ArrowCursor);
    _cursorRect = makeZeroRect();
}

// ---------------------------------------------------------------------------------------------------------------------
bool ResponderChain::sendEvent(const UIEvent* event) {
    if (_willSendEventFn) _willSendEventFn(this, event);

    _lastEvent = *event;

    if (isMouseEvent(event)) _lastMousePos = event->pt;

    // Make sure that we go back to arrow cursor if outside the region
    if (isMouseEvent(event)) {
        Responder* capturedResponder = _capturedResponders.empty() ? nullptr : _capturedResponders.back();
        if (_cursorResponder != capturedResponder) {
            if (!isPointInRect(event->pt, _cursorRect)) {
                releaseCursor();
            }
        }
    }

    bool isProcessed = false;

    Responder* capturedResponder = _capturedResponders.empty() ? nullptr : _capturedResponders.back();
    if (capturedResponder) {
        try {
            isProcessed = capturedResponder->handleEvent(event);
        } catch (const std::runtime_error& e) {
            Platform::sharedInstance()->throwException(e);
            isProcessed = true;
        }
    }

    if (!isProcessed) {
        std::list<Responder*>::reverse_iterator rit = _responders.rbegin();
        for (rit = _responders.rbegin(); rit != _responders.rend(); ++rit) {
            Responder* responder = *rit;
            if (responder == capturedResponder) continue;
            try {
                if (responder->handleEvent(event)) {
                    isProcessed = true;
#if RESPONDER_CHAIN_DEBUG_EVENTS
                    std::cout << "Event processed by " << responder->name() << "\n";
#endif
                    break;
                }
            } catch (const std::runtime_error& e) {
                Platform::sharedInstance()->throwException(e);
                isProcessed = true;
                break;
            }
        }
    }

    // If the event was a key event and it was not processed, do a beep
    if (event->type == KEY_UIEVENT && !isProcessed && event->key != KEY_CONTROL) Platform::sharedInstance()->beep();

    return isProcessed;
}

// ---------------------------------------------------------------------------------------------------------------------
bool ResponderChain::captureResponder(Responder* responder) {
    assert(responder);

    bool isFound = false;
    for (auto r : _responders)
        if (r == responder) {
            isFound = true;
            break;
        }

    assert(isFound);

    _capturedResponders.push_back(responder);
#if RESPONDER_CHAIN_DEBUG
    std::cout << "Responder " << responder->name() << " captured\n";
#endif
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
void ResponderChain::releaseResponder(Responder* responder) {
#if RESPONDER_CHAIN_DEBUG
    assert(_capturedResponders.size() > 0 && responder == _capturedResponders.back());
#endif

    if (_capturedResponders.size() > 0) {
#if RESPONDER_CHAIN_DEBUG
        std::cout << "Responder " << _capturedResponders.back()->name() << " released, "
                  << "remaining " << _capturedResponders.size() - 1 << "\n";
#endif
        _capturedResponders.pop_back();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ResponderChain::makeFirstResponder(Responder* responder) {
    if (_firstResponder) {
        _firstResponder->resignFirstResponder();
#if RESPONDER_CHAIN_DEBUG
        std::cout << "Responder " << _firstResponder->name() << " resigned first responder\n";
#endif
    }
    _firstResponder = responder;
#if RESPONDER_CHAIN_DEBUG
    if (responder) std::cout << "Responder " << responder->name() << " became first responder\n";
#endif
}

// ---------------------------------------------------------------------------------------------------------------------
void ResponderChain::setCursorInRect(Responder* responder, Platform::CursorEnumType cursor, Rect rect) {
    _cursorRect = rect;
    _cursorResponder = responder;
    Platform::sharedInstance()->setCursor(this, cursor);
}

// ---------------------------------------------------------------------------------------------------------------------
void ResponderChain::selectAll() {
    if (_firstResponder) _firstResponder->selectAll();
}
