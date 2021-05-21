//
//  keyboardh.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-26.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#include "keyboardh.h"

#include "draw.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
KeyboardH::KeyboardH(std::string name, void* owner) : Keyboard(name, owner) {
    _whiteKeyImage = std::make_shared<Image>("WhiteKey@2x.png");
    _blackKeyImage = std::make_shared<Image>("BlackKey@2x.png");
    _whiteKeyPressedImage = std::make_shared<Image>("WhiteKeyPressed@2x.png");
    _blackKeyPressedImage = std::make_shared<Image>("BlackKeyPressed@2x.png");

    calculateKeyLocations();
}

// ---------------------------------------------------------------------------------------------------------------------
KeyboardH::~KeyboardH() {}

// ---------------------------------------------------------------------------------------------------------------------
void KeyboardH::calculateKeyLocations() {
    Size whiteKeySize = _whiteKeyImage->size();
    Size blackKeySize = _blackKeyImage->size();

    // We calculate the locations of the white keys
    for (int i = 0; i < 7; i++) {
        _whiteKeyLocations[i] = makeRect(i * whiteKeySize.width, 0.0f, whiteKeySize.width, whiteKeySize.height);
    }

    // We calculate the locations of the black keys
    for (int i = 0; i < 5; i++)
        _blackKeyLocations[i] =
            makeRect(blackKeyToPitchMapping[i] * blackKeySize.width, whiteKeySize.height - blackKeySize.height,
                     blackKeySize.width, blackKeySize.height);
}

// ---------------------------------------------------------------------------------------------------------------------
void KeyboardH::draw() {
    DrawContext* dc = drawContext();

    float octaveWidth = _blackKeyImage->size().width * 12.0f;

    // For each octave
    for (int octave = 0; octave < 8; octave++) {
        // We draw the white keys
        for (int i = 0; i < 7; i++) {
            Rect rc = _whiteKeyLocations[i];
            rc.origin.x += octave * octaveWidth;

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
                    r.origin.x += drumKitWhiteKeyLabelOffsets[i];
                    dc->pushStates();
                    dc->setStrokeColor(veryDimGrayColor);
                    auto t = dc->translation();
                    dc->setTranslation(
                        makePoint(t.x + r.origin.x + r.size.width / 2.0f, t.y + r.origin.y + r.size.height / 2.0f));
                    dc->setRotation(90.0f);
                    dc->drawRightText(
                        _font,
                        makeRect(-r.size.height / 2.0f - 2.0f, -r.size.width / 2.0f - drumKitWhiteKeyLabelOffsets[i],
                                 r.size.height, r.size.width),
                        it->second);
                    dc->popStates();
                }
            } else {
                // Not a drum kit
                if (i == 0) {
                    // Draw the octave label
                    Rect r = rc;
                    r.origin.y += rc.size.width - 20.0f;
                    r.size.height = 20.0f;
                    dc->pushStates();
                    dc->setStrokeColor(veryDimGrayColor);
                    auto t = dc->translation();
                    dc->setTranslation(makePoint(t.x + r.size.width / 2.0f, t.y + r.size.height / 2.0f));
                    dc->setRotation(90.0f);
                    dc->drawCenteredText(SystemFonts::sharedInstance()->semiboldFont(), r,
                                         "C" + std::to_string(octave - 1));
                    dc->popStates();
                }
            }
        }

        // We draw the black keys
        for (int i = 0; i < 5; i++) {
            Rect rc = _blackKeyLocations[i];
            rc.origin.x += octave * octaveWidth;

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
                    auto t = dc->translation();
                    dc->setTranslation(
                        makePoint(t.x + r.origin.x + r.size.width / 2.0f, t.y + r.origin.y + r.size.height / 2.0f));
                    dc->setRotation(90.0f);
                    dc->drawRightText(
                        _font,
                        makeRect(-r.size.height / 2.0f - 2.0f, -r.size.width / 2.0f, r.size.height, r.size.width),
                        it->second);
                }
                dc->popStates();
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
int KeyboardH::pitchAtPoint(Point pt) {
    int pitch = 0;

    Size whiteKeySize = _whiteKeyImage->size();
    Size blackKeySize = _blackKeyImage->size();

    int octave = pt.x / (whiteKeySize.width * 7);

    if (pt.y < whiteKeySize.height - blackKeySize.height) {
        // A white note has been pressed
        int whiteKey = (int)(pt.x) % (int)(whiteKeySize.width * 7) / (int)(whiteKeySize.width);
        pitch = octave * 12 + whiteKeyToPitchMapping[whiteKey];
    } else {
        // A black or white key has been pressed
        int key = (int)(pt.x) % (int)(whiteKeySize.width * 7) / (int)(blackKeySize.width);
        pitch = octave * 12 + key;
    }
    return pitch + 12;
}

// ---------------------------------------------------------------------------------------------------------------------
bool KeyboardH::handleEvent(const UIEvent* event) {
    // Handle the cursor during the selection
    if ((event->type == MOUSE_MOVED_UIEVENT) && isPointInRect(event->pt, clippedRect())) {
        responderChain()->setCursorInRect(this, Platform::PointingHandCursor, clippedRect());
    }

    if (!_isCaptured && isPointInRect(event->pt, resolvedClippedRect())) {
        if (event->type == MOUSE_DOWN_UIEVENT) {
            _isCaptured = responderChain()->captureResponder(this);
        } else if (event->type == SCROLL_UIEVENT) {
            if (_scrollFn) _scrollFn(this, event->deltaX);
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
Size KeyboardH::contentSize() {
    return makeSize(_blackKeyImage->size().width * 12.0f * 8.0f, _whiteKeyImage->size().height);
}
