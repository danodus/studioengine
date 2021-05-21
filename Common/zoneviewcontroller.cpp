//
//  zoneviewcontroller.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-02-08.
//  Copyright © 2016-2020 Daniel Cliche. All rights reserved.
//

#include "zoneviewcontroller.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
ZoneViewController::ZoneViewController(std::shared_ptr<ZoneView> view, MelobaseCore::StudioController* studioController, int zone) : _view(view), _studioController(studioController), _zone(zone)
{
    using namespace std::placeholders;
    
    _channelDidChangeFn = nullptr;
    
    _transposeValue = static_cast<int>(_view->transposeStepper()->value());
    _channel = _zone;
    
    _studioChannelViewController = new StudioChannelViewController(_view->studioChannelView(), _studioController, _zone);
    
    _view->transposeStepper()->setValueDidChangeFn(std::bind(&ZoneViewController::transposeValueDidChange, this, _1));
    _view->channelSegmentedControl()->setDidSelectSegmentFn(std::bind(&ZoneViewController::channelDidChange, this, _1, _2));

}

// ---------------------------------------------------------------------------------------------------------------------
void ZoneViewController::transposeValueDidChange(Stepper *sender)
{
    int value = static_cast<int>(sender->value());
    _view->transposeLabelView()->setTitle(value > 0 ? std::string("+") + std::to_string(value) : std::to_string(value));
    _transposeValue = static_cast<int>(_view->transposeStepper()->value());
}

// ---------------------------------------------------------------------------------------------------------------------
void ZoneViewController::channelDidChange(MDStudio::SegmentedControl *sender, int selectedSegment)
{
    _channel = selectedSegment;
    _studioChannelViewController->setChannel(_channel);
    
    if (_channelDidChangeFn)
        _channelDidChangeFn(this, _channel);
}
