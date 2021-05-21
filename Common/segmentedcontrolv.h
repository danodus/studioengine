//
//  segmentedcontrolv.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-07-11.
//  Copyright (c) 2015-2018 Daniel Cliche. All rights reserved.
//

#ifndef SEGMENTEDCONTROLV_H
#define SEGMENTEDCONTROLV_H

#include <control.h>
#include <button.h>

#include <vector>
#include <functional>
#include <memory>

class SegmentedControlV : public MDStudio::Control {
    
public:
    typedef std::function<void(SegmentedControlV *sender, int selectedSegment)> didSelectSegmentFnType;
    
private:
    
    std::vector<std::shared_ptr<MDStudio::Button>> _buttons;
    
    int _selectedSegment;
    
    didSelectSegmentFnType _didSelectSegmentFn;
    
    void buttonStateDidChange(MDStudio::Button *sender, bool state);
    
    bool _isEnabled;
    
public:
    SegmentedControlV(std::string name, void *owner, std::vector<std::string>strings, std::vector<std::string> tooltipStrings);
    ~SegmentedControlV();
    
    int selectedSegment() { return _selectedSegment; }
    void setSelectedSegment(int selectedSegment, bool isDelegateNotified = true);
    
    void setDidSelectSegment(didSelectSegmentFnType didSelectSegmentFn) { _didSelectSegmentFn = didSelectSegmentFn; }
    
    void setFrame(MDStudio::Rect aFrame) override;
    
    void setIsEnabled(bool isEnabled);
    bool isEnabled() { return _isEnabled; }
};



#endif // SEGMENTEDCONTROLV_H
