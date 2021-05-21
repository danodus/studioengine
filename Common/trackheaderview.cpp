//
//  trackheaderview.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-05-19.
//  Copyright © 2016-2019 Daniel Cliche. All rights reserved.
//

#include "trackheaderview.h"

#include <platform.h>

// ---------------------------------------------------------------------------------------------------------------------
TrackHeaderView::TrackHeaderView(std::string name, void *owner, MelobaseCore::Sequence *sequence, int trackIndex) : MDStudio::View(name, owner), _sequence(sequence), _trackIndex(trackIndex)
{
    _ui.loadUI(this, MDStudio::Platform::sharedInstance()->resourcesPath() + "/TrackHeaderView.lua");
    
    _isHighlighted = false;
    _hasFocus = false;
    
    _boxView = std::shared_ptr<MDStudio::BoxView>(new MDStudio::BoxView("boxView", owner));
    _boxView->setBorderColor(MDStudio::grayColor);
    _boxView->setFillColor(MDStudio::zeroColor);
    addSubview(_boxView);
    
    _channelBoxView = std::shared_ptr<MDStudio::BoxView>(new MDStudio::BoxView("channelView", owner));
    _channelBoxView->setBorderColor(MDStudio::grayColor);
    _channelBoxView->setFillColor(MDStudio::grayColor);
    addSubview(_channelBoxView);

    
    _nameLabelView = std::make_shared<MDStudio::LabelView>("nameLabelView", owner, _ui.findString("nameStr"));
    addSubview(_nameLabelView);
    
    _nameTextField = std::make_shared<MDStudio::TextField>("nameTextField", owner);
    addSubview(_nameTextField);
    
    _nameTextField->setText(sequence->data.tracks[trackIndex]->name);
    
    _armedButton = std::make_shared<MDStudio::Button>("armedButton", owner, _ui.findString("armStr"));
    _armedButton->setType(MDStudio::Button::CheckBoxButtonType);
    addSubview(_armedButton);
    
    _muteButton = std::make_shared<MDStudio::Button>("muteButton", owner, _ui.findString("muteStr"));
    _muteButton->setType(MDStudio::Button::CheckBoxButtonType);
    addSubview(_muteButton);

    _soloButton = std::make_shared<MDStudio::Button>("soloButton", owner, _ui.findString("soloStr"));
    _soloButton->setType(MDStudio::Button::CheckBoxButtonType);
    addSubview(_soloButton);
    
    _channelLabelView = std::make_shared<MDStudio::LabelView>("channelLabelView", owner, _ui.findString("channelStr"));
    addSubview(_channelLabelView);
    
    _channelComboBox = std::make_shared<MDStudio::ComboBox>("channelComboBox", owner);
    _channelComboBox->setListPosition(MDStudio::ComboBox::BelowPosition);
    _channelComboBox->setIsHorizScrollBarVisible(false);
    _channelComboBox->setIsVertScrollBarVisible(false);
    addSubview(_channelComboBox);
}

// ---------------------------------------------------------------------------------------------------------------------
TrackHeaderView::~TrackHeaderView()
{
    removeSubview(_soloButton);
    removeSubview(_muteButton);
    removeSubview(_armedButton);
    removeSubview(_nameLabelView);
    removeSubview(_nameTextField);
    removeSubview(_boxView);
    removeSubview(_channelLabelView);
    removeSubview(_channelComboBox);
}

// ---------------------------------------------------------------------------------------------------------------------
void TrackHeaderView::setFrame(MDStudio::Rect rect)
{
    View::setFrame(rect);
    _boxView->setFrame(bounds());
    
    MDStudio::Rect r = bounds();
    
    r.size.width = 2.0f;
    _channelBoxView->setFrame(r);
    
    r.origin.x += 2.0f;

    r.origin.y += r.size.height - 20.0f;
    r.size.height = 20.0f;
    
    r.size.width = 40.0f;
    _nameLabelView->setFrame(r);
    
    r.origin.x += r.size.width + 8.0f;
    r.size.width = bounds().size.width - r.origin.x;
    _nameTextField->setFrame(r);
    
    r.origin.x = 2.0f;
    r.origin.y -= 20.0f;
    r.size.width = 60.0f;
    _armedButton->setFrame(r);
    
    r.origin.y -= 20.0f;
    _muteButton->setFrame(r);

    r.origin.y -= 20.0f;
    _soloButton->setFrame(r);
    
    r.origin.y -= 20.0f;
    r.origin.x = 2.0f;
    r.size.width = 60.0f;
    _channelLabelView->setFrame(r);
    
    r.origin.x += r.size.width + 8.0f;
    r.size.width = 60.0f;
    _channelComboBox->setFrame(r);
}

// ---------------------------------------------------------------------------------------------------------------------
void TrackHeaderView::startNameEdition()
{
    _nameTextField->startEdition();
}

// ---------------------------------------------------------------------------------------------------------------------
void TrackHeaderView::setAreControlsVisible(bool areVisible)
{
    _nameLabelView->setIsVisible(areVisible);
    _nameTextField->setIsVisible(areVisible);
    _armedButton->setIsVisible(areVisible);
    _muteButton->setIsVisible(areVisible);
    _soloButton->setIsVisible(areVisible);
    _channelLabelView->setIsVisible(areVisible);
    _channelComboBox->setIsVisible(areVisible);
}
