//
//  button.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2013-09-09.
//  Copyright (c) 2013-2020 Daniel Cliche. All rights reserved.
//

#include "button.h"

#include <math.h>

#include "draw.h"
#include "responderchain.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
Button::Button(const std::string& name, void* owner, const std::string& title, std::shared_ptr<Image> image)
    : Control(name, owner), _title(title), _image(image) {
    _font = SystemFonts::sharedInstance()->semiboldFont();
    _clickedFn = nullptr;
    _stateDidChangeFn = nullptr;
    _stateWithModifierDidChangeFn = nullptr;
    _type = StandardButtonType;
    _state = false;
    _isCaptured = false;
    _isHighlighted = false;
    _isEnabled = true;
    _isSortEnabled = false;
    _borderColor = systemButtonBorderColor;
    _highlightColor = blueColor;
    _textColor = whiteColor;
}

// ---------------------------------------------------------------------------------------------------------------------
Button::~Button() {}

// ---------------------------------------------------------------------------------------------------------------------
void Button::setState(bool state, bool isDelegateNotified, unsigned int modifierFlags) {
    if (_state != state) {
        _state = state;
        if (isDelegateNotified) {
            if (_stateDidChangeFn != nullptr) _stateDidChangeFn(this, state);
            if (_stateWithModifierDidChangeFn != nullptr) _stateWithModifierDidChangeFn(this, state, modifierFlags);
        }
        setDirty();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool Button::handleEvent(const UIEvent* event) {
    if (!isEnabled()) return false;

    if (event->type == KEY_UIEVENT) {
        if (event->key == KEY_ESCAPE && _type == CancelButtonType) {
            if (_clickedFn != nullptr) _clickedFn(this);
            return true;
        } else if (event->key == KEY_ENTER && _type == OKButtonType) {
            if (_clickedFn != nullptr) _clickedFn(this);
            return true;
        }
    }

    if (event->type == MOUSE_DOWN_UIEVENT && isPointInRect(event->pt, resolvedClippedRect())) {
        if ((_type != SegmentedControlButtonType) || (!_state)) {
            _isCaptured = responderChain()->captureResponder(this);
            _isHighlighted = true;
            setDirty();
        }
        return true;
    } else if (_isCaptured && (event->type == MOUSE_UP_UIEVENT || event->type == MOUSE_MOVED_UIEVENT)) {
        if (event->type == MOUSE_UP_UIEVENT) {
            responderChain()->releaseResponder(this);
            _isCaptured = false;
            if (isPointInRect(event->pt, resolvedClippedRect())) {
                if ((_type == CheckBoxButtonType) || (_type == CustomCheckBoxButtonType) ||
                    (_type == RadioButtonType) || (_type == DisclosureButtonType)) {
                    setState((_type != RadioButtonType) ? !_state : true, true, event->modifierFlags);
                } else if (_type == SegmentedControlButtonType) {
                    if (!_state) setState(true, true, event->modifierFlags);
                }
                if (_clickedFn != nullptr) _clickedFn(this);

                _isHighlighted = false;
                setDirty();
            }
        } else {
            _isHighlighted = isPointInRect(event->pt, resolvedClippedRect());
            setDirty();
        }
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
void Button::draw() {
    DrawContext* dc = drawContext();

    if (_image || (_type == SegmentedControlButtonType)) {
        Color imageColor = whiteColor;

        if (_isHighlighted) {
            imageColor = makeColor(1.0f, 1.0f, 1.0f, 0.5f);
        } else if (!_isEnabled) {
            imageColor = makeColor(0.5f, 0.5f, 0.5f, 0.5f);
        }

        if (_state) {
            dc->pushStates();
            dc->setFillColor(_type != CustomCheckBoxButtonType ? _highlightColor : redColor);
            dc->drawRect(bounds());
            dc->popStates();
        }
        if (_image) {
            Rect r =
                makeCenteredRectInRectLimited(bounds(), floorf(_image->size().width), floorf(_image->size().height));
            dc->drawImage(r, _image, imageColor);
        }
        dc->pushStates();
        dc->setStrokeColor(_borderColor);
        dc->drawRect(bounds());
        dc->popStates();
    } else {
        Color color;
        if (_type == CheckBoxButtonType || _type == CustomCheckBoxButtonType || _type == SegmentedControlButtonType ||
            _type == RadioButtonType) {
            color = _state ? (_type == CheckBoxButtonType ? redColor : _highlightColor) : veryDimGrayColor;
        } else {
            if (!_image) {
                color = blackColor;
            }
        }

        if (_type != CheckBoxButtonType && _type != CustomCheckBoxButtonType && _type != RadioButtonType &&
            _type != DisclosureButtonType && _type != SortButtonType) {
            dc->pushStates();
            dc->setFillColor(color);
            dc->setStrokeColor(_isEnabled ? _borderColor : grayColor);
            dc->setStrokeWidth(_type == OKButtonType ? 2.0f : 1.0f);
            dc->drawRoundRect(bounds(), 5.0f);
            dc->popStates();
        } else {
            Rect r;
            if (_type == SortButtonType) {
                r = makeCenteredRightRectInRect(bounds(), 20.0f, 20.0f);
            } else if (_type != CustomCheckBoxButtonType) {
                r = makeCenteredLeftRectInRect(bounds(), 15.0f, 15.0f);
            } else {
                r = bounds();
            }
            if (_type == RadioButtonType) {
                dc->pushStates();
                dc->setFillColor(color);
                dc->setStrokeColor(_borderColor);
                dc->drawRoundRect(r, r.size.width / 2.0f);
                dc->popStates();
            } else if (_type != DisclosureButtonType && _type != SortButtonType) {
                dc->pushStates();
                dc->setFillColor(color);
                dc->setStrokeColor(_borderColor);
                dc->drawRect(r);
                dc->popStates();
            }
            if (_type == CheckBoxButtonType || _type == RadioButtonType) {
                if (_state) {
                    auto image = _type == CheckBoxButtonType ? SystemImages::sharedInstance()->checkMarkImage()
                                                             : SystemImages::sharedInstance()->radioButtonImage();
                    r = makeCenteredRectInRectLimited(r, floorf(image->size().width), floorf(image->size().height));
                    dc->drawImage(r, image);
                }
            } else if (_type == DisclosureButtonType) {
                auto image = _state ? SystemImages::sharedInstance()->downArrowImage()
                                    : SystemImages::sharedInstance()->rightArrowImage();
                r = makeCenteredRectInRectLimited(r, floorf(image->size().width), floorf(image->size().height));
                dc->drawImage(r, image);
            } else if (_type == SortButtonType) {
                if (_isSortEnabled) {
                    auto image = _state ? SystemImages::sharedInstance()->upArrowImage()
                                        : SystemImages::sharedInstance()->downArrowImage();
                    r = makeCenteredRectInRectLimited(r, floorf(image->size().width), floorf(image->size().height));
                    dc->drawImage(r, image);
                }
            }
        }
    }

    Rect textRect = bounds();

    if (_type == ComboBoxDownButtonType) {
        textRect.size.width -= 20.0f;
        dc->drawImage(makeCenteredRightRectInRect(bounds(), 10.0f, 10.0f, 5.0f),
                      SystemImages::sharedInstance()->downArrowImage());
    } else if (_type == ComboBoxUpButtonType) {
        textRect.size.width -= 20.0f;
        dc->drawImage(makeCenteredRightRectInRect(bounds(), 10.0f, 10.0f, 5.0f),
                      SystemImages::sharedInstance()->upArrowImage());
    } else if (_type == CheckBoxButtonType || _type == RadioButtonType) {
        textRect = makeRect(bounds().origin.x + 20.0f, bounds().origin.y, bounds().size.width, bounds().size.height);
    } else if (_type == SortButtonType) {
        textRect.size.width -= 20.0f;
    }

    if ((_type == CheckBoxButtonType) || (_type == RadioButtonType) || (_type == SortButtonType)) {
        dc->pushStates();
        dc->setStrokeColor(_isEnabled ? _textColor : grayColor);
        dc->drawLeftText(_font, textRect, _title);
        dc->popStates();
    } else {
        dc->pushStates();
        dc->setStrokeColor(_textColor);
        dc->drawCenteredText(_font, textRect, _title);
        dc->popStates();
    }

    if (!_image) {
        if (_type != CheckBoxButtonType && _type != RadioButtonType) {
            if (_isHighlighted) {
                dc->pushStates();
                dc->setFillColor(makeColor(1.0f, 1.0f, 1.0f, 0.5f));
                if (_type == CustomCheckBoxButtonType) {
                    dc->drawRect(bounds());
                } else {
                    dc->drawRoundRect(bounds(), 5.0f);
                }
                dc->popStates();
            } else if (!_isEnabled && (_type != SortButtonType)) {
                dc->pushStates();
                dc->setFillColor(makeColor(0.0f, 0.0f, 0.0f, 0.5f));
                if (_type == CustomCheckBoxButtonType) {
                    dc->drawRect(makeInsetRect(bounds(), 1.0f, 1.0f));
                } else {
                    dc->drawRoundRect(makeInsetRect(bounds(), 1.0f, 1.0f), 5.0f);
                }
                dc->popStates();
            }
        } else {
            Rect r = makeCenteredLeftRectInRect(bounds(), 15.0f, 15.0f);

            if (_isHighlighted) {
                dc->pushStates();
                dc->setFillColor(makeColor(1.0f, 1.0f, 1.0f, 0.5f));
                if (_type == CheckBoxButtonType) {
                    dc->drawRect(r);
                } else {
                    dc->drawRoundRect(r, 7.0f);
                }
                dc->popStates();
            } else if (!_isEnabled) {
                dc->pushStates();
                dc->setFillColor(makeColor(0.0f, 0.0f, 0.0f, 0.5f));
                if (_type == CheckBoxButtonType) {
                    dc->drawRect(r);
                } else {
                    dc->drawRoundRect(r, 7.0f);
                }
                dc->popStates();
            }
        }
    }
}
