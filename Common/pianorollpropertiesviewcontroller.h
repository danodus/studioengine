//
//  pianorollpropertiesviewcontroller.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-06-29.
//  Copyright (c) 2015-2018 Daniel Cliche. All rights reserved.
//

#ifndef PIANOROLLPROPERTIESVIEWCONTROLLER_H
#define PIANOROLLPROPERTIESVIEWCONTROLLER_H

#include "pianorollpropertiesview.h"

#include "programcomboboxcontroller.h"

#include <listitemview.h>
#include <melobasecore_event.h>
#include <studio.h>

#include <vector>

class PianoRollPropertiesViewController {
    
public:
    typedef std::function<void(PianoRollPropertiesViewController *sender, int channel)> ChannelDidChangeFnType;
    typedef std::function<void(PianoRollPropertiesViewController *sender, int velocity)> VelocityDidChangeFnType;
    typedef std::function<void(PianoRollPropertiesViewController *sender, int instrument)> ProgramDidChangeFnType;
    typedef std::function<void(PianoRollPropertiesViewController *sender, int num, int denum)> TimeSignatureDidChangeFnType;
    typedef std::function<void(PianoRollPropertiesViewController *sender, std::vector<UInt8> data)> SysexDataDidChangeFnType;
    typedef std::function<void(PianoRollPropertiesViewController *sender, std::vector<UInt8> data)> MetaDataDidChangeFnType;
    typedef std::function<void(PianoRollPropertiesViewController *sender, int pitch)> PitchDidChangeFnType;
    
private:
    
    std::shared_ptr<PianoRollPropertiesView> _view;
    
    ProgramComboBoxController *_programComboBoxController;
    
    MDStudio::Studio *_studio;
    
    std::vector<std::string> _presetNames;
    
    std::string presetName(const MDStudio::Preset &preset);
    
    unsigned int channelNbRows(MDStudio::ComboBox *sender);
    std::shared_ptr<MDStudio::ListItemView> channelViewForRow(MDStudio::ComboBox *sender, int row);
    void channelDidSelectRow(MDStudio::ComboBox *sender, int row);
    void channelDidHoverRow(MDStudio::ComboBox *sender, int row);

    void didConfirmPresetSelection(ProgramComboBoxController *sender, MDStudio::Preset preset);

    void sendTimeSignature();
    void timeSignatureDidSelectSegment(MDStudio::SegmentedControl *sender, int segment);
    
    void velocityPosSet(MDStudio::Slider *sender);
    
    void sysexDataDidChange(MDStudio::TextField *sender, std::string text);
    void metaDataDidChange(MDStudio::TextField *sender, std::string text);

    void pitchDidChange(MDStudio::TextField *sender, std::string text);
    
    std::vector<std::shared_ptr<MelobaseCore::Event>> _events;
    bool _isMultiChannel;
    
    ChannelDidChangeFnType _channelDidChangeFn;
    VelocityDidChangeFnType _velocityDidChangeFn;
    ProgramDidChangeFnType _programDidChangeFn;
    TimeSignatureDidChangeFnType _timeSignatureDidChangeFn;
    SysexDataDidChangeFnType _sysexDataDidChangeFn;
    MetaDataDidChangeFnType _metaDataDidChangeFn;
    PitchDidChangeFnType _pitchDidChangeFn;

public:
    PianoRollPropertiesViewController(std::shared_ptr<PianoRollPropertiesView> view, MDStudio::Studio *studio);
    ~PianoRollPropertiesViewController();
    
    void setEvents(std::vector<std::shared_ptr<MelobaseCore::Event>> events, bool isMultiChannel);
    
    void setChannelDidChangeFn(ChannelDidChangeFnType channelDidChange) { _channelDidChangeFn = channelDidChange; }
    void setVelocityDidChangeFn(VelocityDidChangeFnType velocityDidChange) { _velocityDidChangeFn = velocityDidChange; }
    void setProgramDidChangeFn(ProgramDidChangeFnType programDidChange) { _programDidChangeFn = programDidChange; }
    void setTimeSignatureDidChangeFn(TimeSignatureDidChangeFnType timeSignatureDidChange) { _timeSignatureDidChangeFn = timeSignatureDidChange; }
    void setSysexDataDidChangeFn(SysexDataDidChangeFnType sysexDataDidChange) { _sysexDataDidChangeFn = sysexDataDidChange; }
    void setMetaDataDidChangeFn(MetaDataDidChangeFnType metaDataDidChange) { _metaDataDidChangeFn = metaDataDidChange; }
    void setPitchDidChangeFn(PitchDidChangeFnType pitchDidChange) { _pitchDidChangeFn = pitchDidChange; }
};


#endif // PIANOROLLPROPERTIESVIEWCONTROLLER_H
