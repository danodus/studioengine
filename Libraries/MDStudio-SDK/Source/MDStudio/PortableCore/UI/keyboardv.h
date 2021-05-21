//
//  keyboardv.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-26.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#ifndef KEYBOARDV_H
#define KEYBOARDV_H

#include "keyboard.h"

namespace MDStudio {

class KeyboardV : public Keyboard {
    void calculateKeyLocations();
    bool handleEvent(const UIEvent* event) override;
    int pitchAtPoint(Point pt);
    void draw() override;

   public:
    KeyboardV(std::string name, void* owner);
    ~KeyboardV();
    Size contentSize() override;
};

}  // namespace MDStudio

#endif  // KEYBOARDV_H
