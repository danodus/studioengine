//
//  keyboardv.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2014-06-26.
//  Copyright (c) 2014-2015 Daniel Cliche. All rights reserved.
//

#ifndef KEYBOARDV_H
#define KEYBOARDV_H

#include "control.h"
#include "image.h"
#include <studio.h>

#include <font.h>

#include <memory>
#include <map>

class KeyboardV : public MDStudio::Control
{
    int _highlightedKey;

    std::function<void(KeyboardV* sender, int pitch)> _keyPressedFn;
    std::function<void(KeyboardV* sender, int pitch)> _keyReleasedFn;
    std::function<void(KeyboardV* sender, float deltaY)> _scrollFn;
    
    bool _keyStates[STUDIO_MAX_CHANNELS][96];
    
    std::shared_ptr<MDStudio::Image> _whiteKeyImage;
    std::shared_ptr<MDStudio::Image> _blackKeyImage;
    std::shared_ptr<MDStudio::Image> _whiteKeyPressedImage;
    std::shared_ptr<MDStudio::Image> _blackKeyPressedImage;
    
    MDStudio::MultiDPIFont *_font;
    
    MDStudio::Rect _blackKeyLocations[5];
    MDStudio::Rect _whiteKeyLocations[7];
    
    void calculateKeyLocations();
    int pitchAtPoint(MDStudio::Point pt);
    
    bool _isCaptured;
    
    bool _isDrumKit;
    
    int _currentChannel;

protected:
    bool handleEvent(const MDStudio::UIEvent* event) override;

public:
    KeyboardV(std::string name, void* owner);
    ~KeyboardV();

    void draw() override;

    void setKeyPressedFn(std::function<void(KeyboardV* sender, int pitch)> keyPressedFn) { _keyPressedFn = keyPressedFn; }
    void setKeyReleasedFn( std::function<void(KeyboardV* sender, int pitch)> keyReleasedFn) { _keyReleasedFn = keyReleasedFn; }
    void setScrollFn( std::function<void(KeyboardV* sender, float deltaY)> scrollFn) { _scrollFn = scrollFn; }
    
    void setKeyPressedState(int channel, int pitch, bool state);
    void resetKeyPressedStates();
    
    void setIsDrumKit(bool isDrumKit) { _isDrumKit = isDrumKit; setDirty(); }
    bool isDrumKit() { return _isDrumKit; }
    
    void setCurrentChannel(int currentChannel) { _currentChannel = currentChannel; setDirty(); }
    int currentChannel() { return _currentChannel; }
    
};

#endif // KEYBOARDV_H
