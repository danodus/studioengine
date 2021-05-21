//
//  studiochannelviewcontroller.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2014-07-21.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#include "studiochannelviewcontroller.h"
#include "listitemview.h"

#include <math.h>

#define STUDIO_CHANNEL_VIEW_CONTROLLER_MIN_LEVEL    -57.0f

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
StudioChannelViewController::StudioChannelViewController(std::shared_ptr<StudioChannelView> view, MelobaseCore::StudioController* studioController, int channel) : _view(view), _studioController(studioController), _channel(channel)
{
    _programComboBoxController = new ProgramComboBoxController(_view->instrumentComboBox().get(), _studioController->studio(), SystemFonts::sharedInstance()->semiboldFont());
    
    using namespace std::placeholders;
    
    _programComboBoxController->setDidSelectPresetFn(std::bind(&StudioChannelViewController::didSelectPreset, this, _1, _2));
    _programComboBoxController->setDidConfirmPresetSelectionFn(std::bind(&StudioChannelViewController::didSelectPreset, this, _1, _2));
    
    _view->levelSlider()->setPosChangedFn(std::bind(&StudioChannelViewController::levelValueDidChange, this, _1, _2));
    _view->balanceSlider()->setPosChangedFn(std::bind(&StudioChannelViewController::balanceValueDidChange, this, _1, _2));
    _view->reverbSlider()->setPosChangedFn(std::bind(&StudioChannelViewController::reverbValueDidChange, this, _1, _2));
    _view->chorusSlider()->setPosChangedFn(std::bind(&StudioChannelViewController::chorusValueDidChange, this, _1, _2));

    // Observe the instrument changes
    _didSetInstrumentFn = std::shared_ptr<MDStudio::Studio::didSetInstrumentFnType>(new MDStudio::Studio::didSetInstrumentFnType(std::bind(&StudioChannelViewController::didSetInstrument, this, _1, _2, _3, _4, _5, _6)));
    _studioController->studio()->addDidSetInstrumentFn(_didSetInstrumentFn);

    // Observe mixer state changes
    _didSetMixerLevelFn = std::shared_ptr<MDStudio::Studio::didSetMixerLevelFnType>(new MDStudio::Studio::didSetMixerLevelFnType(std::bind(&StudioChannelViewController::didSetMixerLevel, this, _1, _2, _3, _4, _5, _6)));
    _studioController->studio()->addDidSetMixerLevelFn(_didSetMixerLevelFn);
    _didSetMixerBalanceFn = std::shared_ptr<MDStudio::Studio::didSetMixerBalanceFnType>(new MDStudio::Studio::didSetMixerBalanceFnType(std::bind(&StudioChannelViewController::didSetMixerBalance, this, _1, _2, _3, _4, _5, _6)));
    _studioController->studio()->addDidSetMixerBalanceFn(_didSetMixerBalanceFn);

    // Observe controller changes
    _didSetControlValueFn = std::shared_ptr<MDStudio::Studio::didSetControlValueFnType>(new MDStudio::Studio::didSetControlValueFnType(std::bind(&StudioChannelViewController::didSetControlValue, this, _1, _2, _3, _4, _5, _6, _7)));
    _studioController->studio()->addDidSetControlValueFn(_didSetControlValueFn);
}

// ---------------------------------------------------------------------------------------------------------------------
StudioChannelViewController::~StudioChannelViewController()
{
    _studioController->studio()->removeDidSetInstrumentFn(_didSetInstrumentFn);
    _studioController->studio()->removeDidSetMixerLevelFn(_didSetMixerLevelFn);
    _studioController->studio()->removeDidSetMixerBalanceFn(_didSetMixerBalanceFn);
    _studioController->studio()->removeDidSetControlValueFn(_didSetControlValueFn);

    delete _programComboBoxController;
}

// ---------------------------------------------------------------------------------------------------------------------
void StudioChannelViewController::levelValueDidChange(Slider *sender, float pos)
{
    float linearLevel = pos <= STUDIO_CHANNEL_VIEW_CONTROLLER_MIN_LEVEL ? 0.0f : powf(10.0f, pos / 40.0f);
    _studioController->setMixerLevel(linearLevel, _channel);
}

