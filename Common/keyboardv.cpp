//
//  keyboardv.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2014-06-26.
//  Copyright (c) 2014-2019 Daniel Cliche. All rights reserved.
//

#include "keyboardv.h"

#include "draw.h"
#include "responderchain.h"

#include <platform.h>

using namespace MDStudio;

// Mapping white/black key index -> pitch in octave
const static int whiteKeyToPitchMapping[7] = {0, 2, 4, 5, 7, 9, 11};
const static int blackKeyToPitchMapping[5] = {1, 3, 6, 8, 10};

// Mapping pitch in octave -> white/black key index
//const static int pitchToWhiteKeyMapping[12] = {0, -1, 1, -1, 2, 3, -1, 4, -1, 5, -1, 6};
//const static int pitchToBlackKeyMapping[12] = {-1, 0, -1, 1, -1, -1, 2, -1, 3, -1, 4, -1};

// Drum kit labels (GM)
const static std::map<int, std::string> drumKitLabels = {
    {35, "Bass Drum 2"},
    {36, "Bass Drum 1"},
    {37, "Rimshot"},
    {38, "Snare Drum 1"},
    {39, "Hand Clap"},
    {40, "Snare Drum 2"},
    {41, "Low Tom 2"},
    {42, "Closed Hi-hat"},
    {43, "Low Tom 1"},
    {44, "Pedal Hi-hat"},
    {45, "Mid Tom 2"},
    {46, "Open Hi-hat"},
    {47, "Mid Tom 1"},
    {48, "High Tom 2"},
    {49, "Crash Cymbal 1"},
    {50, "High Tom 1"},
    {51, "Ride Cymbal 1"},
    {52, "Chinese Cymbal"},
    {53, "Ride Bell"},
    {54, "Tambourine"},
    {55, "Splash Cymbal"},
    {56, "Cowbell"},
    {57, "Crash Cymbal 2"},
    {58, "Vibra Slap"},
    {59, "Ride Cymbal 2"},
    {60, "High Bongo"},
    {61, "Low Bongo"},
    {62, "Mute High Conga"},
    {63, "Opn High Conga"},
    {64, "Low Conga"},
    {65, "High Timbale"},
    {66, "Low Timbale"},
    {67, "High Agogô"},
    {68, "Low Agogô"},
    {69, "Cabasa"},
    {70, "Maracas"},
    {71, "Short Whistle"},
    {72, "Long Whistle"},
    {73, "Short Güiro"},
    {74, "Long Güiro"},
    {75, "Claves"},
    {76, "High Wood Block"},
    {77, "Low Wood Block"},
    {78, "Mute Cuíca"},
    {79, "Open Cuíca"},
    {80, "Mute Triangle"},
    {81, "Open Triangle"}
};

const static float drumKitWhiteKeyLabelOffsets[7] = { -4.0f, 0.0f, 4.0f, -4.0f, -2.0f, 2.0f, 4.0f };

// Macros to find out if a pitch in octave is on a black or white key
#define MD_KEYBOARD_VIEW_IS_WHITE_KEY(_pitchInOctave_) (pitchToWhiteKeyMapping[_pitchInOctave_] >= 0)
#define MD_KEYBOARD_VIEW_IS_BLACK_KEY(_pitchInOctave_) (pitchToBlackKeyMapping[_pitchInOctave_] >= 0)

// ---------------------------------------------------------------------------------------------------------------------
KeyboardV::KeyboardV(std::string name, void* owner) : Control(name, owner)
{
    _keyPressedFn = nullptr;
    _keyReleasedFn = nullptr;
    _scrollFn = nullptr;
    _highlightedKey = -1;
    _isCaptured = false;
    _isDrumKit = false;
    _currentChannel = 0;
    
    memset(_keyStates, 0, sizeof(_keyStates));
    
    _whiteKeyImage = std::make_shared<Image>("WhiteKeyH@2x.png");
    _blackKeyImage = std::make_shared<Image>("BlackKeyH@2x.png");
    _whiteKeyPressedImage = std::make_shared<Image>("WhiteKeyPressedH@2x.png");
    _blackKeyPressedImage = std::make_shared<Image>("BlackKeyPressedH@2x.png");
    
    std::string path = Platform::sharedInstance()->resourcesPath() + "/OpenSans-Semibold.ttf";
    _font = new MultiDPIFont(7, path);
    
    calculateKeyLocations();
}

// ---------------------------------------------------------------------------------------------------------------------
KeyboardV::~KeyboardV()
{
    delete _font;
}

