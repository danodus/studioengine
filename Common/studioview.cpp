//
//  studioview.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-07-12.
//  Copyright (c) 2014 Daniel Cliche. All rights reserved.
//

#include "studioview.h"
#include "platform.h"

#include <math.h>

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
StudioView::StudioView(std::string name, void *owner) : View(name, owner)
{
    _ui.loadUI(this, MDStudio::Platform::sharedInstance()->resourcesPath() + "/StudioView.lua");
    
    _masterLevelSliderImage = std::make_shared<Image>("MasterLevelRuler@2x.png");
    
    _boxView = std::shared_ptr<BoxView>(new BoxView("boxView", owner));
    _boxView->setFillColors(veryDimGrayColor, blackColor);
    _boxView->setBorderColor(blackColor);
    addSubview(_boxView);
    
    _upperZoneView = std::shared_ptr<ZoneView>(new ZoneView("upperZoneView", owner, 0));
    addSubview(_upperZoneView);

    _lowerZoneView = std::shared_ptr<ZoneView>(new ZoneView("lowerZoneView", owner, 1));
    addSubview(_lowerZoneView);

    _splitView = std::shared_ptr<View>(new View("splitView", owner));
    addSubview(_splitView);

    _splitButton = std::shared_ptr<Button>(new Button("splitButton", owner, _ui.findString("setSplit")));
    _splitView->addSubview(_splitButton);

    _isSplittedButton = std::shared_ptr<Button>(new Button("isSplittedButton", owner, _ui.findString("split"), nullptr));
    _isSplittedButton->setType(Button::CheckBoxButtonType);

    _splitView->addSubview(_isSplittedButton);

    _splitLabelView = std::shared_ptr<LabelView>(new LabelView("splitLabelView", owner, "-"));
    _splitLabelView->setTextAlign(LabelView::CenterTextAlign);
    _splitView->addSubview(_splitLabelView);

    _masterLevelSlider = std::shared_ptr<Slider>(new Slider("masterLevelSlider", owner, -18.0f, 3.0f, -3.0f));
    addSubview(_masterLevelSlider);
    
    _masterLevelSliderImageView = std::shared_ptr<ImageView>(new ImageView("masterLevelSliderImageView", owner, _masterLevelSliderImage));
    addSubview(_masterLevelSliderImageView);
    
    _masterLevelSlider->setThumbImage(std::make_shared<Image>("SliderHThumb@2x.png"));
    _masterLevelSlider->setMinRailImage(std::make_shared<Image>("SliderHRailLeft@2x.png"));
    _masterLevelSlider->setMiddleRailImage(std::make_shared<Image>("SliderHRailCenter@2x.png"));
    _masterLevelSlider->setMaxRailImage(std::make_shared<Image>("SliderHRailRight@2x.png"));
}

// ---------------------------------------------------------------------------------------------------------------------
StudioView::~StudioView()
{
    removeSubview(_upperZoneView);
    removeSubview(_lowerZoneView);
    removeSubview(_splitView);
    removeSubview(_boxView);
    _splitView->removeSubview(_splitButton);
    _splitView->removeSubview(_isSplittedButton);
    _splitView->removeSubview(_splitLabelView);
    removeSubview(_masterLevelSlider);
}

// ---------------------------------------------------------------------------------------------------------------------
void StudioView::setFrame(Rect aRect)
{
    View::setFrame(aRect);
    
    _boxView->setFrame(bounds());
    
    float masterLevelWidth = _masterLevelSlider->isVisible() ? 50.0f : 0.0f;
    float splitWidth = 80.0f;
    
    float zoneWidth = floorf((rect().size.width - masterLevelWidth - splitWidth) / 2.0f);

    Rect r = makeRect(0.0f, 0.0f, zoneWidth, rect().size.height);
    _lowerZoneView->setFrame(r);
    
    r.origin.x += r.size.width;
    r.size.width = splitWidth;
    
    _splitView->setFrame(makeCenteredRectInRect(r, r.size.width, 80.0f));
    
    Rect r2 = makeRect(0.0f, 0.0f, r.size.width, 20.0f);
    _splitLabelView->setFrame(r2);
    
    r2.origin.y += 30.0f;
    _splitButton->setFrame(r2);
    
    r2.origin.y += 30.0f;
    _isSplittedButton->setFrame(r2);

    r.origin.x += r.size.width;
    r.size.width = zoneWidth;
    
    _upperZoneView->setFrame(r);
    
    r = makeRect((rect().size.width - masterLevelWidth) / 2.0f, 0.0f, 80.0f, rect().size.height);

    r = makeRect(rect().size.width - 20.0f, 0.0f, 20.0f, rect().size.height);
    _masterLevelSlider->setFrame(r);
    
    r = makeRect(rect().size.width - 40.0f, 0.0f, 20.0f, rect().size.height);
    _masterLevelSliderImageView->setFrame(r);
}