// ---------------------------------------------------------------------------------------------------------------------
void StudioChannelViewController::balanceValueDidChange(Slider *sender, float pos)
{
    _studioController->setMixerBalance(pos, _channel);
}

// ---------------------------------------------------------------------------------------------------------------------
void StudioChannelViewController::reverbValueDidChange(Slider *sender, float pos)
{
    _studioController->setControlValue(STUDIO_CONTROL_REVERB, pos * 127.0f, _channel);
}

// ---------------------------------------------------------------------------------------------------------------------
void StudioChannelViewController::chorusValueDidChange(Slider *sender, float pos)
{
    _studioController->setControlValue(STUDIO_CONTROL_CHORUS, pos * 127.0f, _channel);
}

// ---------------------------------------------------------------------------------------------------------------------
void StudioChannelViewController::updateInstrumentState()
{
    const MDStudio::Preset *preset = _studioController->studio()->presetForInstrument(_studioController->studio()->instrument(_channel));
    _programComboBoxController->setPreset(preset);
}

// ---------------------------------------------------------------------------------------------------------------------
void StudioChannelViewController::updateMixerLevelState()
{
    float levelValue = _studioController->studio()->mixerLevel(_channel) == 0.0f ? STUDIO_CHANNEL_VIEW_CONTROLLER_MIN_LEVEL : 40.0f * log10f(_studioController->studio()->mixerLevel(_channel));
    if (_view->levelSlider()->pos() != levelValue)
        _view->levelSlider()->setPos(levelValue, false);
}

// ---------------------------------------------------------------------------------------------------------------------
void StudioChannelViewController::updateMixerBalanceState()
{
    auto balancePos = _studioController->studio()->mixerBalance(_channel);
    if (_view->balanceSlider()->pos() != balancePos)
        _view->balanceSlider()->setPos(balancePos, false);
}

// ---------------------------------------------------------------------------------------------------------------------
void StudioChannelViewController::updateReverbState()
{
    auto pos = _studioController->studio()->controlValue(_channel, STUDIO_CONTROL_REVERB) / 127.0f;
    if (_view->reverbSlider()->pos() != pos)
        _view->reverbSlider()->setPos(pos, false);
}
// ---------------------------------------------------------------------------------------------------------------------
void StudioChannelViewController::updateChorusState()
{
    auto pos = _studioController->studio()->controlValue(_channel, STUDIO_CONTROL_CHORUS) / 127.0f;
    if (_view->chorusSlider()->pos() != pos)
        _view->chorusSlider()->setPos(pos , false);
}

// ---------------------------------------------------------------------------------------------------------------------
void StudioChannelViewController::didSetInstrument(MDStudio::Studio* sender, int source, double timestamp, UInt32 tick, int channel, int instrument)
{
    if (channel == _channel) {
        updateInstrumentState();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void StudioChannelViewController::didSetMixerLevel(MDStudio::Studio* sender, int source, double timestamp, UInt32 tick, int channel, Float32 mixerLevel)
{
    if (channel == _channel) {
        updateMixerLevelState();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void StudioChannelViewController::didSetMixerBalance(MDStudio::Studio* sender, int source, double timestamp, UInt32 tick, int channel, Float32 mixerBalance)
{
    if (channel == _channel) {
        updateMixerBalanceState();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void StudioChannelViewController::didSetControlValue(MDStudio::Studio* sender, int source, double timestamp, UInt32 tick, int channel, UInt32 control, UInt32 value)
{
    if (channel == _channel) {
        if (control == STUDIO_CONTROL_REVERB) {
            updateReverbState();
        } else if (control == STUDIO_CONTROL_CHORUS) {
            updateChorusState();
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void StudioChannelViewController::didSelectPreset(ProgramComboBoxController *sender, MDStudio::Preset preset)
{
    _studioController->setInstrument(STUDIO_INSTRUMENT_FROM_PRESET(preset._bank, preset._number), _channel);
}

// ---------------------------------------------------------------------------------------------------------------------
void StudioChannelViewController::setChannel(int channel)
{
    _channel = channel;
    _view->setChannel(channel);
    updateInstrumentState();
    updateMixerLevelState();
    updateMixerBalanceState();
    updateReverbState();
    updateChorusState();
}
