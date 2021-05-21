//
//  stepper.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-07-21.
//  Copyright (c) 2014 Daniel Cliche. All rights reserved.
//

#include "stepper.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
void upButtonClicked(Button* sender) {
    Stepper* stepper = static_cast<Stepper*>(sender->owner());
    stepper->setValue(stepper->value() + stepper->delta());
}

// ---------------------------------------------------------------------------------------------------------------------
void downButtonClicked(Button* sender) {
    Stepper* stepper = static_cast<Stepper*>(sender->owner());
    stepper->setValue(stepper->value() - stepper->delta());
}

// ---------------------------------------------------------------------------------------------------------------------
Stepper::Stepper(const std::string& name, void* owner, float delta, float min, float max, float value)
    : Control(name, owner), _delta(delta), _min(min), _max(max), _value(value) {
    _valueDidChangeFn = nullptr;

    // Add up button
    _upButton =
        std::shared_ptr<Button>(new Button("upButton", this, "", SystemImages::sharedInstance()->upArrowImage()));
    _upButton->setClickedFn(upButtonClicked);
    addSubview(_upButton);

    // Add down button
    _downButton =
        std::shared_ptr<Button>(new Button("downButton", this, "", SystemImages::sharedInstance()->downArrowImage()));
    _downButton->setClickedFn(downButtonClicked);
    addSubview(_downButton);
}

// ---------------------------------------------------------------------------------------------------------------------
Stepper::~Stepper() {
    removeSubview(_upButton);
    removeSubview(_downButton);
}

// ---------------------------------------------------------------------------------------------------------------------
void Stepper::setFrame(Rect aRect) {
    Control::setFrame(aRect);

    Rect r = makeRect(0.0f, rect().size.height / 2.0f, rect().size.width, rect().size.height / 2.0f);
    _upButton->setFrame(r);

    r = makeRect(0.0f, 0.0f, rect().size.width, rect().size.height / 2.0f + 1.0f);
    _downButton->setFrame(r);
}

// ---------------------------------------------------------------------------------------------------------------------
void Stepper::setValue(float value) {
    if (value < _min) {
        value = _min;
    } else if (value > _max) {
        value = _max;
    }

    _value = value;
    if (_valueDidChangeFn != nullptr) _valueDidChangeFn(this);
}
