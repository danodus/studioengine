//
//  pianorollutilitiesview.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-07-26.
//  Copyright © 2016-2020 Daniel Cliche. All rights reserved.
//

#include "pianorollutilitiesview.h"
#include <platform.h>

// ---------------------------------------------------------------------------------------------------------------------
PianoRollUtilitiesView::PianoRollUtilitiesView(std::string name, void *owner) : MDStudio::View(name, owner)
{
    _ui.loadUI(this, MDStudio::Platform::sharedInstance()->resourcesPath() + "/PianoRollUtilitiesView.lua");
    
    using namespace std::placeholders;
    
    std::vector<Any> items = {_ui.findString("propertiesStr"), _ui.findString("eventsStr")};
    _viewSelectionSegmentedControl = std::make_shared<MDStudio::SegmentedControl>("viewSelectionSegmentedControl", this, items);
    addSubview(_viewSelectionSegmentedControl);

    _pianoRollPropertiesView = std::make_shared<PianoRollPropertiesView>("pianoRollPropertiesView", this);
    addSubview(_pianoRollPropertiesView);
    
    _pianoRollEventsListView = std::make_shared<PianoRollEventsListView>("pianoRollEventsListView", this);
    addSubview(_pianoRollEventsListView);
    _pianoRollEventsListView->setIsVisible(false);
    
    _viewSelectionSegmentedControl->setSelectedSegment(0, false);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollUtilitiesView::setFrame(MDStudio::Rect rect)
{
    View::setFrame(rect);
    MDStudio::Rect r = bounds();
    r.size.height -= 20.0f;
    _pianoRollPropertiesView->setFrame(r);
    _pianoRollEventsListView->setFrame(r);
    r.origin.y += r.size.height;
    r.size.height = 20.0f;
    _viewSelectionSegmentedControl->setFrame(r);
}
