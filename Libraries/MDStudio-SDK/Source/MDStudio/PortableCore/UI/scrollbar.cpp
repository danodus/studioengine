//
//  scrollbar.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-28.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#include "scrollbar.h"

#include "draw.h"
#include "responderchain.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
ScrollBar::ScrollBar(std::string name, void* owner, float min, float max, float posMin, float posMax)
    : _min(min), _max(max), _posMin(posMin), _posMax(posMax), Control(name, owner) {
    _trackingPosMin = posMin;
    _trackingPosMax = posMax;
    _isCaptured = false;
    _posChangedFn = nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------
ScrollBar::~ScrollBar() {}

// ---------------------------------------------------------------------------------------------------------------------
MDStudio::Rect ScrollBar::makeAdjustedKnobRect(Rect knobRect) {
    // Adjust vertically

    // If the height is below 20px, we set a fixed size
    float maxHeight = std::min(20.0f, bounds().size.height);
    if (knobRect.size.height < maxHeight) {
        float delta = maxHeight - knobRect.size.height;
        knobRect.origin.y -= delta / 2.0f;
        knobRect.size.height = maxHeight;
        if (knobRect.origin.y < 0) knobRect.origin.y = 0;
        if (knobRect.origin.y + knobRect.size.height > bounds().size.height)
            knobRect.origin.y = bounds().size.height - knobRect.size.height;
    }

    // Adjust horizontally

    // If the width is below 20px, we set a fixed size
    float maxWidth = std::min(20.0f, bounds().size.width);
    if (knobRect.size.width < maxHeight) {
        float delta = maxWidth - knobRect.size.height;
        knobRect.origin.x -= delta / 2.0f;
        knobRect.size.width = maxWidth;
        if (knobRect.origin.x < 0) knobRect.origin.x = 0;
        if (knobRect.origin.x + knobRect.size.width > bounds().size.width)
            knobRect.origin.x = bounds().size.width - knobRect.size.width;
    }

    return knobRect;
}

// ---------------------------------------------------------------------------------------------------------------------
void ScrollBar::draw() {
    DrawContext* dc = drawContext();

    dc->pushStates();
    dc->setFillColor(veryDimGrayColor);
    dc->setStrokeColor(lightGrayColor);
    dc->drawRect(bounds());
    dc->popStates();

    // If horizontal scroll bar
    Rect knobRect;
    if (bounds().size.width > bounds().size.height) {
        float f = frame().size.width / _max;
        knobRect = makeRect(f * _posMin, 0.0f, f * (_posMax - _posMin), frame().size.height);
    } else {
        // Vertical scroll bar
        float f = frame().size.height / _max;
        knobRect = makeRect(0.0f, f * _posMin, frame().size.width, f * (_posMax - _posMin));
    }

    knobRect = makeAdjustedKnobRect(knobRect);

    knobRect = makeInsetRect(knobRect, 2.0f, 2.0f);

    dc->pushStates();
    dc->setFillColor(lightGrayColor);
    dc->setStrokeColor(veryDimGrayColor);
    dc->drawRoundRect(knobRect, 5.0f);
    dc->popStates();
}

// ---------------------------------------------------------------------------------------------------------------------
bool ScrollBar::handleEvent(const UIEvent* event) {
    if ((event->type == MOUSE_DOWN_UIEVENT || event->type == MOUSE_MOVED_UIEVENT) &&
        (_isCaptured || isPointInRect(event->pt, resolvedClippedRect()))) {
        // If horizontal scroll bar
        if (rect().size.width > rect().size.height) {
            Rect knobRect;
            float f = rect().size.width / _max;
            knobRect = makeRect(f * _trackingPosMin, 0.0f, f * (_trackingPosMax - _trackingPosMin), rect().size.height);
            knobRect = makeAdjustedKnobRect(knobRect);
            knobRect.origin.x += resolvedRect().origin.x;
            knobRect.origin.y += resolvedRect().origin.y;

            if (_isCaptured || isPointInRect(event->pt, knobRect)) {
                if (!_isCaptured && (event->type == MOUSE_DOWN_UIEVENT)) {
                    _isCaptured = responderChain()->captureResponder(this);
                    _trackingPosMin = _posMin;
                    _trackingPosMax = _posMax;
                } else if (_isCaptured) {
                    float delta = (event->pt.x - _lastEvent.pt.x) / f;
                    _trackingPosMin += delta;
                    _trackingPosMax += delta;
                    setPos(_trackingPosMin, _trackingPosMax);
                }
                _lastEvent = *event;
            }
        } else {
            // Vertical scroll bar
            Rect knobRect;
            float f = rect().size.height / _max;
            knobRect = makeRect(0.0f, f * _trackingPosMin, rect().size.width, f * (_trackingPosMax - _trackingPosMin));
            knobRect = makeAdjustedKnobRect(knobRect);
            knobRect.origin.x += resolvedRect().origin.x;
            knobRect.origin.y += resolvedRect().origin.y;

            if (_isCaptured || isPointInRect(event->pt, knobRect)) {
                if (!_isCaptured && (event->type == MOUSE_DOWN_UIEVENT)) {
                    _isCaptured = responderChain()->captureResponder(this);
                    _trackingPosMin = _posMin;
                    _trackingPosMax = _posMax;
                } else if (_isCaptured) {
                    float delta = (event->pt.y - _lastEvent.pt.y) / f;
                    _trackingPosMin += delta;
                    _trackingPosMax += delta;
                    setPos(_trackingPosMin, _trackingPosMax);
                }
                _lastEvent = *event;
            }
        }

        return true;
    } else if (event->type == MOUSE_UP_UIEVENT && _isCaptured) {
        responderChain()->releaseResponder(this);
        _isCaptured = false;
        _trackingPosMin = _posMin;
        _trackingPosMax = _posMax;
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
void ScrollBar::setMin(float min) {
    _min = min;
    if (_posMin < _min) _posMin = _min;

    if (!_isCaptured) {
        _trackingPosMin = _posMin;
        _trackingPosMax = _posMax;
    }

    setDirty();

    if (_posChangedFn) _posChangedFn(this, _posMin, _posMax);
}

// ---------------------------------------------------------------------------------------------------------------------
void ScrollBar::setMax(float max) {
    _max = max;
    if (_posMax > _max) _posMax = _max;

    if (!_isCaptured) {
        _trackingPosMin = _posMin;
        _trackingPosMax = _posMax;
    }

    setDirty();

    if (_posChangedFn) _posChangedFn(this, _posMin, _posMax);
}

// ---------------------------------------------------------------------------------------------------------------------
void ScrollBar::setPos(float posMin, float posMax) {
    float delta = posMax - posMin;
    if (posMax > _max) {
        posMax = _max;
        posMin = posMax - delta;
    }
    if (posMin < _min) {
        posMin = _min;
        posMax = posMin + delta;
    }
    if (posMin < _min) posMin = _min;
    if (posMax > _max) posMax = _max;
    _posMin = posMin;
    _posMax = posMax;

    if (!_isCaptured) {
        _trackingPosMin = _posMin;
        _trackingPosMax = _posMax;
    }

    setDirty();

    if (_posChangedFn) _posChangedFn(this, _posMin, _posMax);
}
