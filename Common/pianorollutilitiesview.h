//
//  pianorollutilitiesview.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-07-26.
//  Copyright © 2016-2020 Daniel Cliche. All rights reserved.
//

#ifndef PIANOROLLUTILITIESVIEW_H
#define PIANOROLLUTILITIESVIEW_H

#include <view.h>
#include <segmentedcontrol.h>
#include <ui.h>

#include "pianorollpropertiesview.h"
#include "pianorolleventslistview.h"

class PianoRollUtilitiesView : public MDStudio::View {
    
    MDStudio::UI _ui;
    
    std::shared_ptr<MDStudio::SegmentedControl> _viewSelectionSegmentedControl;
    
    std::shared_ptr<PianoRollPropertiesView> _pianoRollPropertiesView;
    std::shared_ptr<PianoRollEventsListView> _pianoRollEventsListView;
    
public:
    PianoRollUtilitiesView(std::string name, void *owner);
    
    void setFrame(MDStudio::Rect rect) override;
    
    std::shared_ptr<PianoRollPropertiesView> pianoRollPropertiesView() { return _pianoRollPropertiesView; }
    std::shared_ptr<PianoRollEventsListView> pianoRollEventsListView() { return _pianoRollEventsListView; }
    std::shared_ptr<MDStudio::SegmentedControl> viewSelectionSegmentedControl() { return _viewSelectionSegmentedControl; }
};

#endif // PIANOROLLUTILITIESVIEW_H
