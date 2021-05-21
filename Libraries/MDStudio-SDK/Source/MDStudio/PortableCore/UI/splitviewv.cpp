//
//  splitviewv.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2013-09-09.
//  Copyright (c) 2016-2020 Daniel Cliche. All rights reserved.
//

#include "splitviewv.h"

#include "../platform.h"
#include "draw.h"
#include "math.h"
#include "responderchain.h"

#define SPLITTER_HEIGHT 6

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
SplitViewV::SplitViewV(const std::string& name, void* owner, std::shared_ptr<View> bottomPane,
                       std::shared_ptr<View> topPane, float splitPos)
    : View(name, owner), _bottomPane(bottomPane), _topPane(topPane) {
    addSubview(bottomPane);
    addSubview(topPane);

    setSplitPos(splitPos);

    _isCaptured = false;

    _posChangedFn = nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------
SplitViewV::~SplitViewV() {
    removeSubview(_bottomPane);
    removeSubview(_topPane);
}

// ---------------------------------------------------------------------------------------------------------------------
void SplitViewV::setSplitPos(float splitPos) {
    Rect r;

    r = makeRect(0.0f, 0.0f, rect().size.width, splitPos - (SPLITTER_HEIGHT / 2));
    _bottomPane->setFrame(r);

    r = makeRect(0.0f, splitPos + (SPLITTER_HEIGHT / 2), rect().size.width,
                 rect().size.height - splitPos - (SPLITTER_HEIGHT / 2));
    _topPane->setFrame(r);

    _splitPos = splitPos;

    setDirty();

    if (_posChangedFn) _posChangedFn(this, _splitPos);
}

// ---------------------------------------------------------------------------------------------------------------------
void SplitViewV::draw() {
    DrawContext* dc = drawContext();

    if (_bottomPane->isVisible() && _topPane->isVisible()) {
        Rect r = makeRect(-1.0f, _splitPos - (SPLITTER_HEIGHT / 2), rect().size.width + 1.0f, SPLITTER_HEIGHT);
        dc->pushStates();
        dc->setFillColor(lightGrayColor);
        dc->setStrokeColor(veryDimGrayColor);
        dc->drawRect(r);
        dc->popStates();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool SplitViewV::handleEvent(const UIEvent* event) {
    Rect splitterRect = makeZeroRect();
    if (_bottomPane->isVisible() && _topPane->isVisible())
        splitterRect = makeRect(rect().origin.x, rect().origin.y + _splitPos - (SPLITTER_HEIGHT / 2), rect().size.width,
                                SPLITTER_HEIGHT);

    // Update the mouse cursor
    if (isMouseEvent(event)) {
        if (isPointInRect(event->pt, splitterRect)) {
            responderChain()->setCursorInRect(this, Platform::ResizeUpDownCursor, splitterRect);
        }
    }

    if (((event->type == MOUSE_DOWN_UIEVENT) || (event->type == MOUSE_MOVED_UIEVENT)) &&
        (_isCaptured || isPointInRect(event->pt, splitterRect))) {
        if (!_isCaptured && event->type == MOUSE_DOWN_UIEVENT) {
            _isCaptured = responderChain()->captureResponder(this);
            return true;
        } else if (_isCaptured) {
            float splitPos = floor(event->pt.y) - rect().origin.y;
            if (splitPos < 0.0f) splitPos = 0.0f;

            Rect r = makeRect(0.0f, 0.0f, rect().size.width, rect().origin.y + _splitPos - (SPLITTER_HEIGHT / 2));
            _bottomPane->setFrame(r);

            r = makeRect(0.0f, _splitPos + (SPLITTER_HEIGHT / 2), rect().size.width,
                         rect().size.height - _splitPos - (SPLITTER_HEIGHT / 2));
            _topPane->setFrame(r);

            setSplitPos(splitPos);
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
void SplitViewV::updateFrames() {
    if (_bottomPane->isVisible() && _topPane->isVisible()) {
        Rect r = makeRect(0.0f, 0.0f, rect().size.width, _splitPos - (SPLITTER_HEIGHT / 2));
        _bottomPane->setFrame(r);

        r = makeRect(0.0f, _splitPos + (SPLITTER_HEIGHT / 2), rect().size.width,
                     rect().size.height - _splitPos - (SPLITTER_HEIGHT / 2));
        _topPane->setFrame(r);
    } else if (_bottomPane->isVisible() && !_topPane->isVisible()) {
        _bottomPane->setFrame(bounds());
    } else if (!_bottomPane->isVisible() && _topPane->isVisible()) {
        _topPane->setFrame(bounds());
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void SplitViewV::setFrame(Rect aRect) {
    View::setFrame(aRect);

    updateFrames();
}
