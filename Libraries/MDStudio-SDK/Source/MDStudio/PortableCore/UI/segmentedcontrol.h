//
//  segmentedcontrol.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-09-15.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef SEGMENTEDCONTROL_H
#define SEGMENTEDCONTROL_H

#include <functional>
#include <memory>
#include <vector>

#include "any.h"
#include "button.h"
#include "control.h"
#include "image.h"

namespace MDStudio {

class SegmentedControl : public Control {
   public:
    typedef std::function<void(SegmentedControl* sender, int selectedSegment)> didSelectSegmentFnType;

   private:
    std::vector<std::shared_ptr<Button>> _buttons;

    int _selectedSegment;

    didSelectSegmentFnType _didSelectSegmentFn;

    void buttonStateDidChange(Button* sender, bool state);

    bool _isEnabled;
    Color _borderColor, _textColor, _highlightColor;

   public:
    SegmentedControl(const std::string& name, void* owner, std::vector<Any> items);
    ~SegmentedControl();

    void setItems(std::vector<Any> items);

    void setFont(MultiDPIFont* font);

    int selectedSegment() { return _selectedSegment; }
    void setSelectedSegment(int selectedSegment, bool isDelegateNotified = true);

    void setDidSelectSegmentFn(didSelectSegmentFnType didSelectSegmentFn) { _didSelectSegmentFn = didSelectSegmentFn; }

    void setFrame(Rect aFrame) override;

    void setIsEnabled(bool isEnabled);
    bool isEnabled() { return _isEnabled; }

    Color borderColor() { return _borderColor; }
    void setBorderColor(Color borderColor);

    Color textColor() { return _textColor; }
    void setTextColor(Color textColor);

    Color highlightColor() { return _highlightColor; }
    void setHighlightColor(Color highlightColor);
};

}  // namespace MDStudio

#endif  // SEGMENTEDCONTROL_H
