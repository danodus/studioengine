//
//  studioview.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-07-12.
//  Copyright (c) 2014-2018 Daniel Cliche. All rights reserved.
//

#ifndef STUDIOVIEW_H
#define STUDIOVIEW_H

#include <view.h>
#include <button.h>
#include <labelview.h>
#include "zoneview.h"
#include <ui.h>
#include <boxview.h>
#include <slider.h>
#include <imageview.h>

#include <memory>

class StudioView : public MDStudio::View
{
    MDStudio::UI _ui;
    
    std::shared_ptr<MDStudio::BoxView> _boxView;
    std::shared_ptr<ZoneView> _upperZoneView, _lowerZoneView;
    std::shared_ptr<MDStudio::View> _splitView;
    std::shared_ptr<MDStudio::Button> _splitButton;
    std::shared_ptr<MDStudio::Button> _isSplittedButton;
    std::shared_ptr<MDStudio::LabelView> _splitLabelView;
    std::shared_ptr<MDStudio::Slider> _masterLevelSlider;
    std::shared_ptr<MDStudio::ImageView> _masterLevelSliderImageView;

    std::shared_ptr<MDStudio::Image> _masterLevelSliderImage;

public:
    StudioView(std::string name, void *owner);
    ~StudioView();

    void setFrame(MDStudio::Rect rect) override;

    std::shared_ptr<ZoneView> upperZoneView() { return _upperZoneView; }
    std::shared_ptr<ZoneView> lowerZoneView() { return _lowerZoneView; }
    std::shared_ptr<MDStudio::Button> splitButton() { return _splitButton; }
    std::shared_ptr<MDStudio::Button> isSplittedButton() { return _isSplittedButton; }
    std::shared_ptr<MDStudio::LabelView> splitLabelView() { return _splitLabelView; }
    std::shared_ptr<MDStudio::Slider> masterLevelSlider() { return _masterLevelSlider; }
    void setIsMasterLevelSliderVisible(bool isMasterLevelSliderVisible) {
        _masterLevelSlider->setIsVisible(isMasterLevelSliderVisible);
        _masterLevelSliderImageView->setIsVisible(isMasterLevelSliderVisible);
        setFrame(frame());
        setDirty();
    }
};

#endif // STUDIOVIEW_H
