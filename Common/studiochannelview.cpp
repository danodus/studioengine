//
//  studiochannelview.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-07-14.
//  Copyright (c) 2014-2018 Daniel Cliche. All rights reserved.
//

#include "studiochannelview.h"

#include "helpers.h"

#include <platform.h>
#include <draw.h>

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
StudioChannelView::StudioChannelView(std::string name, void* owner, int channel) : View(name, owner)
{
    _moduleView = std::make_shared<View>("moduleView", this);
    addSubview(_moduleView);
    
    ui.loadUI(_moduleView.get(), MDStudio::Platform::sharedInstance()->resourcesPath() + "/StudioChannelView.lua");

    _backgroundImageView = std::dynamic_pointer_cast<ImageView>(ui.findView("backgroundImageView"));
    _backgroundGlossImageView = std::dynamic_pointer_cast<ImageView>(ui.findView("backgroundGlossImageView"));
    
    _instrumentComboBox = std::dynamic_pointer_cast<ComboBox>(ui.findView("instrumentComboBox"));
    _instrumentComboBox->setIsHorizScrollBarVisible(false);
    _instrumentComboBox->setSearchFieldIsVisible(true);
    
    _levelSlider = std::dynamic_pointer_cast<Slider>(ui.findView("levelSlider"));
    _balanceSlider = std::dynamic_pointer_cast<Slider>(ui.findView("balanceSlider"));
    _reverbSlider = std::dynamic_pointer_cast<Slider>(ui.findView("reverbSlider"));
    _chorusSlider = std::dynamic_pointer_cast<Slider>(ui.findView("chorusSlider"));
    
    setChannel(channel);
}

// ---------------------------------------------------------------------------------------------------------------------
StudioChannelView::~StudioChannelView()
{
}

// ---------------------------------------------------------------------------------------------------------------------
void StudioChannelView::setFrame(Rect aRect)
{
    View::setFrame(aRect);
    
    MDStudio::Rect r = bounds();
    _moduleView->setFrame(r);
    r = _moduleView->bounds();
    _backgroundImageView->setFrame(r);
    _backgroundGlossImageView->setFrame(r);

}

// ---------------------------------------------------------------------------------------------------------------------
void StudioChannelView::setChannel(int channel)
{
    _backgroundImageView->setColor(channelColors[channel]);
    _instrumentComboBox->setIsEnabled((channel == 9) ? false : true);
}
