//
//  studiochannelviewcontroller.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2014-07-21.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef STUDIOCHANNELVIEWCONTROLLER_H
#define STUDIOCHANNELVIEWCONTROLLER_H

#include "studiochannelview.h"

#include "programcomboboxcontroller.h"

#include <studiocontroller.h>
#include <memory>
#include <functional>
#include <vector>
#include <string>

class StudioChannelViewController
{
    std::shared_ptr<StudioChannelView> _view;
    MelobaseCore::StudioController* _studioController;
    int _channel;
    
    ProgramComboBoxController *_programComboBoxController;

    std::shared_ptr<MDStudio::Studio::didSetInstrumentFnType> _didSetInstrumentFn;
    std::shared_ptr<MDStudio::Studio::didSetMixerLevelFnType> _didSetMixerLevelFn;
    std::shared_ptr<MDStudio::Studio::didSetMixerBalanceFnType> _didSetMixerBalanceFn;
    std::shared_ptr<MDStudio::Studio::didSetControlValueFnType> _didSetControlValueFn;

    void levelValueDidChange(MDStudio::Slider *sender, float pos);
    void balanceValueDidChange(MDStudio::Slider *sender, float pos);
    void reverbValueDidChange(MDStudio::Slider *sender, float pos);
    void chorusValueDidChange(MDStudio::Slider *sender, float pos);
    
    void didSetInstrument(MDStudio::Studio* sender, int source, double timestamp, UInt32 tick, int channel, int instrument);
    void didSetMixerLevel(MDStudio::Studio* sender, int source, double timestamp, UInt32 tick, int channel, Float32 mixerLevel);
    void didSetMixerBalance(MDStudio::Studio* sender, int source, double timestamp, UInt32 tick, int channel, Float32 mixerBalance);
    void didSetControlValue(MDStudio::Studio* sender, int source, double timestamp, UInt32 tick, int channel, UInt32 control, UInt32 value);
    
    void updateInstrumentState();
    void updateMixerLevelState();
    void updateMixerBalanceState();
    void updateReverbState();
    void updateChorusState();
    
    void didSelectPreset(ProgramComboBoxController *sender, MDStudio::Preset preset);

public:
    StudioChannelViewController(std::shared_ptr<StudioChannelView> view, MelobaseCore::StudioController* studioController, int channel);
    ~StudioChannelViewController();

    std::shared_ptr<StudioChannelView> view() { return _view; }
    MelobaseCore::StudioController* studioController() { return _studioController; }
    
    void setChannel(int channel);
    int channel() { return _channel; }
};

#endif // STUDIOCHANNELVIEWCONTROLLER_H
