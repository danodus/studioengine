//
//  zoneview.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-02-08.
//  Copyright © 2016 Daniel Cliche. All rights reserved.
//

#ifndef ZONEVIEW_H
#define ZONEVIEW_H

#include "studiochannelview.h"

#include <view.h>
#include <stepper.h>

class ZoneView : public MDStudio::View
{
    std::shared_ptr<MDStudio::SegmentedControl> _channelSegmentedControl;

    std::shared_ptr<MDStudio::BoxView> _transposeBoxView;
    std::shared_ptr<MDStudio::LabelView> _transposeLabelView;
    std::shared_ptr<MDStudio::Stepper> _transposeStepper;
    std::shared_ptr<MDStudio::ImageView> _transposeLabelImageView;
    
    std::shared_ptr<MDStudio::Image> _transposeLabelImage;
    
    std::shared_ptr<StudioChannelView> _studioChannelView;
    
    int _channel;
    
public:
    
    ZoneView(std::string name, void *owner, int zone);
    ~ZoneView();
    
    void setFrame(MDStudio::Rect rect) override;
    
    std::shared_ptr<MDStudio::Stepper> transposeStepper() { return _transposeStepper; }
    std::shared_ptr<MDStudio::LabelView> transposeLabelView() { return _transposeLabelView; }
    std::shared_ptr<MDStudio::SegmentedControl> channelSegmentedControl() { return _channelSegmentedControl; }
    std::shared_ptr<StudioChannelView> studioChannelView() { return _studioChannelView; }
};

#endif // ZONEVIEW_H
