//
//  pianorollpropertiesview.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-06-29.
//  Copyright (c) 2015-2018 Daniel Cliche. All rights reserved.
//

#ifndef PIANOROLLPROPERTIESVIEW_H
#define PIANOROLLPROPERTIESVIEW_H

#include <view.h>
#include <ui.h>
#include <combobox.h>
#include <slider.h>
#include <labelview.h>
#include <boxview.h>
#include <segmentedcontrol.h>

class PianoRollPropertiesView : public MDStudio::View {
    MDStudio::UI _ui;
 
    std::shared_ptr<MDStudio::BoxView> _boxView;
    std::shared_ptr<MDStudio::LabelView> _channelLabelView;
    std::shared_ptr<MDStudio::ComboBox> _channelComboBox;
    std::shared_ptr<MDStudio::LabelView> _velocityLabelView;
    std::shared_ptr<MDStudio::Slider> _velocitySlider;
    std::shared_ptr<MDStudio::LabelView> _programLabelView;
    std::shared_ptr<MDStudio::ComboBox> _programComboBox;
    std::shared_ptr<MDStudio::LabelView> _timeSignatureLabelView;
    std::shared_ptr<MDStudio::SegmentedControl> _timeSignatureSegmentedControls[2];
    std::shared_ptr<MDStudio::LabelView> _sysexDataLabelView;
    std::shared_ptr<MDStudio::TextField> _sysexDataTextField;
    std::shared_ptr<MDStudio::LabelView> _metaDataLabelView;
    std::shared_ptr<MDStudio::TextField> _metaDataTextField;
    std::shared_ptr<MDStudio::LabelView> _pitchLabelView;
    std::shared_ptr<MDStudio::TextField> _pitchTextField;

    void addSubviews(bool isChannelAvailable, bool isVelocityAvailable, bool isProgramAvailable, bool isTimeSignatureAvailable, bool isSysexDataAvailable, bool isMetaDataAvailable, bool isPitchAvailable);

public:
    
    PianoRollPropertiesView(std::string name, void *owner);
    
    void setFrame(MDStudio::Rect aFrame) override;

    void setSubviewVisibility(bool isChannelAvailable, bool isVelocityAvailable, bool isProgramAvailable, bool isTimeSignatureAvailable, bool isSysexDataAvailable, bool isMetaDataAvailable, bool isPitchAvailable, bool isMetaDataString = false);
    
    std::shared_ptr<MDStudio::ComboBox> channelComboBox() { return _channelComboBox; }
    std::shared_ptr<MDStudio::Slider> velocitySlider() { return _velocitySlider; }
    std::shared_ptr<MDStudio::ComboBox> programComboBox() { return _programComboBox; }
    std::shared_ptr<MDStudio::SegmentedControl> timeSignatureSegmentedControl(int index) { return _timeSignatureSegmentedControls[index]; }
    std::shared_ptr<MDStudio::TextField> sysexDataTextField() { return _sysexDataTextField; }
    std::shared_ptr<MDStudio::TextField> metaDataTextField() { return _metaDataTextField; }
    std::shared_ptr<MDStudio::TextField> pitchTextField() { return _pitchTextField; }
    
};

#endif // PIANOROLLPROPERTIESVIEW_H
