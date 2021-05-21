//
//  preferencesviewcontroller.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-01-22.
//  Copyright (c) 2015-2019 Daniel Cliche. All rights reserved.
//

#ifndef PREFERENCESVIEWCONTROLLER_H
#define PREFERENCESVIEWCONTROLLER_H

#include "topview.h"

#include <ui.h>
#include <button.h>
#include <combobox.h>
#include <listitemview.h>

#include "modalviewcontroller.h"

#include <memory>

class PreferencesViewController : public ModalViewController {
    
public:
    
    typedef std::function<void(PreferencesViewController *sender)> DidChangeAudioOutputSettingsFnType;
    typedef std::function<void(PreferencesViewController *sender)> DidChangeAutoStopRecordSettingsFnType;
    typedef std::function<void(PreferencesViewController *sender)> DidChangeMIDIDestinationSettingsFnType;
    typedef std::function<void(PreferencesViewController *sender)> DidChangeNbVoicesSettingsFnType;

    typedef enum { NoneMIDIDestination, InternalSynthMIDIDestination, MIDIOutputMIDIDestination } MIDIDestinationEnum;
    
private:

    std::shared_ptr<MDStudio::ComboBox> _midiInputComboBox, _midiOutputComboBox;
    std::shared_ptr<MDStudio::ComboBox> _audioOutputComboBox;
    std::vector<std::string> _midiInputPortNames, _midiOutputPortNames;
    std::vector<std::pair<std::string, double>> _audioOutputDevices;
    std::string _midiInputPortName, _midiOutputPortName;
    std::string _audioOutputDeviceName;
    MIDIDestinationEnum _midiDestination;
    bool _isAutoStopRecordEnabled;
    float _autoStopRecordPeriod;
    unsigned int _nbVoices;
    
    double _audioOutputLatency;
    
    std::shared_ptr<MDStudio::Slider> _audioOutputLatencySlider;
    std::shared_ptr<MDStudio::LabelView> _audioOutputLatencyValueLabelView;

    std::shared_ptr<MDStudio::Button> _midiDestInternalSynthButton;
    std::shared_ptr<MDStudio::Button> _midiDestMIDIOutputButton;

    std::shared_ptr<MDStudio::LabelView> _nbVoicesLabelView;
    std::shared_ptr<MDStudio::Slider> _nbVoicesSlider;
    std::shared_ptr<MDStudio::LabelView> _nbVoicesValueLabelView;

    std::shared_ptr<MDStudio::Button> _autoStopEnableButton;
    std::shared_ptr<MDStudio::Slider> _autoStopPeriodSlider;
    std::shared_ptr<MDStudio::LabelView> _autoStopPeriodValueLabelView;
    
    void okButtonClicked(MDStudio::Button *sender);
    void autoStopEnableButtonStateDidChange(MDStudio::Button *sender, bool state);

    void midiDestInternalSynthButtonStateDidChange(MDStudio::Button *sender, bool state);
    void midiDestMIDIOutputButtonStateDidChange(MDStudio::Button *sender, bool state);

    
    unsigned int midiInputNbRows(MDStudio::ComboBox *sender);
    std::shared_ptr<MDStudio::ListItemView> midiInputViewForRow(MDStudio::ComboBox *sender, int row);
    void midiInputDidSelectRow(MDStudio::ComboBox *sender, int row);
    void midiInputDidHoverRow(MDStudio::ComboBox *sender, int row);

    unsigned int midiOutputNbRows(MDStudio::ComboBox *sender);
    std::shared_ptr<MDStudio::ListItemView> midiOutputViewForRow(MDStudio::ComboBox *sender, int row);
    void midiOutputDidSelectRow(MDStudio::ComboBox *sender, int row);
    void midiOutputDidHoverRow(MDStudio::ComboBox *sender, int row);
    
    unsigned int audioOutputNbRows(MDStudio::ComboBox *sender);
    std::shared_ptr<MDStudio::ListItemView> audioOutputViewForRow(MDStudio::ComboBox *sender, int row);
    void audioOutputDidSelectRow(MDStudio::ComboBox *sender, int row);
    void audioOutputDidHoverRow(MDStudio::ComboBox *sender, int row);
    void audioOutputLatencySliderPosChanged(MDStudio::Slider *sender, float value);
    void audioOutputLatencySliderPosSet(MDStudio::Slider *sender, float value);
    
    void autoStopSliderPosChanged(MDStudio::Slider *sender, float value);
    void nbVoicesSliderPosChanged(MDStudio::Slider *sender, float value);

    void updateNbVoicesVisibility();

    void saveStates();
    
    DidChangeAudioOutputSettingsFnType _didChangeAudioOutputSettingsFn;
    DidChangeAutoStopRecordSettingsFnType _didChangeAutoStopRecordSettingsFn;
    DidChangeMIDIDestinationSettingsFnType _didChangeMIDIDestinationSettingsFn;
    DidChangeNbVoicesSettingsFnType _didChangeNbVoicesSettingsFn;

public:

    void restoreStates();
    
    PreferencesViewController(TopView *topView, std::shared_ptr<MDStudio::View> view, std::string uiPath, std::string midiInputPortName, std::string midiOutputPortName, std::string audioOutputDeviceName, double audioOutputLatency);
    void setMIDIInputPortNames(std::vector<std::string> midiInputPortNames);
    void setMIDIOutputPortNames(std::vector<std::string> midiOutputPortNames);
    void setAudioOutputDevices(std::vector<std::pair<std::string, double>> audioOutputDevices);
    
    std::string midiInputPortName() { return _midiInputPortName; }
    std::string midiOutputPortName() { return _midiOutputPortName; }
    std::string audioOutputDeviceName() { return _audioOutputDeviceName; }
    double audioOutputLatency() { return _audioOutputLatency; }
    bool isAutoStopRecordEnabled() { return _isAutoStopRecordEnabled; }
    float autoStopRecordPeriod() { return _autoStopRecordPeriod; }
    MIDIDestinationEnum midiDestination() { return _midiDestination; }
    unsigned int nbVoices() { return _nbVoices; }
    
    void setDidChangeAudioOutputSettingsFn(DidChangeAudioOutputSettingsFnType didChangeAudioOutputSettingsFn) { _didChangeAudioOutputSettingsFn = didChangeAudioOutputSettingsFn; }
    void setDidChangeAutoStopRecordSettingsFn(DidChangeAutoStopRecordSettingsFnType didChangeAutoStopRecordSettingsFn) { _didChangeAutoStopRecordSettingsFn = didChangeAutoStopRecordSettingsFn; }
    void setDidChangeMIDIDestinationSettingsFn(DidChangeMIDIDestinationSettingsFnType didChangeMIDIDestinationSettingsFn) { _didChangeMIDIDestinationSettingsFn = didChangeMIDIDestinationSettingsFn; }
    void setDidChangeNbVoicesSettingsFn(DidChangeNbVoicesSettingsFnType didChangeNbVoicesSettingsFn) { _didChangeNbVoicesSettingsFn = didChangeNbVoicesSettingsFn; }
};

#endif // PREFERENCESVIEWCONTROLLER_H
