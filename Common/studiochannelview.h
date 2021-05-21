//
//  studiochannelview.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-07-14.
//  Copyright (c) 2014-2016 Daniel Cliche. All rights reserved.
//

#ifndef STUDIOCHANNELVIEW_H
#define STUDIOCHANNELVIEW_H

#include <view.h>
#include <imageview.h>
#include <combobox.h>
#include <labelview.h>
#include <slider.h>
#include <image.h>
#include <segmentedcontrol.h>
#include <ui.h>
#include <studio.h>

#include <memory>

class StudioChannelView : public MDStudio::View
{
    MDStudio::UI ui;
    
    std::shared_ptr<MDStudio::View> _moduleView;
    
    std::shared_ptr<MDStudio::ImageView> _backgroundImageView;
    std::shared_ptr<MDStudio::ImageView> _backgroundGlossImageView;
    std::shared_ptr<MDStudio::ComboBox> _instrumentComboBox;
    std::shared_ptr<MDStudio::Slider> _levelSlider;
    std::shared_ptr<MDStudio::Slider> _balanceSlider;
    std::shared_ptr<MDStudio::Slider> _reverbSlider;
    std::shared_ptr<MDStudio::Slider> _chorusSlider;
    std::shared_ptr<MDStudio::Image> _backgroundImage, _backgroundGlossImage;
    
    int _channel;

public:
    StudioChannelView(std::string name, void *owner, int channel);
    ~StudioChannelView();

    void setFrame(MDStudio::Rect rect) override;

    std::shared_ptr<MDStudio::ComboBox> instrumentComboBox() { return _instrumentComboBox; }
    std::shared_ptr<MDStudio::Slider> levelSlider() { return _levelSlider; }
    std::shared_ptr<MDStudio::Slider> balanceSlider() { return _balanceSlider; }
    std::shared_ptr<MDStudio::Slider> reverbSlider() { return _reverbSlider; }
    std::shared_ptr<MDStudio::Slider> chorusSlider() { return _chorusSlider; }
    
    void setChannel(int channel);
    int channel() { return _channel; }
};

#endif // STUDIOCHANNELVIEW_H
