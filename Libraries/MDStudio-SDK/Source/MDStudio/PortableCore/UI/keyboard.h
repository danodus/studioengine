//
//  keyboard.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-26.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <font.h>
#include <studio.h>

#include <map>
#include <memory>

#include "control.h"
#include "image.h"

namespace MDStudio {

class Keyboard : public MDStudio::Control {
   protected:
    const static int whiteKeyToPitchMapping[7];
    const static int blackKeyToPitchMapping[5];
    const static std::map<int, std::string> drumKitLabels;
    const static float drumKitWhiteKeyLabelOffsets[7];

    int _highlightedKey;

    std::function<void(Keyboard* sender, int pitch)> _keyPressedFn;
    std::function<void(Keyboard* sender, int pitch)> _keyReleasedFn;
    std::function<void(Keyboard* sender, float deltaY)> _scrollFn;

    bool _keyStates[STUDIO_MAX_CHANNELS][96];

    std::shared_ptr<MDStudio::Image> _whiteKeyImage;
    std::shared_ptr<MDStudio::Image> _blackKeyImage;
    std::shared_ptr<MDStudio::Image> _whiteKeyPressedImage;
    std::shared_ptr<MDStudio::Image> _blackKeyPressedImage;

    MDStudio::MultiDPIFont* _font;

    MDStudio::Rect _blackKeyLocations[5];
    MDStudio::Rect _whiteKeyLocations[7];

    bool _isCaptured;

    bool _isDrumKit;

    int _currentChannel;

    virtual bool handleEvent(const MDStudio::UIEvent* event) = 0;

   public:
    Keyboard(std::string name, void* owner);
    virtual ~Keyboard();

    virtual void draw() = 0;

    void setKeyPressedFn(std::function<void(Keyboard* sender, int pitch)> keyPressedFn) {
        _keyPressedFn = keyPressedFn;
    }
    void setKeyReleasedFn(std::function<void(Keyboard* sender, int pitch)> keyReleasedFn) {
        _keyReleasedFn = keyReleasedFn;
    }
    void setScrollFn(std::function<void(Keyboard* sender, float deltaY)> scrollFn) { _scrollFn = scrollFn; }

    void setKeyPressedState(int channel, int pitch, bool state);
    void resetKeyPressedStates();

    void setIsDrumKit(bool isDrumKit) {
        _isDrumKit = isDrumKit;
        setDirty();
    }
    bool isDrumKit() { return _isDrumKit; }

    void setCurrentChannel(int currentChannel) {
        _currentChannel = currentChannel;
        setDirty();
    }
    int currentChannel() { return _currentChannel; }

    virtual Size contentSize() = 0;
};

}  // namespace MDStudio

#endif  // KEYBOARD_H
