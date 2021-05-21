//
//  trackview.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-05-19.
//  Copyright © 2016 Daniel Cliche. All rights reserved.
//

#include "trackview.h"

// ---------------------------------------------------------------------------------------------------------------------
TrackView::TrackView(std::string name, void *owner, MDStudio::Studio *studio, int trackIndex, UInt8 trackChannel, double eventTickWidth, float eventHeight) : MDStudio::View(name, owner)
{
    _isHighlighted = false;
    _hasFocus = false;
    
    _boxView = std::shared_ptr<MDStudio::BoxView>(new MDStudio::BoxView("boxView", owner));
    _boxView->setBorderColor(MDStudio::blackColor);
    addSubview(_boxView);

    _trackClipsView = std::make_shared<TrackClipsView>("trackClipsView", this, studio, trackIndex, trackChannel, eventTickWidth, eventHeight);
    addSubview(_trackClipsView);
}

// ---------------------------------------------------------------------------------------------------------------------
TrackView::~TrackView()
{
    removeSubview(_boxView);
    removeSubview(_trackClipsView);
}

// ---------------------------------------------------------------------------------------------------------------------
void TrackView::setFrame(MDStudio::Rect rect)
{
    View::setFrame(rect);
    MDStudio::Rect r = bounds();
    r.size.height -= 2.0f;
    _trackClipsView->setFrame(r);
    _boxView->setFrame(bounds());
}

// ---------------------------------------------------------------------------------------------------------------------
void TrackView::setIsHighlighted(bool isHighlighted)
{
    _isHighlighted = isHighlighted;
    _boxView->setFillColor(_isHighlighted ? (_hasFocus ? MDStudio::blueColor : MDStudio::veryDimGrayColor) : MDStudio::blackColor);
}

// ---------------------------------------------------------------------------------------------------------------------
void TrackView::setFocusState(bool focusState)
{
    _hasFocus = focusState;
    _boxView->setFillColor(_isHighlighted ? (_hasFocus ? MDStudio::blueColor : MDStudio::veryDimGrayColor) : MDStudio::blackColor);
}