// ---------------------------------------------------------------------------------------------------------------------
void KeyboardV::calculateKeyLocations()
{
    Size whiteKeySize = _whiteKeyImage->size();
    Size blackKeySize = _blackKeyImage->size();
    
    // We calculate the locations of the white keys
    for (int i = 0; i < 7; i++) {
        _whiteKeyLocations[i] = makeRect(0, i * whiteKeySize.height, whiteKeySize.width, whiteKeySize.height);
    }
    
    // We calculate the locations of the black keys
    for (int i = 0; i < 5; i++)
        _blackKeyLocations[i] = makeRect(0, blackKeyToPitchMapping[i] * blackKeySize.height, blackKeySize.width, blackKeySize.height);
}


// ---------------------------------------------------------------------------------------------------------------------
void KeyboardV::draw()
{
    DrawContext *dc = drawContext();
    
    float octaveHeight = _blackKeyImage->size().height * 12.0f;
    
    // For each octave
    for (int octave = 0; octave < 8; octave++)
    {
        // We draw the white keys
        for (int i = 0; i < 7; i++)
        {
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
                    dc->drawCenteredText(SystemFonts::sharedInstance()->semiboldFont(), r, "C" + std::to_string(octave - 1));
                    dc->popStates();
                }
            }
        }
        
        // We draw the black keys
        for (int i = 0; i < 5; i++)
        {
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
int KeyboardV::pitchAtPoint(Point pt)
{
    int pitch = 0;
    
    Size whiteKeySize = _whiteKeyImage->size();
    Size blackKeySize = _blackKeyImage->size();
    
    int octave = pt.y / (whiteKeySize.height * 7);
    
    if (pt.x > blackKeySize.width)
    {
        // A white note has been pressed
        int whiteKey = (int)(pt.y) % (int)(whiteKeySize.height * 7) / (int)(whiteKeySize.height);
        pitch = octave * 12 + whiteKeyToPitchMapping[whiteKey];
    }
    else
    {
        // A black or white key has been pressed
        int key = (int)(pt.y) % (int)(whiteKeySize.height * 7) / (int)(blackKeySize.height);
        pitch = octave * 12 + key;
    }
    return pitch + 12;
}

// ---------------------------------------------------------------------------------------------------------------------
bool KeyboardV::handleEvent(const UIEvent* event)
{
    // Handle the cursor during the selection
    if ((event->type == MOUSE_MOVED_UIEVENT) && isPointInRect(event->pt, clippedRect())) {
        responderChain()->setCursorInRect(this, Platform::PointingHandCursor, clippedRect());
    }
    
    if (!_isCaptured && isPointInRect(event->pt, resolvedClippedRect())) {
        if (event->type == MOUSE_DOWN_UIEVENT) {
            _isCaptured = responderChain()->captureResponder(this);
        } else if (event->type == SCROLL_UIEVENT) {
            if (_scrollFn)
                _scrollFn(this, event->deltaY);
        }
    }
    
    if (_isCaptured && (event->type == MOUSE_DOWN_UIEVENT || event->type == MOUSE_UP_UIEVENT || event->type == MOUSE_MOVED_UIEVENT)) {
        Point pt = event->pt;
        pt.x = pt.x - rect().origin.x - offset().x;
        pt.y = pt.y - rect().origin.y - offset().y;
        int pitch = pitchAtPoint(pt);
        if (event->type == MOUSE_DOWN_UIEVENT || event->type == MOUSE_MOVED_UIEVENT) {
            if (!isPointInRect(event->pt, resolvedClippedRect()))
                pitch = -1;
            
            if (_highlightedKey != pitch) {
                if ((_highlightedKey >= 0) && (_keyReleasedFn != nullptr))
                    _keyReleasedFn(this, _highlightedKey);
                
                _highlightedKey = pitch;
                setDirty();
                if (pitch >= 0) {
                    if (_keyPressedFn != nullptr)
                        _keyPressedFn(this, pitch);
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
            
            if (_keyReleasedFn != nullptr)
                _keyReleasedFn(this, pitch);
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
void KeyboardV::setKeyPressedState(int channel, int pitch, bool state)
{
    if (pitch >= 12 && pitch < (96 + 12)) {
        _keyStates[channel][pitch - 12] = state;
        setDirty();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void KeyboardV::resetKeyPressedStates()
{
    memset(_keyStates, 0, sizeof(_keyStates));
    setDirty();
}
