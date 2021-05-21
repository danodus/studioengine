//
//  slider.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-14.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#include "slider.h"

#include <math.h>

#include "draw.h"
#include "responderchain.h"

#define TAU (2 * M_PI)

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
Slider::Slider(std::string name, void* owner, float min, float max, float pos)
    : _min(min), _max(max), _pos(pos), Control(name, owner) {
    _trackingPos = pos;
    _isCaptured = false;
    _type = LinearSliderType;
    _posChangedFn = nullptr;
    _posSetFn = nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------
Slider::~Slider() {}

// ---------------------------------------------------------------------------------------------------------------------
bool Slider::handleEvent(const UIEvent* event) {
    if ((event->type == MOUSE_DOWN_UIEVENT || event->type == MOUSE_MOVED_UIEVENT || event->type == SCROLL_UIEVENT) &&
        (_isCaptured || isPointInRect(event->pt, resolvedClippedRect()))) {
        if (event->type == SCROLL_UIEVENT) {
            setPos(pos() + event->deltaY * (_max - _min) * 0.001f);
        } else if (_type == LinearSliderType) {
            // If horizontal slider
            if (rect().size.width > rect().size.height) {
                Rect knobRect;
                float f = (rect().size.width - bounds().size.height) / (_max - _min);
                knobRect = makeRect(resolvedRect().origin.x + bounds().size.height / 2.0f + f * (_trackingPos - _min) -
                                        bounds().size.height / 2.0f,
                                    resolvedRect().origin.y, bounds().size.height, rect().size.height);

                if (_isCaptured || isPointInRect(event->pt, knobRect)) {
                    if (!_isCaptured && (event->type == MOUSE_DOWN_UIEVENT)) {
                        _isCaptured = responderChain()->captureResponder(this);
                        _trackingPos = _pos;
                    } else if (_isCaptured) {
                        float delta = (event->pt.x - _lastEvent.pt.x) / f;
                        _trackingPos += delta;
                        setPos(_trackingPos);
                    }
                    _lastEvent = *event;
                }
            } else {
                // Vertical slider
                Rect knobRect;

                float f = (bounds().size.height - bounds().size.width) / (_max - _min);
                knobRect = makeRect(resolvedRect().origin.x,
                                    resolvedRect().origin.y + bounds().size.width / 2.0f + f * (_pos - _min) -
                                        bounds().size.width / 2.0f,
                                    bounds().size.width, bounds().size.width);

                if (_isCaptured || isPointInRect(event->pt, knobRect)) {
                    if (!_isCaptured && (event->type == MOUSE_DOWN_UIEVENT)) {
                        _isCaptured = responderChain()->captureResponder(this);
                        _trackingPos = _pos;
                    } else if (_isCaptured) {
                        float delta = (event->pt.y - _lastEvent.pt.y) / f;
                        _trackingPos += delta;
                        setPos(_trackingPos);
                    }
                    _lastEvent = *event;
                }
            }
        } else if (_type == RadialSliderType) {
            Rect r = resolvedClippedRect();
            Point center = makePoint(r.origin.x + r.size.width / 2.0f, r.origin.y + r.size.height / 2.0f);
            if (_isCaptured || isPointInRect(event->pt, r)) {
                if (!_isCaptured && (event->type == MOUSE_DOWN_UIEVENT)) {
                    _isCaptured = responderChain()->captureResponder(this);
                    _trackingPos = _pos;
                } else if (_isCaptured) {
                    double angle = angleBetweenPoints(center, event->pt);
                    if (angle > 5.0 / 8.0 * TAU && angle <= 3.0 / 4.0 * TAU) {
                        angle = 5.0 / 8.0 * TAU;
                    } else if (angle > 3.0 / 4.0 * TAU && angle < TAU - 1.0 / 8.0 * TAU) {
                        angle = 7.0 / 8.0 * TAU;
                    }
                    angle = -angle + 5.0 / 8.0 * TAU;
                    if (angle < 0) angle += TAU;
                    _trackingPos = angle * (_max - _min) / (3.0 / 4.0 * TAU) + _min;
                    setPos(_trackingPos);
                }
                _lastEvent = *event;
            }
        }

        return true;
    } else if (event->type == MOUSE_UP_UIEVENT && _isCaptured) {
        responderChain()->releaseResponder(this);
        _isCaptured = false;

        _trackingPos = _pos;

        if (_posSetFn) _posSetFn(this, _pos);

        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
void Slider::drawHSlider(DrawContext* dc, Rect r) {
    Rect knobRect, railRect;

    float f = (r.size.width - r.size.height) / (_max - _min);
    knobRect = makeRect(floorf(r.origin.x + r.size.height / 2.0f + f * (_pos - _min) - r.size.height / 2.0f),
                        floorf(r.origin.y), floorf(r.size.height), floorf(r.size.height));
    auto railHeight = 3.0f;
    if (_thumbImage && _middleRailImage)
        railHeight = _middleRailImage->size().height * knobRect.size.height / _thumbImage->size().height;
    railRect = makeCenteredRectInRect(r, r.size.width - knobRect.size.width, railHeight);

    dc->pushStates();
    if (_minRailImage || _middleRailImage || _maxRailImage) {
        auto minRailWidth = 0.0f;
        auto maxRailWidth = 0.0f;
        if (_minRailImage && _thumbImage)
            minRailWidth = _minRailImage->size().width * knobRect.size.height / _thumbImage->size().height;
        if (_maxRailImage && _thumbImage)
            maxRailWidth = _maxRailImage->size().width * knobRect.size.height / _thumbImage->size().height;
        if (_middleRailImage)
            dc->drawImage(makeRect(railRect.origin.x + minRailWidth, railRect.origin.y,
                                   railRect.size.width - minRailWidth - maxRailWidth, railRect.size.height),
                          _middleRailImage);
        if (_minRailImage)
            dc->drawImage(makeRect(railRect.origin.x, railRect.origin.y, minRailWidth, railRect.size.height),
                          _minRailImage);
        if (_maxRailImage)
            dc->drawImage(makeRect(railRect.origin.x + railRect.size.width - maxRailWidth, railRect.origin.y,
                                   maxRailWidth, railRect.size.height),
                          _maxRailImage);
    } else {
        dc->setFillColor(veryDimGrayColor);
        dc->setStrokeColor(lightGrayColor);
        dc->drawRect(railRect);
    }

    dc->drawImage(knobRect, _thumbImage ? _thumbImage : SystemImages::sharedInstance()->sliderThumbImage());
    dc->popStates();
}

// ---------------------------------------------------------------------------------------------------------------------
void Slider::draw() {
    DrawContext* dc = drawContext();

    if (_type == LinearSliderType) {
        // If horizontal slider
        if (bounds().size.width > bounds().size.height) {
            dc->pushStates();
            auto r = bounds();
            auto t = dc->translation();
            dc->setTranslation(makePoint(t.x + r.size.width / 2.0f, t.y + r.size.height / 2.0f));
            drawHSlider(dc, makeRect(-r.size.width / 2.0f, -r.size.height / 2.0f, r.size.width, r.size.height));
            dc->popStates();
        } else {
            // Vertical slider
            dc->pushStates();
            auto r = bounds();
            auto t = dc->translation();
            dc->setTranslation(makePoint(t.x + r.size.width / 2.0f, t.y + r.size.height / 2.0f));
            dc->setRotation(90.0f);
            drawHSlider(dc, makeRect(-r.size.height / 2.0f, -r.size.width / 2.0f, r.size.height, r.size.width));
            dc->popStates();
        }
    } else if (_type == RadialSliderType) {
        Rect r = bounds();
        double knobAngle = -3.0 / 4.0 * TAU * (_pos - _min) / (_max - _min) + 5.0 / 8.0 * TAU;

        if (_thumbImage) {
            dc->pushStates();
            auto t = dc->translation();
            dc->setTranslation(
                makePoint(t.x + r.origin.x + r.size.width / 2.0f, t.y + r.origin.y + r.size.height / 2.0f));
            auto radius = std::min(r.size.width, r.size.height);
            dc->setRotation(knobAngle * 180.0 / M_PI);
            dc->drawImage(makeRect(-radius / 2.0f, -radius / 2.0f, radius, radius), _thumbImage);
            dc->popStates();
        } else {
            // Draw outside arc
            dc->pushStates();
            dc->setStrokeWidth(1.5f);
            dc->setStrokeColor(lightGrayColor);

            dc->drawArc(makePoint(r.origin.x + r.size.width / 2.0f, r.origin.y + r.size.height / 2.0f),
                        r.size.width / 2.0f - 2.0f, -1.0 / 8.0 * TAU, 3.0 / 4.0 * TAU);
            dc->popStates();

            // Draw knob
            dc->pushStates();
            float knobRailRadius = 1.6f / 3.0f * r.size.width / 2.0f;

            Point knobPoint = makePoint(r.origin.x + r.size.width / 2.0f + knobRailRadius * cos(knobAngle),
                                        r.origin.y + r.size.height / 2.0f + knobRailRadius * sin(knobAngle));
            dc->setFillColor(lightGrayColor);
            dc->setStrokeColor(lightGrayColor);
            dc->setStrokeWidth(0.5f);
            dc->drawCircle(knobPoint, 2.0f);
            dc->popStates();
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Slider::setMin(float min, bool isDelegateNotified) {
    _min = min;
    if (_pos < _min) _pos = _min;

    if (!_isCaptured) _trackingPos = _pos;

    setDirty();

    if (isDelegateNotified && _posChangedFn) _posChangedFn(this, _pos);
}

// ---------------------------------------------------------------------------------------------------------------------
void Slider::setMax(float max, bool isDelegateNotified) {
    _max = max;
    if (_pos > _max) _pos = _max;

    if (!_isCaptured) _trackingPos = _pos;

    setDirty();

    if (isDelegateNotified && _posChangedFn) _posChangedFn(this, _pos);
}

// ---------------------------------------------------------------------------------------------------------------------
void Slider::setPos(float pos, bool isDelegateNotified) {
    _pos = pos;

    // Clamp the actual pos
    if (_pos < _min) _pos = _min;
    if (_pos > _max) _pos = _max;

    if (!_isCaptured) _trackingPos = _pos;

    setDirty();

    if (isDelegateNotified && _posChangedFn) _posChangedFn(this, _pos);
}
