//
//  zoneviewcontroller.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-02-08.
//  Copyright © 2016-2020 Daniel Cliche. All rights reserved.
//

#ifndef ZONEVIEWCONTROLLER_H
#define ZONEVIEWCONTROLLER_H

#include "zoneview.h"
#include "studiochannelviewcontroller.h"

#include <studio.h>
#include <stepper.h>
#include <segmentedcontrol.h>

class ZoneViewController
{
public:
    typedef std::function<void(ZoneViewController *sender, int channel)> ChannelDidChangeFnType;
    
private:
    std::shared_ptr<ZoneView> _view;
    int _zone;
    StudioChannelViewController *_studioChannelViewController;
    MelobaseCore::StudioController *_studioController;
    std::atomic<int> _transposeValue;
    std::atomic<int> _channel;
    
    ChannelDidChangeFnType _channelDidChangeFn;
    
    void transposeValueDidChange(MDStudio::Stepper *sender);
    void channelDidChange(MDStudio::SegmentedControl *sender, int selectedSegment);

    
    
public:
    
    ZoneViewController(std::shared_ptr<ZoneView> view, MelobaseCore::StudioController* studioController, int zone);
    
    void setChannelDidChangeFn(ChannelDidChangeFnType channelDidChangeFn) { _channelDidChangeFn = channelDidChangeFn; }

    int transposeValue() { return _transposeValue; }
    int channel() { return _channel; }
};

#endif // ZONEVIEWCONTROLLER_H
