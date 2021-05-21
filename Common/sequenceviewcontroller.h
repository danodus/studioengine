//
//  sequenceviewcontroller.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-06-14.
//  Copyright (c) 2016-2021 Daniel Cliche. All rights reserved.
//

#ifndef SEQUENCEVIEWCONTROLLER_H
#define SEQUENCEVIEWCONTROLLER_H

#include "arrangementviewcontroller.h"
#include "pianorollviewcontroller.h"
#include "sequenceview.h"

class SequenceViewController {
    std::shared_ptr<SequenceView> _sequenceView;

    MelobaseCore::StudioController* _studioController;

    ArrangementViewController* _arrangementViewController;
    PianoRollViewController* _pianoRollViewController;
    MelobaseCore::SequenceEditor* _sequenceEditor;

    std::atomic<bool> _stopUpdateThread;
    std::thread _updateThread;

    float _lastVertSplitPos;

    void updateThread();

    void arrangementViewControllerDidSetCursorTickPos(ArrangementViewController* sender, unsigned int cursorTickPos);
    void pianoRollViewControllerDidSetCursorTickPos(PianoRollViewController* sender, unsigned int cursorTickPos);

   public:
    SequenceViewController(std::shared_ptr<SequenceView> sequenceView, MelobaseCore::StudioController* studioController,
                           MelobaseCore::SequenceEditor* sequenceEditor);
    ~SequenceViewController();

    void setIsArrangementViewVisible(bool isVisible);

    void updateControls();

    ArrangementViewController* arrangementViewController() { return _arrangementViewController; }
    PianoRollViewController* pianoRollViewController() { return _pianoRollViewController; }
};

#endif  // SEQUENCEVIEWCONTROLLER_H
