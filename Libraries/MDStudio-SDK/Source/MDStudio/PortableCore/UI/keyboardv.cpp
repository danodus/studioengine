//
//  keyboardv.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-26.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#include "keyboardv.h"

#include "draw.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
KeyboardV::KeyboardV(std::string name, void* owner) : Keyboard(name, owner) {
    _whiteKeyImage = std::make_shared<Image>("WhiteKeyH@2x.png");
    _blackKeyImage = std::make_shared<Image>("BlackKeyH@2x.png");
    _whiteKeyPressedImage = std::make_shared<Image>("WhiteKeyPressedH@2x.png");
    _blackKeyPressedImage = std::make_shared<Image>("BlackKeyPressedH@2x.png");

    calculateKeyLocations();
}

// ---------------------------------------------------------------------------------------------------------------------
KeyboardV::~KeyboardV() {}

// ---------------------------------------------------------------------------------------------------------------------
void KeyboardV::calculateKeyLocations() {
    Size whiteKeySize = _whiteKeyImage->size();
    Size blackKeySize = _blackKeyImage->size();

    // We calculate the locations of the white keys
    for (int i = 0; i < 7; i++) {
        _whiteKeyLocations[i] = makeRect(0, i * whiteKeySize.height, whiteKeySize.width, whiteKeySize.height);
    }

    // We calculate the locations of the black keys
    for (int i = 0; i < 5; i++)
        _blackKeyLocations[i] =
            makeRect(0, blackKeyToPitchMapping[i] * blackKeySize.height, blackKeySize.width, blackKeySize.height);
}

// ---------------------------------------------------------------------------------------------------------------------
void KeyboardV::draw() {
    DrawContext* dc = drawContext();

    float octaveHeight = _blackKeyImage->size().height * 12.0f;

    // For each octave
    for (int octave = 0; octave < 8; octave++) {
        // We draw the white keys
        for (int i = 0; i < 7; i++) {
            Rect rc = _whiteKeyLocations[i];
            rc.origin.y += octave * octaveHeight;

            int pitch = 12 * octave + whiteKeyToPitchMapping[i];

            bool keyState = _keyStates[_currentChannel][pitch];

            dc->pushStates();

            if (keyState) {
                dc->drawImage(rc.origin, _whiteKeyPressedImage);
            } else {
                dc->drawImage(rc.origin, _whiteKeyImage);
            }

            dc->popStates();

            if (_isDrumKit) {
                auto it = drumKitLabels.find(pitch + 12);
                if (it != drumKitLabels.end()) {
                    Rect r = rc;
                    r.size.width -= 2.0f;
                    r.origin.y += drumKitWhiteKeyLabelOffsets[i];
                    dc->pushStates();
                    dc->setStrokeColor(veryDimGrayColor);
                    dc->drawRightText(_font, r, it->second);
                    dc->popStates();
                }
            } else {
                // Not a drum kit
                if (i == 0) {
                    // Draw the octave label
                    Rect r = rc;
                    r.origin.x += rc.size.width - 20.0f;
                    r.size.width = 20.0f;
                    dc->pushStates();
                    dc->setStrokeColor(veryDimGrayColor);
                    dc->drawCenteredText(SystemFonts::sharedInstance()->semiboldFont(), r,
                                         "C" + std::to_string(octave - 1));
                    dc->popStates();
                }
            }
        }

        // We draw the black keys
        for (int i = 0; i < 5; i++) {
            Rect rc = _blackKeyLocations[i];
            rc.origin.y += octave * octaveHeight;

            int pitch = 12 * octave + blackKeyToPitchMapping[i];

            bool keyState = _keyStates[_currentChannel][pitch];

            dc->pushStates();

            if (keyState) {
                dc->drawImage(rc.origin, _blackKeyPressedImage);
            } else {
                dc->drawImage(rc.origin, _blackKeyImage);
            }

            dc->popStates();

            if (_isDrumKit) {
                dc->pushStates();
                dc->setStrokeColor(whiteColor);
                auto it = drumKitLabels.find(pitch + 12);
                if (it != drumKitLabels.end()) {
                    Rect r = rc;
                    r.size.width -= 2.0f;
                    dc->drawRightText(_font, r, it->second);
                }
                dc->popStates();
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
int KeyboardV::pitchAtPoint(Point pt) {
    int pitch = 0;

    Size whiteKeySize = _whiteKeyImage->size();
    Size blackKeySize = _blackKeyImage->size();

    int octave = pt.y / (whiteKeySize.height * 7);

    if (pt.x > blackKeySize.width) {
        // A white note has been pressed
        int whiteKey = (int)(pt.y) % (int)(whiteKeySize.height * 7) / (int)(whiteKeySize.height);
        pitch = octave * 12 + whiteKeyToPitchMapping[whiteKey];
    } else {
        // A black or white key has been pressed
        int key = (int)(pt.y) % (int)(whiteKeySize.height * 7) / (int)(blackKeySize.height);
        pitch = octave * 12 + key;
    }
    return pitch + 12;
}

// ---------------------------------------------------------------------------------------------------------------------
bool KeyboardV::handleEvent(const UIEvent* event) {
    // Handle the cursor during the selection
    if ((event->type == MOUSE_MOVED_UIEVENT) && isPointInRect(event->pt, clippedRect())) {
        responderChain()->setCursorInRect(this, Platform::PointingHandCursor, clippedRect());
    }

    if (!_isCaptured && isPointInRect(event->pt, resolvedClippedRect())) {
        if (event->type == MOUSE_DOWN_UIEVENT) {
            _isCaptured = responderChain()->captureResponder(this);
        } else if (event->type == SCROLL_UIEVENT) {
            if (_scrollFn) _scrollFn(this, event->deltaY);
        }
    }

    if (_isCaptured &&
        (event->type == MOUSE_DOWN_UIEVENT || event->type == MOUSE_UP_UIEVENT || event->type == MOUSE_MOVED_UIEVENT)) {
        Point pt = event->pt;
        pt.x = pt.x - rect().origin.x - offset().x;
        pt.y = pt.y - rect().origin.y - offset().y;
        int pitch = pitchAtPoint(pt);
        if (event->type == MOUSE_DOWN_UIEVENT || event->type == MOUSE_MOVED_UIEVENT) {
            if (!isPointInRect(event->pt, resolvedClippedRect())) pitch = -1;

            if (_highlightedKey != pitch) {
                if ((_highlightedKey >= 0) && (_keyReleasedFn != nullptr)) _keyReleasedFn(this, _highlightedKey);

                _highlightedKey = pitch;
                setDirty();
                if (pitch >= 0) {
                    if (_keyPressedFn != nullptr) _keyPressedFn(this, pitch);
                }
            }
            return true;
        } else if (event->type == MOUSE_UP_UIEVENT) {
            responderChain()->releaseResponder(this);
            _isCaptured = false;
            _highlightedKey = -1;
            setDirty();

            if (!isPointInRect(event->pt, clippedRect())) {
                responderChain()->releaseCursor();
            }

            if (_keyReleasedFn != nullptr) _keyReleasedFn(this, pitch);
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
Size KeyboardV::contentSize() {
    return makeSize(_whiteKeyImage->size().width, _blackKeyImage->size().height * 12.0f * 8.0f);
}
