//
//  segmentedcontrolv.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-07-11.
//  Copyright (c) 2015-2018 Daniel Cliche. All rights reserved.
//

#include "segmentedcontrolv.h"

#include <draw.h>
#include <math.h>

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
SegmentedControlV::SegmentedControlV(std::string name, void *owner, std::vector<std::string> strings, std::vector<std::string> tooltipStrings) : Control(name, owner)
{
    _selectedSegment = -1;
    _didSelectSegmentFn = nullptr;
    _isEnabled = true;
    
    int i = 0;
    for (auto string : strings) {
        std::shared_ptr<Button> button = std::shared_ptr<Button>(new Button(name + std::to_string(i), this, string, nullptr));
        button->setType(Button::SegmentedControlButtonType);
        button->setFont(SystemFonts::sharedInstance()->semiboldFontSmall());
        _buttons.push_back(button);
        addSubview(button);
        button->setTooltipText(tooltipStrings[i]);
        using namespace std::placeholders;
        button->setStateDidChangeFn(std::bind(&SegmentedControlV::buttonStateDidChange, this, _1, _2));
        ++i;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
SegmentedControlV::~SegmentedControlV()
{
    for (auto button : _buttons) {
        removeSubview(button);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void SegmentedControlV::setSelectedSegment(int selectedSegment, bool isDelegateNotified)
{
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
void SegmentedControlV::setFrame(Rect aFrame)
{
    Control::setFrame(aFrame);
    float height = roundf(frame().size.height / _buttons.size());
    float y = frame().size.height;
    for (auto button : _buttons) {
        Rect f = makeRect(0.0f, y - height, frame().size.width, height);
        button->setFrame(f);
        y -= height - 1.0f;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void SegmentedControlV::buttonStateDidChange(Button *sender, bool state)
{
    int i = 0;
    for (auto button : _buttons) {
        if (sender == button.get())
            break;
        ++i;
    }
    setSelectedSegment(i);
}

// ---------------------------------------------------------------------------------------------------------------------
void SegmentedControlV::setIsEnabled(bool isEnabled)
{
    _isEnabled = isEnabled;
    
    for (auto button : _buttons) {
        button->setIsEnabled(isEnabled);
    }
}
