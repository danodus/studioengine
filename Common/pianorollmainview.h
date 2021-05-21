//
//  pianorollmainview.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-09-28.
//  Copyright © 2016-2018 Daniel Cliche. All rights reserved.
//

#ifndef PIANOROLLMAINVIEW_H
#define PIANOROLLMAINVIEW_H

#include <view.h>
#include <studio.h>
#include <ui.h>
#include <boxview.h>
#include "keyboardv.h"
#include <labelview.h>
#include <slider.h>
#include <segmentedcontrol.h>
#include <combobox.h>

#include "pianorollheaderview.h"
#include "pianorolleventsview.h"
#include "segmentedcontrolv.h"
#include "rulerview.h"

#define PIANO_ROLL_MAIN_VIEW_CONTROLLER_HEIGHT   148.0f


class PianoRollMainView : public MDStudio::View {
    
    MDStudio::UI _ui;

    std::shared_ptr<KeyboardV> _keyboard;
    std::shared_ptr<MDStudio::BoxView> _keyboardChannelBoxView;
    std::shared_ptr<PianoRollHeaderView> _pianoRollHeaderView;
    std::shared_ptr<PianoRollEventsView> _pianoRollEventsView;
    std::shared_ptr<PianoRollEventsView> _pianoRollControllerEventsView;
    std::shared_ptr<PianoRollEventsView> _pianoRollMetaEventView;
    std::shared_ptr<MDStudio::ScrollBar> _pianoRollEventsScrollBar;
    std::shared_ptr<MDStudio::ScrollView> _pianoRollEventsScrollView;
    
    std::shared_ptr<MDStudio::LabelView> _currentVelocityLabelView;
    std::shared_ptr<MDStudio::Slider> _currentVelocitySlider;
    std::shared_ptr<SegmentedControlV> _controllerSegmentedControlV;
    std::shared_ptr<RulerView> _controllerEventsRulerView;
    std::shared_ptr<MDStudio::SegmentedControl> _zoneSelectionSegmentedControl;
    std::shared_ptr<MDStudio::ComboBox> _controlChangeComboBox;
    std::shared_ptr<MDStudio::ComboBox> _metaTypeComboBox;

    std::shared_ptr<MDStudio::Image> _upperZoneImage, _lowerZoneImage;
    
public:
    PianoRollMainView(std::string name, void *owner, MDStudio::Studio *studio, double eventTickWidth, float eventHeight);
    ~PianoRollMainView();
    
    std::shared_ptr<KeyboardV> keyboard() { return _keyboard; }
    std::shared_ptr<MDStudio::BoxView> keyboardChannelBoxView() { return _keyboardChannelBoxView; }
    std::shared_ptr<PianoRollHeaderView> pianoRollHeaderView() { return _pianoRollHeaderView; }
    std::shared_ptr<PianoRollEventsView> pianoRollEventsView() { return _pianoRollEventsView; }
    std::shared_ptr<SegmentedControlV> controllerSegmentedControlV() { return _controllerSegmentedControlV; }
    std::shared_ptr<RulerView> controllerEventsRulerView() { return _controllerEventsRulerView; }
    std::shared_ptr<PianoRollEventsView> pianoRollControllerEventsView() { return _pianoRollControllerEventsView; }
    std::shared_ptr<PianoRollEventsView> pianoRollMetaEventView() { return _pianoRollMetaEventView; }
    std::shared_ptr<MDStudio::ScrollView> pianoRollEventsScrollView() { return _pianoRollEventsScrollView; }
    std::shared_ptr<MDStudio::ScrollBar> pianoRollEventsScrollBar() { return _pianoRollEventsScrollBar; }
    std::shared_ptr<MDStudio::LabelView> currentVelocityLabelView() { return _currentVelocityLabelView;}
    std::shared_ptr<MDStudio::Slider> currentVelocitySlider() { return _currentVelocitySlider; }
    std::shared_ptr<MDStudio::SegmentedControl> zoneSelectionSegmentedControl() { return _zoneSelectionSegmentedControl; }
    std::shared_ptr<MDStudio::ComboBox> controlChangeComboBox() { return _controlChangeComboBox; }
    std::shared_ptr<MDStudio::ComboBox> metaTypeComboBox() { return _metaTypeComboBox; }

    void scrollToCenteredCursor();
    void scrollToVisibleCursor();
    void scrollToVisibleRect(MDStudio::Rect r, bool isHorizOnly = false);
    
    void setFrame(MDStudio::Rect rect) override;

};

#endif // PIANOROLLMAINVIEW_H

