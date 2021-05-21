//
//  sequenceview.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-05-19.
//  Copyright (c) 2016-2021 Daniel Cliche. All rights reserved.
//

#include "sequenceview.h"

// ---------------------------------------------------------------------------------------------------------------------
SequenceView::SequenceView(std::string name, void* owner, MDStudio::Studio* studio, double eventTickWidth,
                           float eventHeight)
    : MDStudio::View(name, owner) {
    // Create piano roll view
    _pianoRollView =
        std::shared_ptr<PianoRollView>(new PianoRollView("pianoRollView", owner, studio, eventTickWidth, eventHeight));

    // Create the arrangement view
    _arrangementView =
        std::shared_ptr<ArrangementView>(new ArrangementView("arrangementView", owner, eventTickWidth * 0.1, 1.0f));

    // Add split view
    _vertSplitView = std::shared_ptr<MDStudio::SplitViewV>(
        new MDStudio::SplitViewV("vertSplitView", owner, _pianoRollView, _arrangementView, 300));
    addSubview(_vertSplitView);
}

// ---------------------------------------------------------------------------------------------------------------------
void SequenceView::setFrame(MDStudio::Rect aRect) {
    View::setFrame(aRect);

    _vertSplitView->setFrame(bounds());
}
