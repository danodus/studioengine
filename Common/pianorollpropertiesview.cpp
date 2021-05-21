//
//  pianorollpropertiesview.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-06-29.
//  Copyright (c) 2015-2018 Daniel Cliche. All rights reserved.
//

#include "pianorollpropertiesview.h"

#include <platform.h>

// ---------------------------------------------------------------------------------------------------------------------
PianoRollPropertiesView::PianoRollPropertiesView(std::string name, void *owner) : MDStudio::View(name, owner)
{
    _ui.loadUI(this, MDStudio::Platform::sharedInstance()->resourcesPath() + "/PianoRollPropertiesView.lua");
    
    _boxView = nullptr;
    _channelLabelView = nullptr;
    _channelComboBox = nullptr;
    _velocityLabelView = nullptr;
    _velocitySlider = nullptr;
    _programLabelView = nullptr;
    _programComboBox = nullptr;
    _timeSignatureLabelView = nullptr;
    _timeSignatureSegmentedControls[0] = nullptr;
    _timeSignatureSegmentedControls[1] = nullptr;
    
    addSubviews(true, true, true, true, true, true, true);
    setSubviewVisibility(false, false, false, false, false, false, false);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollPropertiesView::addSubviews(bool isChannelAvailable, bool isVelocityAvailable, bool isProgramAvailable, bool isTimeSignatureAvailable, bool isSysexDataAvailable, bool isMetaDataAvailable, bool isPitchAvailable)
{
    _boxView = std::make_shared<MDStudio::BoxView>("boxView", this);
    _boxView->setBorderColor(MDStudio::zeroColor);
    
    addSubview(_boxView);
    
    if (isChannelAvailable) {
        _channelLabelView = std::make_shared<MDStudio::LabelView>("channelLabelView", this, _ui.findString("channelStr"));
        _channelLabelView->setTextAlign(MDStudio::LabelView::RightTextAlign);
        _channelLabelView->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
        _channelComboBox = std::make_shared<MDStudio::ComboBox>("channelComboBox", this);
        _channelComboBox->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
        addSubview(_channelLabelView);
        addSubview(_channelComboBox);
    }
    
    if (isVelocityAvailable) {
        _velocityLabelView = std::make_shared<MDStudio::LabelView>("velocityLabelView", this, _ui.findString("velocityStr"));
        _velocityLabelView->setTextAlign(MDStudio::LabelView::RightTextAlign);
        _velocityLabelView->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
        _velocitySlider = std::make_shared<MDStudio::Slider>("velocitySlider", this, 0.0f, 127.0f, 100.0f);
        addSubview(_velocityLabelView);
        addSubview(_velocitySlider);
    }
    
    if (isProgramAvailable) {
        _programLabelView = std::make_shared<MDStudio::LabelView>("programLabelView", this, _ui.findString("programStr"));
        _programLabelView->setTextAlign(MDStudio::LabelView::RightTextAlign);
        _programLabelView->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
        _programComboBox = std::make_shared<MDStudio::ComboBox>("programComboBox", this, 14.0f);
        _programComboBox->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
        _programComboBox->setSearchFieldFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
        _programComboBox->setSearchFieldIsVisible(true);
        _programComboBox->setIsHorizScrollBarVisible(false);
        addSubview(_programLabelView);
        addSubview(_programComboBox);
    }
    
    if (isTimeSignatureAvailable) {
        _timeSignatureLabelView = std::make_shared<MDStudio::LabelView>("timeSignatureLabelView", this, _ui.findString("timeSignatureStr"));
        _timeSignatureLabelView->setTextAlign(MDStudio::LabelView::RightTextAlign);
        _timeSignatureLabelView->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
        std::vector<Any> items = {std::string("2"),  std::string("3"), std::string("4"), std::string("5"), std::string("7"), std::string("11")};
        _timeSignatureSegmentedControls[0] = std::make_shared<MDStudio::SegmentedControl>("timeSignatureSegmentedControl0", this, items);
        _timeSignatureSegmentedControls[0]->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
        items = {std::string("1"),  std::string("2"), std::string("4"), std::string("8"), std::string("16"), std::string("32"), std::string("64")};
        _timeSignatureSegmentedControls[1] = std::make_shared<MDStudio::SegmentedControl>("timeSignatureSegmentedControl1", this, items);
        _timeSignatureSegmentedControls[1]->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
        addSubview(_timeSignatureLabelView);
        addSubview(_timeSignatureSegmentedControls[0]);
        addSubview(_timeSignatureSegmentedControls[1]);
    }
    
    if (isSysexDataAvailable) {
        _sysexDataLabelView = std::make_shared<MDStudio::LabelView>("sysexDataLabelView", this, _ui.findString("sysexDataStr"));
        _sysexDataLabelView->setTextAlign(MDStudio::LabelView::RightTextAlign);
        _sysexDataLabelView->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
        _sysexDataTextField = std::make_shared<MDStudio::TextField>("sysexDataTextField", this);
        _sysexDataTextField->setBorderColor(MDStudio::systemButtonBorderColor);
        _sysexDataTextField->setBorderSize(2.0f);
        _sysexDataTextField->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
        addSubview(_sysexDataLabelView);
        addSubview(_sysexDataTextField);
    }
    
    if (isMetaDataAvailable) {
        _metaDataLabelView = std::make_shared<MDStudio::LabelView>("metaDataLabelView", this, _ui.findString("metaDataStr"));
        _metaDataLabelView->setTextAlign(MDStudio::LabelView::RightTextAlign);
        _metaDataLabelView->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
        _metaDataTextField = std::make_shared<MDStudio::TextField>("metaDataTextField", this);
        _metaDataTextField->setBorderColor(MDStudio::systemButtonBorderColor);
        _metaDataTextField->setBorderSize(2.0f);
        _metaDataTextField->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
        addSubview(_metaDataLabelView);
        addSubview(_metaDataTextField);
    }
    
    if (isPitchAvailable) {
        _pitchLabelView = std::make_shared<MDStudio::LabelView>("pitchLabelView", this, _ui.findString("pitchStr"));
        _pitchLabelView->setTextAlign(MDStudio::LabelView::RightTextAlign);
        _pitchLabelView->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
        _pitchTextField = std::make_shared<MDStudio::TextField>("pitchTextField", this);
        _pitchTextField->setBorderColor(MDStudio::systemButtonBorderColor);
        _pitchTextField->setBorderSize(2.0f);
        _pitchTextField->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
        addSubview(_pitchLabelView);
        addSubview(_pitchTextField);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollPropertiesView::setSubviewVisibility(bool isChannelAvailable, bool isVelocityAvailable, bool isProgramAvailable, bool isTimeSignatureAvailable, bool isSysexDataAvailable, bool isMetaDataAvailable, bool isPitchAvailable, bool isMetaDataString)
{
    _channelComboBox->close();
    _programComboBox->close();
    
    _channelLabelView->setIsVisible(isChannelAvailable);
    _channelComboBox->setIsVisible(isChannelAvailable);
    _velocityLabelView->setIsVisible(isVelocityAvailable);
    _velocitySlider->setIsVisible(isVelocityAvailable);
    _programLabelView->setIsVisible(isProgramAvailable);
    _programComboBox->setIsVisible(isProgramAvailable);
    _timeSignatureLabelView->setIsVisible(isTimeSignatureAvailable);
    _timeSignatureSegmentedControls[0]->setIsVisible(isTimeSignatureAvailable);
    _timeSignatureSegmentedControls[1]->setIsVisible(isTimeSignatureAvailable);
    _sysexDataLabelView->setIsVisible(isSysexDataAvailable);
    _sysexDataTextField->setIsVisible(isSysexDataAvailable);
    _metaDataLabelView->setTitle(_ui.findString(isMetaDataString ? "metaDataStringStr" : "metaDataStr"));
    _metaDataLabelView->setIsVisible(isMetaDataAvailable);
    _metaDataTextField->setIsVisible(isMetaDataAvailable);
    _pitchLabelView->setIsVisible(isPitchAvailable);
    _pitchTextField->setIsVisible(isPitchAvailable);
    
    setFrame(frame());
}


// ---------------------------------------------------------------------------------------------------------------------
void PianoRollPropertiesView::setFrame(MDStudio::Rect aFrame)
{
    View::setFrame(aFrame);
    
    MDStudio::Rect r = bounds();
    
    if (_boxView) {
        _boxView->setFrame(r);
    }
    
    r = MDStudio::makeInsetRect(r, 2.0f, 2.0f);
    
    const float labelWidth = 70.0f;
    
    MDStudio::Rect labelRect = MDStudio::makeRect(0.0f, r.size.height - 20.0f, labelWidth, 20.0f);
    MDStudio::Rect valueRect = MDStudio::makeRect(labelWidth + 8.0f, r.size.height - 20.0f, r.size.width - labelWidth - 8.0f, 20.0f);
    
    if (_channelLabelView->isVisible() && _channelComboBox->isVisible()) {
        _channelLabelView->setFrame(labelRect);
        _channelComboBox->setFrame(valueRect);
        
        labelRect.origin.y -= 28.0f;
        valueRect.origin.y -= 28.0f;
    }
    
    if (_velocityLabelView->isVisible() && _velocitySlider->isVisible()) {
        _velocityLabelView->setFrame(labelRect);
        _velocitySlider->setFrame(valueRect);
        
        labelRect.origin.y -= 28.0f;
        valueRect.origin.y -= 28.0f;
    }
    
    if (_programLabelView->isVisible() && _programComboBox->isVisible()) {
        _programLabelView->setFrame(labelRect);
        _programComboBox->setFrame(valueRect);
        
        labelRect.origin.y -= 28.0f;
        valueRect.origin.y -= 28.0f;
    }

    if (_timeSignatureLabelView->isVisible() && _timeSignatureSegmentedControls[0]->isVisible()) {
        _timeSignatureLabelView->setFrame(labelRect);
        _timeSignatureSegmentedControls[0]->setFrame(valueRect);
        
        labelRect.origin.y -= 28.0f;
        valueRect.origin.y -= 28.0f;
    }
    
    if (_timeSignatureLabelView->isVisible() && _timeSignatureSegmentedControls[1]->isVisible()) {
        _timeSignatureSegmentedControls[1]->setFrame(valueRect);
        
        labelRect.origin.y -= 28.0f;
        valueRect.origin.y -= 28.0f;
    }
    
    if (_sysexDataLabelView->isVisible() && _sysexDataTextField->isVisible()) {
        _sysexDataLabelView->setFrame(labelRect);
        _sysexDataTextField->setFrame(valueRect);
        
        labelRect.origin.y -= 28.0f;
        valueRect.origin.y -= 28.0f;
    }

    if (_metaDataLabelView->isVisible() && _metaDataTextField->isVisible()) {
        _metaDataLabelView->setFrame(labelRect);
        _metaDataTextField->setFrame(valueRect);
        
        labelRect.origin.y -= 28.0f;
        valueRect.origin.y -= 28.0f;
    }

    if (_pitchLabelView->isVisible() && _pitchTextField->isVisible()) {
        _pitchLabelView->setFrame(labelRect);
        _pitchTextField->setFrame(valueRect);
        
        labelRect.origin.y -= 28.0f;
        valueRect.origin.y -= 28.0f;
    }
}
