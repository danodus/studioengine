//
//  preferencesviewcontroller.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-01-22.
//  Copyright (c) 2015-2019 Daniel Cliche. All rights reserved.
//

#include "preferencesviewcontroller.h"

#include <platform.h>

#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>

// ---------------------------------------------------------------------------------------------------------------------
PreferencesViewController::PreferencesViewController(TopView *topView, std::shared_ptr<MDStudio::View> view, std::string uiPath, std::string midiInputPortName, std::string midiOutputPortName, std::string audioOutputDeviceName, double audioOutputLatency) : _midiInputPortName(midiInputPortName), _midiOutputPortName(midiOutputPortName), _audioOutputDeviceName(audioOutputDeviceName), _audioOutputLatency(audioOutputLatency), ModalViewController(topView, view, uiPath)
{
    using namespace std::placeholders;
    
    _didChangeAudioOutputSettingsFn = nullptr;
    _didChangeAutoStopRecordSettingsFn = nullptr;
    _didChangeMIDIDestinationSettingsFn = nullptr;
    _didChangeNbVoicesSettingsFn = nullptr;

    _midiDestination = InternalSynthMIDIDestination;
    _isAutoStopRecordEnabled = true;
    _autoStopRecordPeriod = 4.0f;
    _nbVoices = 64;

    
    //
    // MIDI input
    //
    
    _midiInputComboBox = std::dynamic_pointer_cast<MDStudio::ComboBox>(_ui->findView("midiInputComboBox"));
    _midiInputComboBox->setListContainerView(_view.get());
    _midiInputComboBox->setMaxHeight(150.0f);
    
    _midiInputComboBox->setNbRowsFn(std::bind(&PreferencesViewController::midiInputNbRows, this, _1));
    _midiInputComboBox->setViewForRowFn(std::bind(&PreferencesViewController::midiInputViewForRow, this, _1, _2));
    _midiInputComboBox->setDidSelectRowFn(std::bind(&PreferencesViewController::midiInputDidSelectRow, this, _1, _2));
    _midiInputComboBox->setDidHoverRowFn(std::bind(&PreferencesViewController::midiInputDidHoverRow, this, _1, _2));
    
    _midiInputComboBox->setTitle(_midiInputPortName.empty() ? "---" : _midiInputPortName);

    //
    // MIDI output
    //
    
    _midiOutputComboBox = std::dynamic_pointer_cast<MDStudio::ComboBox>(_ui->findView("midiOutputComboBox"));
    _midiOutputComboBox->setListContainerView(_view.get());
    _midiOutputComboBox->setMaxHeight(150.0f);
    
    _midiOutputComboBox->setNbRowsFn(std::bind(&PreferencesViewController::midiOutputNbRows, this, _1));
    _midiOutputComboBox->setViewForRowFn(std::bind(&PreferencesViewController::midiOutputViewForRow, this, _1, _2));
    _midiOutputComboBox->setDidSelectRowFn(std::bind(&PreferencesViewController::midiOutputDidSelectRow, this, _1, _2));
    _midiOutputComboBox->setDidHoverRowFn(std::bind(&PreferencesViewController::midiOutputDidHoverRow, this, _1, _2));
    
    _midiOutputComboBox->setTitle(_midiOutputPortName.empty() ? "---" : _midiOutputPortName);
    
    //
    // Audio output
    //
    
    _audioOutputComboBox = std::dynamic_pointer_cast<MDStudio::ComboBox>(_ui->findView("audioOutputComboBox"));
    _audioOutputComboBox->setListContainerView(_view.get());
    _audioOutputComboBox->setMaxHeight(150.0f);
    
    _audioOutputComboBox->setNbRowsFn(std::bind(&PreferencesViewController::audioOutputNbRows, this, _1));
    _audioOutputComboBox->setViewForRowFn(std::bind(&PreferencesViewController::audioOutputViewForRow, this, _1, _2));
    _audioOutputComboBox->setDidSelectRowFn(std::bind(&PreferencesViewController::audioOutputDidSelectRow, this, _1, _2));
    _audioOutputComboBox->setDidHoverRowFn(std::bind(&PreferencesViewController::audioOutputDidHoverRow, this, _1, _2));
    
    _audioOutputLatencySlider = std::dynamic_pointer_cast<MDStudio::Slider>(_ui->findView("audioOutputLatencySlider"));
    _audioOutputLatencySlider->setPosChangedFn(std::bind(&PreferencesViewController::audioOutputLatencySliderPosChanged, this, _1, _2));
    _audioOutputLatencySlider->setPosSetFn(std::bind(&PreferencesViewController::audioOutputLatencySliderPosSet, this, _1, _2));
    
    _audioOutputLatencyValueLabelView = std::dynamic_pointer_cast<MDStudio::LabelView>(_ui->findView("audioOutputLatencyValueLabelView"));
    
    _audioOutputComboBox->setTitle(_audioOutputDeviceName);
    _audioOutputLatencySlider->setPos(_audioOutputLatency);
    
    //
    // MIDI destination
    //
    
    _midiDestInternalSynthButton = std::dynamic_pointer_cast<MDStudio::Button>(_ui->findView("midiDestInternalSynthButton"));
    _midiDestInternalSynthButton->setStateDidChangeFn(std::bind(&PreferencesViewController::midiDestInternalSynthButtonStateDidChange, this, _1, _2));

    _midiDestMIDIOutputButton = std::dynamic_pointer_cast<MDStudio::Button>(_ui->findView("midiDestMIDIOutputButton"));
    _midiDestMIDIOutputButton->setStateDidChangeFn(std::bind(&PreferencesViewController::midiDestMIDIOutputButtonStateDidChange, this, _1, _2));

    _midiDestInternalSynthButton->setState(_midiDestination == InternalSynthMIDIDestination, false);
    _midiDestMIDIOutputButton->setState(_midiDestination == MIDIOutputMIDIDestination, false);
    
    //
    // Number of voices
    //
    
    _nbVoicesLabelView = std::dynamic_pointer_cast<MDStudio::LabelView>(_ui->findView("nbVoicesLabelView"));
    
    _nbVoicesSlider = std::dynamic_pointer_cast<MDStudio::Slider>(_ui->findView("nbVoicesSlider"));
    _nbVoicesSlider->setPosChangedFn(std::bind(&PreferencesViewController::nbVoicesSliderPosChanged, this, _1, _2));
    
    _nbVoicesValueLabelView = std::dynamic_pointer_cast<MDStudio::LabelView>(_ui->findView("nbVoicesValueLabelView"));
    _nbVoicesSlider->setPos(_nbVoices);

    
    //
    // Auto stop record
    //
    
    _autoStopEnableButton = std::dynamic_pointer_cast<MDStudio::Button>(_ui->findView("autoStopEnableButton"));
    _autoStopEnableButton->setStateDidChangeFn(std::bind(&PreferencesViewController::autoStopEnableButtonStateDidChange, this, _1, _2));
    _autoStopEnableButton->setState(_isAutoStopRecordEnabled);
    
    _autoStopPeriodSlider = std::dynamic_pointer_cast<MDStudio::Slider>(_ui->findView("autoStopPeriodSlider"));
    _autoStopPeriodSlider->setPosChangedFn(std::bind(&PreferencesViewController::autoStopSliderPosChanged, this, _1, _2));
    
    _autoStopPeriodValueLabelView = std::dynamic_pointer_cast<MDStudio::LabelView>(_ui->findView("autoStopPeriodValueLabelView"));
    _autoStopPeriodSlider->setPos(_autoStopRecordPeriod);
    
    //
    // OK button
    //
    
    std::shared_ptr<MDStudio::Button> okButton = std::dynamic_pointer_cast<MDStudio::Button>(_ui->findView("okButton"));
    okButton->setType(MDStudio::Button::OKButtonType);
    okButton->setClickedFn(std::bind(&PreferencesViewController::okButtonClicked, this, _1));
}

