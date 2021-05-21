//
//  splitview.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2013-09-09.
//  Copyright (c) 2013-2020 Daniel Cliche. All rights reserved.
//

#include "splitviewh.h"

#include "../platform.h"
#include "draw.h"
#include "math.h"
#include "responderchain.h"

#define SPLITTER_WIDTH 6

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
SplitViewH::SplitViewH(const std::string& name, void* owner, std::shared_ptr<View> leftPane,
                       std::shared_ptr<View> rightPane, float splitPos, bool isPosRelativeToRight)
    : View(name, owner), _leftPane(leftPane), _rightPane(rightPane), _isPosRelativeToRight(isPosRelativeToRight) {
    addSubview(leftPane);
    addSubview(rightPane);

    setSplitPos(splitPos);

    _isCaptured = false;

    _posChangedFn = nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------
SplitViewH::~SplitViewH() {
    removeSubview(_leftPane);
    removeSubview(_rightPane);
}

// ---------------------------------------------------------------------------------------------------------------------
void SplitViewH::setSplitPos(float splitPos) {
    _splitPos = splitPos;

    if (_isPosRelativeToRight) splitPos = rect().size.width - splitPos;

    Rect r;

    r = makeRect(0.0f, 0.0f, splitPos - (SPLITTER_WIDTH / 2), rect().size.height);
    _leftPane->setFrame(r);

    r = makeRect(splitPos + (SPLITTER_WIDTH / 2), 0.0f, rect().size.width - splitPos - (SPLITTER_WIDTH / 2),
                 rect().size.height);
    _rightPane->setFrame(r);

    setDirty();

    if (_posChangedFn) _posChangedFn(this, _splitPos);
}

// ---------------------------------------------------------------------------------------------------------------------
void SplitViewH::draw() {
    DrawContext* dc = drawContext();

    float splitPos = _splitPos;
    if (_isPosRelativeToRight) splitPos = rect().size.width - splitPos;

    if (_leftPane->isVisible() && _rightPane->isVisible()) {
        dc->pushStates();
        dc->setFillColor(lightGrayColor);
        dc->setStrokeColor(veryDimGrayColor);
        Rect r = makeRect(splitPos - (SPLITTER_WIDTH / 2), -1.0f, SPLITTER_WIDTH, rect().size.height + 1.0f);
        dc->drawRect(r);
        dc->popStates();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool SplitViewH::handleEvent(const UIEvent* event) {
    float splitPos = _splitPos;
    if (_isPosRelativeToRight) splitPos = rect().size.width - splitPos;

    Rect splitterRect = makeZeroRect();
    if (_leftPane->isVisible() && _rightPane->isVisible())
        splitterRect = makeRect(rect().origin.x + splitPos - (SPLITTER_WIDTH / 2), rect().origin.y, SPLITTER_WIDTH,
                                rect().size.height);

    // Update the mouse cursor
    if (isMouseEvent(event)) {
        if (isPointInRect(event->pt, splitterRect)) {
            responderChain()->setCursorInRect(this, Platform::ResizeLeftRightCursor, splitterRect);
        }
    }

    if (((event->type == MOUSE_DOWN_UIEVENT) || (event->type == MOUSE_MOVED_UIEVENT)) &&
        (_isCaptured || isPointInRect(event->pt, splitterRect))) {
        if (!_isCaptured && event->type == MOUSE_DOWN_UIEVENT) {
            _isCaptured = responderChain()->captureResponder(this);
            return true;
        } else if (_isCaptured) {
            float newSplitPos = floor(event->pt.x) - rect().origin.x;
            if (newSplitPos < 0.0f) newSplitPos = 0.0f;

            Rect r = makeRect(0.0f, 0.0f, newSplitPos - (SPLITTER_WIDTH / 2), rect().size.height);
            _leftPane->setFrame(r);

            r = makeRect(newSplitPos + (SPLITTER_WIDTH / 2), 0.0f,
                         rect().size.width - newSplitPos - (SPLITTER_WIDTH / 2), rect().size.height);
            _rightPane->setFrame(r);

            if (_isPosRelativeToRight) newSplitPos = rect().size.width - newSplitPos;
            setSplitPos(newSplitPos);
            return true;
        }
        return false;
    } else if (_isCaptured && event->type == MOUSE_UP_UIEVENT) {
        responderChain()->releaseResponder(this);
        _isCaptured = false;
        return true;
    }

    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
void SplitViewH::updateFrames() {
    float splitPos = _splitPos;
    if (_isPosRelativeToRight) splitPos = rect().size.width - splitPos;

    if (_leftPane->isVisible() && _rightPane->isVisible()) {
        Rect r = makeRect(0.0f, 0.0f, splitPos - (SPLITTER_WIDTH / 2), rect().size.height);
        _leftPane->setFrame(r);
        r = makeRect(splitPos + (SPLITTER_WIDTH / 2), 0.0f, rect().size.width - splitPos - (SPLITTER_WIDTH / 2),
                     rect().size.height);
        _rightPane->setFrame(r);
    } else if (_leftPane->isVisible() && !_rightPane->isVisible()) {
        _leftPane->setFrame(bounds());
    } else if (!_leftPane->isVisible() && _rightPane->isVisible()) {
        _rightPane->setFrame(bounds());
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void SplitViewH::setFrame(Rect aRect) {
    View::setFrame(aRect);

    updateFrames();
}
