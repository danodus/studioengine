//
//  keyboardh.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-26.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#ifndef KEYBOARDH_H
#define KEYBOARDH_H

#include "keyboard.h"

namespace MDStudio {

class KeyboardH : public Keyboard {
    void calculateKeyLocations();
    bool handleEvent(const UIEvent* event) override;
    int pitchAtPoint(Point pt);
    void draw() override;

   public:
    KeyboardH(std::string name, void* owner);
    ~KeyboardH();

    Size contentSize() override;
};

}  // namespace MDStudio

#endif  // KEYBOARDH_H