// ---------------------------------------------------------------------------------------------------------------------
void PreferencesViewController::updateNbVoicesVisibility()
{
    _nbVoicesLabelView->setIsVisible(_midiDestination == InternalSynthMIDIDestination);
    _nbVoicesSlider->setIsVisible(_midiDestination == InternalSynthMIDIDestination);
    _nbVoicesValueLabelView->setIsVisible(_midiDestination == InternalSynthMIDIDestination);
}

// ---------------------------------------------------------------------------------------------------------------------
void PreferencesViewController::restoreStates()
{
    std::string path = MDStudio::Platform::sharedInstance()->dataPath() + "/Preferences.ini";

    std::ifstream inFile(path);
    if (!inFile.is_open())
        return;
    
    std::string line;
    while (std::getline(inFile, line)) {
        size_t divPos = line.find_first_of("=");
        if (divPos != std::string::npos) {
            std::string parameter = line.substr(0, divPos);
            std::string value = line.substr(divPos + 1, std::string::npos);
            
            if (parameter == "midiInputPortName") {
                _midiInputPortName = value;
                _midiInputComboBox->setTitle(_midiInputPortName.empty() ? "---" : _midiInputPortName);
            } else if (parameter == "midiOutputPortName") {
                _midiOutputPortName = value;
                _midiOutputComboBox->setTitle(_midiOutputPortName.empty() ? "---" : _midiOutputPortName);
            } else if (parameter == "audioOutputDeviceName") {
                _audioOutputDeviceName = value;
                _audioOutputComboBox->setTitle(_audioOutputDeviceName);
            } else if (parameter == "audioOutputLatency") {
                _audioOutputLatency = std::atof(value.c_str());
                _audioOutputLatencySlider->setPos(_audioOutputLatency);
            } else if (parameter == "midiDestination") {
                if (value == "InternalSynthMIDIDestination") {
                    _midiDestination = InternalSynthMIDIDestination;
                } else if (value == "MIDIOutputMIDIDestination") {
                    _midiDestination = MIDIOutputMIDIDestination;
                } else {
                    _midiDestination = NoneMIDIDestination;
                }
                _midiDestInternalSynthButton->setState(_midiDestination == InternalSynthMIDIDestination);
                _midiDestMIDIOutputButton->setState(_midiDestination == MIDIOutputMIDIDestination);
                
                updateNbVoicesVisibility();
            } else if (parameter == "isAutoStopRecordEnabled") {
                _isAutoStopRecordEnabled = value == "true" ? true : false;
                _autoStopEnableButton->setState(_isAutoStopRecordEnabled);
            } else if (parameter == "autoStopRecordPeriod") {
                _autoStopRecordPeriod = std::atof(value.c_str());
                _autoStopPeriodSlider->setPos(_autoStopRecordPeriod);
            } else if (parameter == "nbVoices") {
                _nbVoices = std::atoi(value.c_str());
                _nbVoicesSlider->setPos(_nbVoices);
            }

        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PreferencesViewController::saveStates()
{
    std::string path = MDStudio::Platform::sharedInstance()->dataPath() + "/Preferences.ini";
    
    std::ofstream outFile(path);
    if (!outFile.is_open())
        return;
    
    outFile << "midiInputPortName=" << _midiInputPortName << std::endl;
    outFile << "midiOutputPortName=" << _midiOutputPortName << std::endl;
    outFile << "audioOutputDeviceName=" << _audioOutputDeviceName << std::endl;
    outFile << "audioOutputLatency=" << _audioOutputLatency << std::endl;
    std::string midiDest = (_midiDestination == InternalSynthMIDIDestination) ? "InternalSynthMIDIDestination" :
        (_midiDestination == MIDIOutputMIDIDestination) ? "MIDIOutputMIDIDestination" : "";
    outFile << "midiDestination=" << midiDest << std::endl;
    outFile << "isAutoStopRecordEnabled=" << (_isAutoStopRecordEnabled ? std::string("true") : std::string("false")) << std::endl;
    outFile << "autoStopRecordPeriod=" << _autoStopRecordPeriod << std::endl;
    outFile << "nbVoices=" << _nbVoices << std::endl;
    outFile.close();
}

// ---------------------------------------------------------------------------------------------------------------------
void PreferencesViewController::okButtonClicked(MDStudio::Button *sender)
{
    _midiInputComboBox->close();
    
    ModalViewController::dismiss();
    
    saveStates();
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int PreferencesViewController::midiInputNbRows(MDStudio::ComboBox *sender)
{
    return static_cast<unsigned int>(_midiInputPortNames.size() + 1);
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MDStudio::ListItemView> PreferencesViewController::midiInputViewForRow(MDStudio::ComboBox *sender, int row)
{
    return std::shared_ptr<MDStudio::ListItemView>(new MDStudio::ListItemView("listItemView" + std::to_string(row), this, (row == 0) ? "---" : _midiInputPortNames[row - 1]));
}

// ---------------------------------------------------------------------------------------------------------------------
void PreferencesViewController::midiInputDidSelectRow(MDStudio::ComboBox *sender, int row)
{
    sender->setSelectedRow(-1, false);
    _midiInputComboBox->close();
    
    _midiInputPortName = (row == 0) ? "" : _midiInputPortNames[row - 1];
    _midiInputComboBox->setTitle(_midiInputPortName.empty() ? "---" : _midiInputPortName);
}

// ---------------------------------------------------------------------------------------------------------------------
void PreferencesViewController::midiInputDidHoverRow(MDStudio::ComboBox *sender, int row)
{
    for (unsigned int i = 0; i < sender->rowViews().size(); ++i) {
        std::shared_ptr<MDStudio::View> view = sender->rowViews()[i];
        std::shared_ptr<MDStudio::ListItemView> listItemView = (std::static_pointer_cast<MDStudio::ListItemView>)(view);
        listItemView->setIsHovering(i == row);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PreferencesViewController::setMIDIInputPortNames(std::vector<std::string> midiInputPortNames)
{
    _midiInputComboBox->close();
    _midiInputPortNames = midiInputPortNames;
    _midiInputComboBox->reload();
    
    if (std::find(_midiInputPortNames.begin(), _midiInputPortNames.end(), _midiInputPortName) == _midiInputPortNames.end()) {
        _midiInputPortName = "";
        _midiInputComboBox->setTitle("---");
    }
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int PreferencesViewController::midiOutputNbRows(MDStudio::ComboBox *sender)
{
    return static_cast<unsigned int>(_midiOutputPortNames.size()) + 1;
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MDStudio::ListItemView> PreferencesViewController::midiOutputViewForRow(MDStudio::ComboBox *sender, int row)
{
    return std::shared_ptr<MDStudio::ListItemView>(new MDStudio::ListItemView("listItemView" + std::to_string(row), this, (row == 0) ? "---" : _midiOutputPortNames[row - 1]));
}

// ---------------------------------------------------------------------------------------------------------------------
void PreferencesViewController::midiOutputDidSelectRow(MDStudio::ComboBox *sender, int row)
{
    sender->setSelectedRow(-1, false);
    _midiOutputComboBox->close();
    
    _midiOutputPortName = (row == 0) ? "" : _midiOutputPortNames[row - 1];
    _midiOutputComboBox->setTitle((row == 0) ? "---" : _midiOutputPortName);
}

// ---------------------------------------------------------------------------------------------------------------------
void PreferencesViewController::midiOutputDidHoverRow(MDStudio::ComboBox *sender, int row)
{
    for (unsigned int i = 0; i < sender->rowViews().size(); ++i) {
        std::shared_ptr<MDStudio::View> view = sender->rowViews()[i];
        std::shared_ptr<MDStudio::ListItemView> listItemView = (std::static_pointer_cast<MDStudio::ListItemView>)(view);
        listItemView->setIsHovering(i == row);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PreferencesViewController::setMIDIOutputPortNames(std::vector<std::string> midiOutputPortNames)
{
    _midiOutputComboBox->close();
    _midiOutputPortNames = midiOutputPortNames;
    _midiOutputComboBox->reload();
    
    if (std::find(_midiOutputPortNames.begin(), _midiOutputPortNames.end(), _midiOutputPortName) == _midiOutputPortNames.end()) {
        _midiOutputPortName = "";
        _midiOutputComboBox->setTitle("---");
    }
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int PreferencesViewController::audioOutputNbRows(MDStudio::ComboBox *sender)
{
    return static_cast<unsigned int>(_audioOutputDevices.size());
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MDStudio::ListItemView> PreferencesViewController::audioOutputViewForRow(MDStudio::ComboBox *sender, int row)
{
    return std::shared_ptr<MDStudio::ListItemView>(new MDStudio::ListItemView("listItemView" + std::to_string(row), this, _audioOutputDevices[row].first));
}

// ---------------------------------------------------------------------------------------------------------------------
void PreferencesViewController::audioOutputDidSelectRow(MDStudio::ComboBox *sender, int row)
{
    sender->setSelectedRow(-1, false);
    _audioOutputComboBox->close();
    
    _audioOutputDeviceName = _audioOutputDevices[row].first;
    _audioOutputComboBox->setTitle(_audioOutputDeviceName);
    _audioOutputLatencySlider->setPos(_audioOutputDevices[row].second);
    
    if (_didChangeAudioOutputSettingsFn)
        _didChangeAudioOutputSettingsFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PreferencesViewController::audioOutputDidHoverRow(MDStudio::ComboBox *sender, int row)
{
    for (unsigned int i = 0; i < sender->rowViews().size(); ++i) {
        std::shared_ptr<MDStudio::View> view = sender->rowViews()[i];
        std::shared_ptr<MDStudio::ListItemView> listItemView = (std::static_pointer_cast<MDStudio::ListItemView>)(view);
        listItemView->setIsHovering(i == row);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PreferencesViewController::setAudioOutputDevices(std::vector<std::pair<std::string, double>> audioOutputDevices)
{
    _audioOutputDevices = audioOutputDevices;
    _audioOutputComboBox->reload();
}

// ---------------------------------------------------------------------------------------------------------------------
void PreferencesViewController::audioOutputLatencySliderPosChanged(MDStudio::Slider *sender, float value)
{
    _audioOutputLatency = static_cast<double>(value);
    
    std::stringstream ss;
    ss << std::setprecision(3) << value * 1000 << " ms";
    _audioOutputLatencyValueLabelView->setTitle(ss.str());
}

// ---------------------------------------------------------------------------------------------------------------------
void PreferencesViewController::audioOutputLatencySliderPosSet(MDStudio::Slider *sender, float value)
{
    if (_didChangeAudioOutputSettingsFn)
        _didChangeAudioOutputSettingsFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PreferencesViewController::autoStopEnableButtonStateDidChange(MDStudio::Button *sender, bool state)
{
    _isAutoStopRecordEnabled = state;
    
    if (_didChangeAutoStopRecordSettingsFn)
        _didChangeAutoStopRecordSettingsFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PreferencesViewController::autoStopSliderPosChanged(MDStudio::Slider *sender, float value)
{
    _autoStopRecordPeriod = value;
    
    std::stringstream ss;
    ss << std::setprecision(2) << value << " s";
    _autoStopPeriodValueLabelView->setTitle(ss.str());
    
    if (_didChangeAutoStopRecordSettingsFn)
        _didChangeAutoStopRecordSettingsFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PreferencesViewController::nbVoicesSliderPosChanged(MDStudio::Slider *sender, float value)
{
    _nbVoices = value;
    
    std::stringstream ss;
    ss << _nbVoices;
    _nbVoicesValueLabelView->setTitle(ss.str());
    
    if (_didChangeNbVoicesSettingsFn)
        _didChangeNbVoicesSettingsFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PreferencesViewController::midiDestInternalSynthButtonStateDidChange(MDStudio::Button *sender, bool state)
{
    if (!state)
        return;
    
    _midiDestMIDIOutputButton->setState(false, false);
    _midiDestination = InternalSynthMIDIDestination;
    
    updateNbVoicesVisibility();

    if (_didChangeMIDIDestinationSettingsFn)
        _didChangeMIDIDestinationSettingsFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PreferencesViewController::midiDestMIDIOutputButtonStateDidChange(MDStudio::Button *sender, bool state)
{
    if (!state)
        return;
    
    _midiDestInternalSynthButton->setState(false, false);
    _midiDestination = MIDIOutputMIDIDestination;
    
    updateNbVoicesVisibility();
    
    if (_didChangeMIDIDestinationSettingsFn)
        _didChangeMIDIDestinationSettingsFn(this);
}
