//
//  pianorollpropertiesviewcontroller.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-06-29.
//  Copyright (c) 2015-2021 Daniel Cliche. All rights reserved.
//

#include "pianorollpropertiesviewcontroller.h"
#include "helpers.h"

#include <listitemview.h>

#include <iomanip>
#include <sstream>
#include <algorithm>

// ---------------------------------------------------------------------------------------------------------------------
PianoRollPropertiesViewController::PianoRollPropertiesViewController(std::shared_ptr<PianoRollPropertiesView> view, MDStudio::Studio *studio) : _view(view), _studio(studio)
{
    _channelDidChangeFn = nullptr;
    _velocityDidChangeFn = nullptr;
    _programDidChangeFn = nullptr;
    _timeSignatureDidChangeFn = nullptr;
    _sysexDataDidChangeFn = nullptr;
    _metaDataDidChangeFn = nullptr;
    _pitchDidChangeFn = nullptr;
    
    _programComboBoxController = new ProgramComboBoxController(_view->programComboBox().get(), _studio, MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    
    // Create a list of preset names
    std::vector<MDStudio::Preset> presets = _studio->presets();
    for (auto preset : presets) {
        _presetNames.push_back(presetName(preset));
    }
    
    _isMultiChannel = true;
}

// ---------------------------------------------------------------------------------------------------------------------
PianoRollPropertiesViewController::~PianoRollPropertiesViewController()
{
    delete _programComboBoxController;
}

// ---------------------------------------------------------------------------------------------------------------------
std::string PianoRollPropertiesViewController::presetName(const MDStudio::Preset &preset)
{
    std::stringstream s;
    s << std::setfill('0') << std::setw(3) << preset._bank << std::string("-") << std::setfill('0') << std::setw(3) << preset._number << std::string(" ") << preset._name;
    return s.str();
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int PianoRollPropertiesViewController::channelNbRows(MDStudio::ComboBox *sender)
{
    return STUDIO_MAX_CHANNELS;
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MDStudio::ListItemView> PianoRollPropertiesViewController::channelViewForRow(MDStudio::ComboBox *sender, int row)
{
    auto listItemView = std::shared_ptr<MDStudio::ListItemView>(new MDStudio::ListItemView("channelListItemView" + std::to_string(row), this, std::to_string(row + 1)));
    listItemView->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    return listItemView;
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollPropertiesViewController::channelDidSelectRow(MDStudio::ComboBox *sender, int row)
{
    sender->setSelectedRow(-1, false);
    _view->channelComboBox()->close();
    
    _view->channelComboBox()->setTitle(std::to_string(row + 1));
    
    if (_channelDidChangeFn)
        _channelDidChangeFn(this, row);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollPropertiesViewController::channelDidHoverRow(MDStudio::ComboBox *sender, int row)
{
    for (unsigned int i = 0; i < sender->rowViews().size(); ++i) {
        std::shared_ptr<MDStudio::View> view = sender->rowViews()[i];
        std::shared_ptr<MDStudio::ListItemView> listItemView = (std::static_pointer_cast<MDStudio::ListItemView>)(view);
        listItemView->setIsHovering(i == row);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollPropertiesViewController::velocityPosSet(MDStudio::Slider *sender)
{
    if (_velocityDidChangeFn)
        _velocityDidChangeFn(this, static_cast<int>(sender->pos()));
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollPropertiesViewController::didConfirmPresetSelection(ProgramComboBoxController *sender, MDStudio::Preset preset)
{
    _programDidChangeFn(this, STUDIO_INSTRUMENT_FROM_PRESET(preset._bank, preset._number));
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollPropertiesViewController::sendTimeSignature()
{
    const int numerators[] = {2, 3, 4, 5, 7, 11};
    const int denumerators[] = {1, 2, 4, 8, 16, 32, 64};

    if (_timeSignatureDidChangeFn) {
        auto numSelectedSegment = _view->timeSignatureSegmentedControl(0)->selectedSegment();
        auto denSelectedSegment = _view->timeSignatureSegmentedControl(1)->selectedSegment();
        if (numSelectedSegment >= 0 && denSelectedSegment >= 0)
            _timeSignatureDidChangeFn(this, numerators[numSelectedSegment], denumerators[denSelectedSegment]);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollPropertiesViewController::timeSignatureDidSelectSegment(MDStudio::SegmentedControl *sender, int segment)
{
    sendTimeSignature();
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollPropertiesViewController::sysexDataDidChange(MDStudio::TextField *sender, std::string text)
{
    // Parse data
    auto data = parseHexString(text);
    
    // If the user did not provide the terminal F7, add it
    if ((data.size() == 0) || (data.back() != 0xF7))
        data.push_back(0xF7);
    
    if (_sysexDataDidChangeFn)
        _sysexDataDidChangeFn(this, data);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollPropertiesViewController::metaDataDidChange(MDStudio::TextField *sender, std::string text)
{
    // Parse data

    size_t nbMetaEventFound = std::count_if(_events.begin(), _events.end(), [](std::shared_ptr<MelobaseCore::Event> event) -> bool {
        auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(event);
        return channelEvent->type() == CHANNEL_EVENT_TYPE_META_GENERIC;
    });

    size_t nbStringMetaEventFound = std::count_if(_events.begin(), _events.end(), [](std::shared_ptr<MelobaseCore::Event> event) -> bool {
        auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(event);
        return channelEvent->type() == CHANNEL_EVENT_TYPE_META_GENERIC && channelEvent->param1() >= 1 && channelEvent->param1() <= 7;
    });
    
    std::vector<UInt8> data;
    if (nbStringMetaEventFound == nbMetaEventFound) {
        auto s = MDStudio::UString(text);
        for (auto c16 : *(s.str16())) {
            data.push_back(c16 & 0x7f);
        }
    } else {
        data = parseHexString(text);
    }

    if (_metaDataDidChangeFn)
        _metaDataDidChangeFn(this, data);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollPropertiesViewController::pitchDidChange(MDStudio::TextField *sender, std::string text)
{
    int pitch = 0;
    if (isNumber(text)) {
        pitch = std::stoi(text);
    }
    if (pitch < 0)
        pitch = 0;
    if (pitch > 127)
        pitch = 127;
    if (_pitchDidChangeFn)
        _pitchDidChangeFn(this, pitch);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollPropertiesViewController::setEvents(std::vector<std::shared_ptr<MelobaseCore::Event>> events, bool isMultiChannel)
{
    _events = events;
    _isMultiChannel = isMultiChannel;
    
    int channel = -1;
    int velocity = -1;
    int program = -1;
    int timeSignatureNum = -1;
    int timeSignatureDenum = -1;
    std::vector<UInt8> sysexData;
    std::vector<UInt8> metaData;
    int pitch = -1;
    
    bool isChannelSet = false;
    bool isVelocitySet = false;
    bool isProgramSet = false;
    bool isTimeSignatureSet = false;
    bool isSysexDataSet = false;
    bool isPitchSet = false;
    bool isMetaDataSet = false;
    
    bool isMetaDataString = false;
    
    for (auto event : _events) {
        auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(event);
        if (!getIsMetaEvent(channelEvent) && !isChannelSet) {
            channel = channelEvent->channel();
            isChannelSet = isMultiChannel;
        } else {
            if (channelEvent->channel() != channel)
                channel = -1;
        }
        if (channelEvent->type() == CHANNEL_EVENT_TYPE_NOTE) {
            if (!isVelocitySet) {
                velocity = channelEvent->param2() < 0 ? 127 : channelEvent->param2();
                isVelocitySet = true;
            }
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_PROGRAM_CHANGE) {
            if (!isProgramSet) {
                program = (channelEvent->channel() == 9) ? STUDIO_INSTRUMENT_GM_STANDARD_DRUM_KIT : channelEvent->param1();
                isProgramSet = true;
            } else {
                int p = (channelEvent->channel() == 9) ? STUDIO_INSTRUMENT_GM_STANDARD_DRUM_KIT : channelEvent->param1();
                if (p != program)
                    program = -1;
            }
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE) {
            if (!isTimeSignatureSet) {
                timeSignatureNum = channelEvent->param1();
                timeSignatureDenum = channelEvent->param2();
                isTimeSignatureSet = true;
            } else {
                if (channelEvent->param1() != timeSignatureNum)
                    timeSignatureNum = -1;
            }
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_SYSTEM_EXCLUSIVE) {
            if (!isSysexDataSet) {
                sysexData = channelEvent->data();
                isSysexDataSet = true;
            } else {
                if (channelEvent->data() != sysexData)
                    sysexData = {};
            }
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_KEY_AFTERTOUCH) {
            if (!isPitchSet) {
                pitch = channelEvent->param1();
                isPitchSet = true;
            } else {
                if (channelEvent->param1() != pitch)
                    pitch = -1;
            }
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_META_GENERIC) {
            if (!isMetaDataSet) {
                metaData = channelEvent->data();
                isMetaDataSet = true;
            } else {
                if (channelEvent->data() != metaData)
                    metaData = {};
            }
            isMetaDataString = channelEvent->param1() >= 1 && channelEvent->param1() <= 7;
        }
    }
    
    _view->setSubviewVisibility(isChannelSet, isVelocitySet, isProgramSet, isTimeSignatureSet, isSysexDataSet, isMetaDataSet, isPitchSet,isMetaDataString);
    
    if (isChannelSet) {
        using namespace std::placeholders;
        
        _view->channelComboBox()->setListPosition(MDStudio::ComboBox::BelowPosition);
        _view->channelComboBox()->setMaxHeight(100.0f);
        
        _view->channelComboBox()->setNbRowsFn(std::bind(&PianoRollPropertiesViewController::channelNbRows, this, _1));
        _view->channelComboBox()->setViewForRowFn(std::bind(&PianoRollPropertiesViewController::channelViewForRow, this, _1, _2));
        _view->channelComboBox()->setDidSelectRowFn(std::bind(&PianoRollPropertiesViewController::channelDidSelectRow, this, _1, _2));
        _view->channelComboBox()->setDidHoverRowFn(std::bind(&PianoRollPropertiesViewController::channelDidHoverRow, this, _1, _2));
        _view->channelComboBox()->reload();
        _view->channelComboBox()->setTitle(channel >= 0 ? std::to_string(channel + 1) : "");
    }
    
    if (isVelocitySet) {
        using namespace std::placeholders;

        _view->velocitySlider()->setPosSetFn(std::bind(&PianoRollPropertiesViewController::velocityPosSet, this, _1));
        _view->velocitySlider()->setPos(velocity >= 0 ? static_cast<float>(velocity) : 0.0f);
    }
    
    if (isProgramSet) {
        using namespace std::placeholders;
        
        _view->programComboBox()->setListPosition(MDStudio::ComboBox::BelowPosition);
        _view->programComboBox()->setMaxHeight(300.0f);
        
        _programComboBoxController->setDidConfirmPresetSelectionFn(std::bind(&PianoRollPropertiesViewController::didConfirmPresetSelection, this, _1, _2));
        
        if (program >= 0) {
            const MDStudio::Preset *preset = _studio->presetForInstrument(program);
            _programComboBoxController->setPreset(preset);
        } else {
            _programComboBoxController->setPreset(nullptr);
        }
        
        // Disable the list view if channel 10
        if (channel == 9) {
            _view->programComboBox()->setIsEnabled(false);
        } else {
            _view->programComboBox()->setIsEnabled(true);
        }
    }
    
    if (isTimeSignatureSet) {
        using namespace std::placeholders;
        
        //
        // Numerator
        //
        
        _view->timeSignatureSegmentedControl(0)->setDidSelectSegmentFn(std::bind(&PianoRollPropertiesViewController::timeSignatureDidSelectSegment, this, _1, _2));
        
        int segmentIndex = -1;
        switch (timeSignatureNum) {
            case 2:
                segmentIndex = 0;
                break;
            case 3:
                segmentIndex = 1;
                break;
            case 4:
                segmentIndex = 2;
                break;
            case 5:
                segmentIndex = 3;
                break;
            case 7:
                segmentIndex = 4;
                break;
            case 11:
                segmentIndex = 5;
                break;
        }
        _view->timeSignatureSegmentedControl(0)->setSelectedSegment(segmentIndex, false);
        
        
        //
        // Denominator
        //
        
        _view->timeSignatureSegmentedControl(1)->setDidSelectSegmentFn(std::bind(&PianoRollPropertiesViewController::timeSignatureDidSelectSegment, this, _1, _2));
        
        segmentIndex = -1;
        switch (timeSignatureDenum) {
            case 1:
                segmentIndex = 0;
                break;
            case 2:
                segmentIndex = 1;
                break;
            case 4:
                segmentIndex = 2;
                break;
            case 8:
                segmentIndex = 3;
                break;
            case 16:
                segmentIndex = 4;
                break;
            case 32:
                segmentIndex = 5;
                break;
            case 64:
                segmentIndex = 6;
                break;
        }
        _view->timeSignatureSegmentedControl(1)->setSelectedSegment(segmentIndex, false);
        
    }
    
    if (isSysexDataSet) {
        using namespace std::placeholders;
        
        _view->sysexDataTextField()->setTextDidChangeFn(std::bind(&PianoRollPropertiesViewController::sysexDataDidChange, this, _1, _2));
        std::string s;
        size_t len = sysexData.size();
        if (len > 1) {
            for (size_t i = 0; i < len - 1; ++i) {
                s += toStringHex(sysexData[i]);
                if (i < len - 2)
                    s += " ";
            }
        }
        _view->sysexDataTextField()->setText(s, false);
    }
    
    if (isMetaDataSet) {
        using namespace std::placeholders;
        
        _view->metaDataTextField()->setTextDidChangeFn(std::bind(&PianoRollPropertiesViewController::metaDataDidChange, this, _1, _2));
        
        size_t nbMetaEventFound = std::count_if(_events.begin(), _events.end(), [](std::shared_ptr<MelobaseCore::Event> event) -> bool {
            auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(event);
            return channelEvent->type() == CHANNEL_EVENT_TYPE_META_GENERIC;
        });
        
        size_t nbStringMetaEventFound = std::count_if(_events.begin(), _events.end(), [](std::shared_ptr<MelobaseCore::Event> event) -> bool {
            auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(event);
            return channelEvent->type() == CHANNEL_EVENT_TYPE_META_GENERIC && channelEvent->param1() >= 1 && channelEvent->param1() <= 7;
        });
        
        std::string s;
        if (nbStringMetaEventFound == nbMetaEventFound) {
            for (auto c : metaData)
                s += c;
        } else {
            size_t len = metaData.size();
            for (size_t i = 0; i < len; ++i) {
                s += toStringHex(metaData[i]);
                if (i < len - 1)
                    s += " ";
            }
        }
        _view->metaDataTextField()->setText(s, false);
    }
    
    if (isPitchSet) {
        using namespace std::placeholders;
        
        _view->pitchTextField()->setTextDidChangeFn(std::bind(&PianoRollPropertiesViewController::pitchDidChange, this, _1, _2));
        _view->pitchTextField()->setText(pitch >= 0 ? std::to_string(pitch) : "", false);
    }
}
