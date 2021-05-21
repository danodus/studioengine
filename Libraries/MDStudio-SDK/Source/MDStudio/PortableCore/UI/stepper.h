//
//  stepper.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-07-21.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef STEPPER_H
#define STEPPER_H

#include <functional>
#include <memory>

#include "button.h"
#include "control.h"

namespace MDStudio {

class Stepper : public Control {
    std::shared_ptr<Button> _upButton, _downButton;
    float _delta;
    float _min, _max, _value;
    std::function<void(Stepper* sender)> _valueDidChangeFn;

   public:
    Stepper(const std::string& name, void* owner, float delta, float min, float max, float value);
    ~Stepper();
    float value() { return _value; }
    void setValue(float value);
    float delta() { return _delta; }

    void setFrame(Rect rect) override;

    void setValueDidChangeFn(std::function<void(Stepper* sender)> valueDidChangeFn) {
        _valueDidChangeFn = valueDidChangeFn;
    }
};

}  // namespace MDStudio

#endif  // STEPPER_H
