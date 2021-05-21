//
//  sequenceviewcontroller.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-06-14.
//  Copyright (c) 2016-2021 Daniel Cliche. All rights reserved.
//

#include "sequenceviewcontroller.h"

#include <platform.h>

// ---------------------------------------------------------------------------------------------------------------------
SequenceViewController::SequenceViewController(std::shared_ptr<SequenceView> sequenceView,
                                               MelobaseCore::StudioController* studioController,
                                               MelobaseCore::SequenceEditor* sequenceEditor)
    : _sequenceView(sequenceView), _studioController(studioController), _sequenceEditor(sequenceEditor) {
    using namespace std::placeholders;

    _lastVertSplitPos = sequenceView->vertSplitView()->splitPos();

    _pianoRollViewController =
        new PianoRollViewController(sequenceView->pianoRollView(), studioController, sequenceEditor);
    _arrangementViewController = new ArrangementViewController(sequenceView->arrangementView(), studioController,
                                                               sequenceEditor, _pianoRollViewController);

    _pianoRollViewController->setDidSetCursorTickPosFn(
        std::bind(&SequenceViewController::pianoRollViewControllerDidSetCursorTickPos, this, _1, _2));
    _arrangementViewController->setDidSetCursorTickPosFn(
        std::bind(&SequenceViewController::arrangementViewControllerDidSetCursorTickPos, this, _1, _2));

    _stopUpdateThread = false;
    _updateThread = std::thread(&SequenceViewController::updateThread, this);
}

// ---------------------------------------------------------------------------------------------------------------------
SequenceViewController::~SequenceViewController() {
    _stopUpdateThread = true;
    _updateThread.join();

    MDStudio::Platform::sharedInstance()->cancelDelayedInvokes(this);

    delete _pianoRollViewController;
    delete _arrangementViewController;
}

// ---------------------------------------------------------------------------------------------------------------------
void SequenceViewController::updateThread() {
    while (!_stopUpdateThread) {
        std::this_thread::sleep_for(std::chrono::microseconds(33333));
        MDStudio::Platform::sharedInstance()->invoke(
            [&]() {
                if (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusPlaying) {
                    _arrangementViewController->setCursorTickPos(_studioController->studio()->metronome()->tick(), true,
                                                                 false);
                    _pianoRollViewController->setCursorTickPos(_studioController->studio()->metronome()->tick());
                } else if (_studioController->status() ==
                           MelobaseCore::StudioController::StudioControllerStatusRecording) {
                    _arrangementViewController->setCursorTickPos(_studioController->studio()->metronome()->tick(), true,
                                                                 false);
                    _pianoRollViewController->setCursorTickPos(_studioController->studio()->metronome()->tick());
                    _arrangementViewController->updateTracks();
                    _pianoRollViewController->setSequence(_studioController->sequence());
                    _pianoRollViewController->updatePianoRoll();
                } else {
                    _arrangementViewController->handleEventRepeat();
                    _pianoRollViewController->handleEventRepeat();
                }
            },
            this);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void SequenceViewController::arrangementViewControllerDidSetCursorTickPos(ArrangementViewController* sender,
                                                                          unsigned int cursorTickPos) {
    _pianoRollViewController->setCursorTickPos(cursorTickPos);
}

// ---------------------------------------------------------------------------------------------------------------------
void SequenceViewController::pianoRollViewControllerDidSetCursorTickPos(PianoRollViewController* sender,
                                                                        unsigned int cursorTickPos) {
    _arrangementViewController->setCursorTickPos(cursorTickPos, false, true);
}

// ---------------------------------------------------------------------------------------------------------------------
void SequenceViewController::setIsArrangementViewVisible(bool isVisible) {
    _sequenceView->vertSplitView()->setTopPaneVisibility(isVisible);
}

// ---------------------------------------------------------------------------------------------------------------------
void SequenceViewController::updateControls() {
    _pianoRollViewController->updateFlagControls();
    _arrangementViewController->updateTrackControls();
}
