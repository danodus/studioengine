//
//  segmentedcontrol.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-09-15.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#include "segmentedcontrol.h"

#include <math.h>

#include "draw.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
SegmentedControl::SegmentedControl(const std::string& name, void* owner, std::vector<Any> items)
    : Control(name, owner) {
    _selectedSegment = -1;
    _didSelectSegmentFn = nullptr;
    _isEnabled = true;
    _borderColor = systemButtonBorderColor;
    _textColor = whiteColor;
    _highlightColor = blueColor;

    setItems(items);
}

// ---------------------------------------------------------------------------------------------------------------------
void SegmentedControl::setItems(std::vector<Any> items) {
    removeAllSubviews();

    int i = 0;
    for (auto item : items) {
        std::shared_ptr<Image> image;
        std::string title = "";
        if (item.is<shared_ptr<Image>>()) {
            image = item.as<std::shared_ptr<Image>>();
        } else if (item.is<std::string>()) {
            title = item.as<std::string>();
        }

        std::shared_ptr<Button> button =
            std::shared_ptr<Button>(new Button(name() + std::to_string(i), this, title, image));
        button->setType(Button::SegmentedControlButtonType);
        button->setBorderColor(_borderColor);
        button->setTextColor(_textColor);
        _buttons.push_back(button);
        addSubview(button);
        using namespace std::placeholders;
        button->setStateDidChangeFn(std::bind(&SegmentedControl::buttonStateDidChange, this, _1, _2));
        ++i;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
SegmentedControl::~SegmentedControl() {
    for (auto button : _buttons) {
        removeSubview(button);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void SegmentedControl::setSelectedSegment(int selectedSegment, bool isDelegateNotified) {
    if (_selectedSegment != selectedSegment) {
        _selectedSegment = selectedSegment;

        int i = 0;
        for (auto button : _buttons) {
            button->setState(i == selectedSegment, false);
            ++i;
        }

        if (isDelegateNotified && _didSelectSegmentFn) {
            _didSelectSegmentFn(this, selectedSegment);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void SegmentedControl::setFrame(Rect aFrame) {
    Control::setFrame(aFrame);
    float width = roundf(frame().size.width / _buttons.size());
    float x = 0.0f;
    for (auto button : _buttons) {
        Rect f = makeRect(x, 0.0f, width, frame().size.height);
        button->setFrame(f);
        x += width - 1.0f;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void SegmentedControl::buttonStateDidChange(Button* sender, bool state) {
    int i = 0;
    for (auto button : _buttons) {
        if (sender == button.get()) break;
        ++i;
    }
    setSelectedSegment(i);
}

// ---------------------------------------------------------------------------------------------------------------------
void SegmentedControl::setIsEnabled(bool isEnabled) {
    _isEnabled = isEnabled;

    for (auto button : _buttons) {
        button->setIsEnabled(isEnabled);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void SegmentedControl::setFont(MultiDPIFont* font) {
    for (auto button : _buttons) button->setFont(font);
}

// ---------------------------------------------------------------------------------------------------------------------
void SegmentedControl::setBorderColor(Color borderColor) {
    _borderColor = borderColor;
    for (auto button : _buttons) button->setBorderColor(borderColor);
}

// ---------------------------------------------------------------------------------------------------------------------
void SegmentedControl::setTextColor(Color textColor) {
    _textColor = textColor;
    for (auto button : _buttons) button->setTextColor(textColor);
}

// ---------------------------------------------------------------------------------------------------------------------
void SegmentedControl::setHighlightColor(Color highlightColor) {
    _highlightColor = highlightColor;
    for (auto button : _buttons) button->setHighlightColor(highlightColor);
}
