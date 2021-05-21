//
//  topviewcontroller.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2014-07-21.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#include "topviewcontroller.h"

#include <math.h>
#include <midifile.h>
#include <platform.h>
#include <responderchain.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <string>  // string

#include "folderlistitemview.h"
#include "helpers.h"
#include "sequencelistitemview.h"
#include "trackheaderview.h"

#define TOP_VIEW_CONTROLLER_MASTER_CHANNEL 0
#define TOP_VIEW_CONTROLLER_AUTO_SAVE_PERIOD 60.0

using namespace MDStudio;

constexpr int TopViewController::metronomeModeNone;
constexpr int TopViewController::metronomeModeTapTempo;
constexpr int TopViewController::metronomeModeSpecified;

// ---------------------------------------------------------------------------------------------------------------------
TopViewController::TopViewController(std::shared_ptr<TopView> view, MelobaseCore::StudioController* studioController,
                                     MelobaseCore::SequencesDB* sequencesDB, MelobaseCore::Server* server,
                                     std::string defaultMIDIInputPortName, std::string defaultMIDIOutputPortName,
                                     bool isFadeInEnabled)
    : _view(view),
      _studioController(studioController),
      _sequencesDB(sequencesDB),
      _server(server),
      _isFadeInEnabled(isFadeInEnabled) {
    using namespace std::placeholders;

    _isExportingAudio = false;

    _sequencesDB->setSequenceAddedFn(std::bind(&TopViewController::sequencesDBSequenceAdded, this, _1));
    _sequencesDB->setFolderAddedFn(std::bind(&TopViewController::sequencesDBFolderAdded, this, _1, _2));
    _sequencesDB->setWillRemoveSequenceFn(std::bind(&TopViewController::sequencesDBWillRemoveSequence, this, _1, _2));
    _sequencesDB->setSequenceRemovedFn(std::bind(&TopViewController::sequencesDBSequenceRemoved, this, _1));
    _sequencesDB->setFolderRemovedFn(std::bind(&TopViewController::sequencesDBFolderRemoved, this, _1));
    _sequencesDB->setSequenceUpdatedFn(std::bind(&TopViewController::sequencesDBSequenceUpdated, this, _1));
    _sequencesDB->setFolderUpdatedFn(std::bind(&TopViewController::sequencesDBFolderUpdated, this, _1));
    _sequencesDB->undoManager()->setStatesDidChangeFn(
        std::bind(&TopViewController::undoManagerStatesDidChange, this, _1));

    _audioExport = new MDStudio::AudioExport(_studioController->sequencer());
    _audioExport->setAudioExportDidStartFn(std::bind(&TopViewController::audioExportDidStart, this, _1));
    _audioExport->setAudioExportDidSetProgressFn(
        std::bind(&TopViewController::audioExportDidSetProgress, this, _1, _2));
    _audioExport->setAudioExportDidFinishFn(std::bind(&TopViewController::audioExportDidFinish, this, _1));

    _midiImport = new MIDIImport(_sequencesDB);
    _midiImport->setMIDIImportDidStartFn(std::bind(&TopViewController::midiImportDidStart, this, _1));
    _midiImport->setMIDIImportDidSetProgressFn(std::bind(&TopViewController::midiImportDidSetProgress, this, _1, _2));
    _midiImport->setMIDIImportDidFinishFn(std::bind(&TopViewController::midiImportDidFinish, this, _1, _2));

    _menuStatesDidChangeFn = nullptr;

    _isRecordingNewSequence = true;
    _isLearningTempo = false;
    _isSettingSplitPosition = false;
    _splitPosition = 60;
    _isKeyboardSplitted = false;
    _lastSequenceWasPlayed = false;
    _lastSequence = nullptr;
    _metronomeArmed = false;
    _metronomeLearningPitch = 0;
    _isMIDIDeviceAvailable = false;
    _isReloadSequencePending = false;
    _isModalViewPresented = false;

    _learnTempoArrowImage = std::make_shared<Image>("RedArrow@2x.png");

    _learnTempoArrowImageView[0] =
        std::shared_ptr<ImageView>(new ImageView("learnTempoArrowImageView0", this, _learnTempoArrowImage));
    _learnTempoArrowImageView[1] =
        std::shared_ptr<ImageView>(new ImageView("learnTempoArrowImageView1", this, _learnTempoArrowImage));
    _view->learnTempoSheet()->addSubview(_view->learnTempoImageView());
    _view->learnTempoSheet()->addSubview(_view->learnTempoLabelView());
    _view->learnTempoSheet()->addSubview(_learnTempoArrowImageView[0]);
    _view->learnTempoSheet()->addSubview(_learnTempoArrowImageView[1]);

    _view->cancelButton()->setClickedFn(std::bind(&TopViewController::splitCancelClicked, this, _1));

    _dbViewController = new DBViewController(view->dbView(), sequencesDB);
    _dbViewController->setSequenceSelectionDidChangeFn(
        std::bind(&TopViewController::dbViewControllerSequenceSelectionDidChange, this, _1, _2));
    _dbViewController->setFolderSelectionDidChangeFn(
        std::bind(&TopViewController::dbViewControllerFolderSelectionDidChange, this, _1, _2));
    _dbViewController->setFoldersWillChangeFn(
        std::bind(&TopViewController::dbViewControllerFoldersWillChange, this, _1));
    _dbViewController->setSequenceNameDidChangeFn(
        std::bind(&TopViewController::dbViewControllerSequenceNameDidChange, this, _1, _2));
    _dbViewController->setSequenceRatingDidChangeFn(
        std::bind(&TopViewController::dbViewControllerSequenceRatingDidChange, this, _1, _2));
    _dbViewController->setFocusDidChangeFn([=](DBViewController* sender) {
        if (_menuStatesDidChangeFn) _menuStatesDidChangeFn(this);
    });
    _dbViewController->setDidSetPaneVisibilityFn(
        std::bind(&TopViewController::dbViewControllerDidSetPaneVisibility, this, _1));

    _sequenceEditor = new MelobaseCore::SequenceEditor(sequencesDB->undoManager(), _studioController->studio(), false);
    _isSequenceModified = false;
    _isSequenceSaved = true;

    _sequenceViewController = new SequenceViewController(view->sequenceView(), _studioController, _sequenceEditor);
    _sequenceViewController->arrangementViewController()->setWillModifySequenceFn(
        std::bind(&TopViewController::arrangementViewControllerWillModifySequence, this, _1));
    _sequenceViewController->pianoRollViewController()->setWillModifySequenceFn(
        std::bind(&TopViewController::pianoRollViewControllerWillModifySequence, this, _1));
    _sequenceViewController->pianoRollViewController()->setDidModifySequenceFn(
        std::bind(&TopViewController::pianoRollViewControllerDidModifySequence, this, _1));
    _sequenceViewController->pianoRollViewController()->setDidSelectEventsFn(
        std::bind(&TopViewController::pianoRollViewControllerDidSelectEvents, this, _1));
    _sequenceViewController->pianoRollViewController()->setDidSetPaneVisibilityFn(
        std::bind(&TopViewController::pianoRollViewControllerDidSetPaneVisibility, this, _1));
    _sequenceViewController->pianoRollViewController()->setDidSelectZoneFn(
        std::bind(&TopViewController::pianoRollViewControllerDidSelectZone, this, _1, _2));

    _sequenceViewController->pianoRollViewController()->setWillModifyAnnotationsFn(
        [this](PianoRollViewController* sender) {
            // We ensure that the undo stack begins at the first edition action
            if (!_isSequenceModified) _sequencesDB->undoManager()->clear();
        });

    _sequenceViewController->pianoRollViewController()->setDidModifyAnnotationsFn(
        [this](PianoRollViewController* sender) {
            _sequenceViewController->arrangementViewController()->updateTracks();
            _isSequenceModified = true;
            _areMetaDataChanged = true;
            _isSequenceSaved = false;
        });

    _upperZoneViewController = new ZoneViewController(_view->studioView()->upperZoneView(), _studioController, 0);
    _lowerZoneViewController = new ZoneViewController(_view->studioView()->lowerZoneView(), _studioController, 1);

    _upperZoneViewController->setChannelDidChangeFn(
        std::bind(&TopViewController::zoneViewControllerChannelDidChange, this, _1, _2));
    _lowerZoneViewController->setChannelDidChangeFn(
        std::bind(&TopViewController::zoneViewControllerChannelDidChange, this, _1, _2));

    _aboutViewController = new AboutViewController(
        _view.get(), _view->aboutView(), MDStudio::Platform::sharedInstance()->resourcesPath() + "/AboutView.lua");
    _aboutViewController->setAcknowledgementButtonClickedFn(
        std::bind(&TopViewController::aboutViewControllerAcknowledgementButtonClicked, this, _1));
    _aboutViewController->setWillAppear(std::bind(&TopViewController::modalViewControllerWillAppear, this, _1));
    _aboutViewController->setDidDisappear(std::bind(&TopViewController::modalViewControllerDidDisappear, this, _1));

    _acknowledgementsViewController =
        new ModalViewController(_view.get(), _view->acknowledgementsView(),
                                MDStudio::Platform::sharedInstance()->resourcesPath() + "/AcknowledgementsView.lua");
    _acknowledgementsViewController->setWillAppear(
        std::bind(&TopViewController::modalViewControllerWillAppear, this, _1));
    _acknowledgementsViewController->setDidDisappear(
        std::bind(&TopViewController::modalViewControllerDidDisappear, this, _1));

    _confirmCleanupViewController =
        new ModalViewController(_view.get(), _view->confirmCleanupView(),
                                MDStudio::Platform::sharedInstance()->resourcesPath() + "/ConfirmCleanupView.lua");
    _confirmCleanupViewController->setWillAppear(
        std::bind(&TopViewController::modalViewControllerWillAppear, this, _1));
    _confirmCleanupViewController->setDidDisappear(
        std::bind(&TopViewController::modalViewControllerDidDisappear, this, _1));

    _confirmEmptyTrashViewController =
        new ModalViewController(_view.get(), _view->confirmEmptyTrashView(),
                                MDStudio::Platform::sharedInstance()->resourcesPath() + "/ConfirmEmptyTrashView.lua");
    _confirmEmptyTrashViewController->setWillAppear(
        std::bind(&TopViewController::modalViewControllerWillAppear, this, _1));
    _confirmEmptyTrashViewController->setDidDisappear(
        std::bind(&TopViewController::modalViewControllerDidDisappear, this, _1));

    _confirmSetAsPlayedAllViewController = new ModalViewController(
        _view.get(), _view->confirmSetAsPlayedAllView(),
        MDStudio::Platform::sharedInstance()->resourcesPath() + "/ConfirmSetAsPlayedAllView.lua");
    _confirmSetAsPlayedAllViewController->setWillAppear(
        std::bind(&TopViewController::modalViewControllerWillAppear, this, _1));
    _confirmSetAsPlayedAllViewController->setDidDisappear(
        std::bind(&TopViewController::modalViewControllerDidDisappear, this, _1));

    _confirmCleanupViewController->setDidReceiveResult(
        std::bind(&TopViewController::modalViewControllerDidReceiveResult, this, _1, _2));
    _confirmEmptyTrashViewController->setDidReceiveResult(
        std::bind(&TopViewController::modalViewControllerDidReceiveResult, this, _1, _2));
    _confirmSetAsPlayedAllViewController->setDidReceiveResult(
        std::bind(&TopViewController::modalViewControllerDidReceiveResult, this, _1, _2));

    _preferencesViewController = new PreferencesViewController(
        _view.get(), _view->preferencesView(),
        MDStudio::Platform::sharedInstance()->resourcesPath() + "/PreferencesView.lua", defaultMIDIInputPortName, "",
        _studioController->studio()->audioOutputDevice().first,
        _studioController->studio()->audioOutputDevice().second);
    _preferencesViewController->setWillAppear(std::bind(&TopViewController::modalViewControllerWillAppear, this, _1));
    _preferencesViewController->setDidDisappear(
        std::bind(&TopViewController::modalViewControllerDidDisappear, this, _1));
    _preferencesViewController->setAudioOutputDevices(_studioController->studio()->audioOutputDevices());
    _preferencesViewController->restoreStates();

    _newSequenceViewController =
        new NewSequenceViewController(_view.get(), _view->newSequenceView(),
                                      MDStudio::Platform::sharedInstance()->resourcesPath() + "/NewSequenceView.lua");
    _newSequenceViewController->setWillAppear(std::bind(&TopViewController::modalViewControllerWillAppear, this, _1));
    _newSequenceViewController->setDidDisappear(
        std::bind(&TopViewController::modalViewControllerDidDisappear, this, _1));
    _newSequenceViewController->setDidReceiveResult(
        std::bind(&TopViewController::modalViewControllerDidReceiveResult, this, _1, _2));

    _audioExportViewController =
        new AudioExportViewController(_view.get(), _view->audioExportView(),
                                      MDStudio::Platform::sharedInstance()->resourcesPath() + "/AudioExportView.lua");
    _audioExportViewController->setWillAppear(std::bind(&TopViewController::modalViewControllerWillAppear, this, _1));
    _audioExportViewController->setDidDisappear(
        std::bind(&TopViewController::modalViewControllerDidDisappear, this, _1));

    _midiImportViewController =
        new MIDIImportViewController(_view.get(), _view->midiImportView(),
                                     MDStudio::Platform::sharedInstance()->resourcesPath() + "/MIDIImportView.lua");
    _audioExportViewController->setWillAppear(std::bind(&TopViewController::modalViewControllerWillAppear, this, _1));
    _audioExportViewController->setDidDisappear(
        std::bind(&TopViewController::modalViewControllerDidDisappear, this, _1));

    _studioController->setIsAutoStopRecordEnabled(_preferencesViewController->isAutoStopRecordEnabled());
    _studioController->setAutoStopRecordPeriod(_preferencesViewController->autoStopRecordPeriod());

    if (_preferencesViewController->midiDestination() == PreferencesViewController::InternalSynthMIDIDestination) {
        _studioController->studio()->setIsInternalSynthEnabled(true);
        _studioController->studio()->setIsMIDIOutputEnabled(false);
    } else if (_preferencesViewController->midiDestination() == PreferencesViewController::MIDIOutputMIDIDestination) {
        _studioController->studio()->setIsInternalSynthEnabled(false);
        _studioController->studio()->setIsMIDIOutputEnabled(true);
    } else {
        _studioController->studio()->setIsInternalSynthEnabled(false);
        _studioController->studio()->setIsMIDIOutputEnabled(false);
    }
    _studioController->studio()->setNbVoices(_preferencesViewController->nbVoices());
    _view->studioView()->setIsMasterLevelSliderVisible(_studioController->studio()->isInternalSynthEnabled());

    _studioController->setSaveSequencesFn(std::bind(&TopViewController::studioControllerSaveSequences, this, _1));
    _studioController->setRecordingDidStartFn(
        std::bind(&TopViewController::studioControllerRecordingDidStart, this, _1));
    _studioController->setRecordingDidFinishFn(
        std::bind(&TopViewController::studioControllerRecordingDidFinish, this, _1));
    _studioController->setUpdateSequencerStatusFn(
        std::bind(&TopViewController::studioControllerUpdateSequencerStatus, this, _1));
    _studioController->setMajorTickFn(std::bind(&TopViewController::studioControllerMajorTick, this, _1));
    _studioController->setMinorTickFn(std::bind(&TopViewController::studioControllerMinorTick, this, _1));

    _view->horizSplitView()->setPosChangedFn(std::bind(&TopViewController::splitViewPosChanged, this, _1, _2));

    _view->studioView()->splitButton()->setClickedFn(std::bind(&TopViewController::splitClicked, this, _1));
    _view->studioView()->isSplittedButton()->setStateDidChangeFn(
        std::bind(&TopViewController::splitStateDidChange, this, _1, _2));
    _view->studioView()->masterLevelSlider()->setPosChangedFn(
        std::bind(&TopViewController::masterLevelValueDidChange, this, _1, _2));

    _view->recordButton()->setClickedFn(std::bind(&TopViewController::recordClicked, this, _1));
    _view->rewindButton()->setClickedFn(std::bind(&TopViewController::rewindClicked, this, _1));
    _view->playButton()->setClickedFn(std::bind(&TopViewController::playClicked, this, _1));
    _view->convertSequenceButton()->setClickedFn(std::bind(&TopViewController::convertSequenceClicked, this, _1));

    _view->setHandleEventFn(std::bind(&TopViewController::handleEvent, this, _1, _2));

    _sequenceEditor->setWillRemoveTrack(std::bind(&TopViewController::sequenceEditorWillRemoveTrack, this, _1, _2));
    _sequenceEditor->setDidModifySequenceFn(std::bind(&TopViewController::sequenceEditorDidModifySequence, this, _1));

    _preferencesViewController->setDidChangeAudioOutputSettingsFn(
        std::bind(&TopViewController::preferencesViewControllerDidChangeAudioOutputSettings, this, _1));
    _preferencesViewController->setDidChangeMIDIDestinationSettingsFn(
        std::bind(&TopViewController::preferencesViewControllerDidChangeMIDIDestinationSettings, this, _1));
    _preferencesViewController->setDidChangeAutoStopRecordSettingsFn(
        std::bind(&TopViewController::preferencesViewControllerDidChangeAutoStopRecordSettings, this, _1));
    _preferencesViewController->setDidChangeNbVoicesSettingsFn(
        std::bind(&TopViewController::preferencesViewControllerDidChangeNbVoicesSettings, this, _1));

    _octave = 5;
    _velocity = 0.5f;
    _sustainState = false;

    updateSplitControls();
    updateControlButtons();

    // Set the document name
    Platform::sharedInstance()->invoke([=] {
        Platform::sharedInstance()->setDocumentName(_view->untitledStr() + " " + folderPath());
        _sequenceViewController->setIsArrangementViewVisible(false);
    });

    // Set the initial position for the master level
    float masterLevelValue = 10.0f * log10f(_studioController->studio()->masterMixerLevel());
    _view->studioView()->masterLevelSlider()->setPos(masterLevelValue, false);

    Platform::sharedInstance()->invokeDelayed(
        this, [=] { autoSave(); }, TOP_VIEW_CONTROLLER_AUTO_SAVE_PERIOD);

    _isEditionInterationInProgress = false;

    _tempo = static_cast<int>(_studioController->studio()->metronome()->bpms().at(0).second);

    _timeSigNum = static_cast<int>(_studioController->studio()->metronome()->timeSignatures().at(0).second.first);
    _timeSigDenum = static_cast<int>(_studioController->studio()->metronome()->timeSignatures().at(0).second.second);

    auto controlsViewUI = _view->controlsViewUI();
    MDStudio::Property::bind(&_metronomeMode, controlsViewUI->findProperty("metronomeModeProperty").get());
    MDStudio::Property::bind(&_timeSigNum, controlsViewUI->findProperty("timeSigNumProperty").get());
    MDStudio::Property::bind(&_timeSigDenum, controlsViewUI->findProperty("timeSigDenumProperty").get());
    MDStudio::Property::bind(&_tempo, controlsViewUI->findProperty("tempoProperty").get());

    _tempo.setValueDidChangeFn(
        [this](MDStudio::Property* sender) { _studioController->setBPMAtCurrentTick(sender->value<int>()); });
    _didSetTempoFn = std::make_shared<MDStudio::Studio::didSetTempoFnType>(
        [this](MDStudio::Studio* sender, int source, double timestamp, UInt32 tick, UInt32 mpqn) {
            _tempo.setValue(static_cast<int>(60000000 / mpqn), false, true);
        });
    _studioController->studio()->addDidSetTempoFn(_didSetTempoFn);

    _didSetTimeSignatureFn = std::make_shared<MDStudio::Studio::didSetTimeSignatureFnType>(
        [this](MDStudio::Studio* sender, int source, double timestamp, UInt32 tick, unsigned int numerator,
               unsigned int denominator) {
            _timeSigNum.setValue(static_cast<int>(numerator), false, true);
            _timeSigDenum.setValue(static_cast<int>(denominator), false, true);
            _studioController->metronomeController()->setDefaultNumerator(numerator);
        });
    _studioController->studio()->addDidSetTimeSignatureFn(_didSetTimeSignatureFn);

    _timeSigNum.setValueDidChangeFn([this](MDStudio::Property* sender) {
        _studioController->setTimeSignatureAtCurrentTick(
            {static_cast<UInt32>(sender->value<int>()), static_cast<UInt32>(_timeSigDenum.value<int>())});
        _studioController->metronomeController()->setDefaultNumerator(static_cast<UInt32>(sender->value<int>()));
    });
    _timeSigDenum.setValueDidChangeFn([this](MDStudio::Property* sender) {
        _studioController->studio()->metronome()->setTimeSignatureAtCurrentTick(
            {static_cast<UInt32>(_timeSigNum.value<int>()), static_cast<UInt32>(sender->value<int>())});
    });

    if (_server) {
        _server->setDidRequestSaveFn([this](MelobaseCore::Server* sender) { saveSequenceIfModified(); });
    }

    _view->loadingBoxView()->setFillColor(MDStudio::makeColor(0.0f, 0.0f, 0.0f, 1.0f));
    MDStudio::Platform::sharedInstance()->invoke([this] {
        // Select the sequences folder
        _dbViewController->selectFolderWithID(SEQUENCES_FOLDER_ID);

        if (_isFadeInEnabled) {
            auto loadingAnimationPath = std::make_shared<MDStudio::LinearAnimationPath>(
                MDStudio::makePoint(1.0f, 0.0f), MDStudio::makePoint(0.0f, 0.0f), 3.0f, false,
                [this](MDStudio::Point pt) {
                    _view->loadingBoxView()->setFillColor(MDStudio::makeColor(0.0f, 0.0f, 0.0f, pt.x));
                });
            _loadingAnimation.addPath(std::move(loadingAnimationPath));
            _loadingAnimation.setAnimationDidFinishFn(
                [this](Animation* sender) { _view->loadingBoxView()->setIsVisible(false); });
            _loadingAnimation.start();
        } else {
            _view->loadingBoxView()->setIsVisible(false);
        }
    });
}

// ---------------------------------------------------------------------------------------------------------------------
TopViewController::~TopViewController() {
    Platform::sharedInstance()->cancelDelayedInvokes(this);

    saveSequenceIfModified();

    _studioController->studio()->removeDidSetTimeSignatureFn(_didSetTimeSignatureFn);
    _studioController->studio()->removeDidSetTempoFn(_didSetTempoFn);

    _view->aboutView()->removeAllSubviews();

    // If the about view is displayed, we remove it
    if (_view->hasSubview(_view->aboutView())) {
        _view->removeSubview(_view->aboutView());
    }

    // If we are currently learning the tempo, we stop
    if (_isLearningTempo) cancelLearnTempo();

    View* topView = _view->topView();

    if (topView->hasSubview(_view->learnTempoSheet())) topView->removeSubview(_view->learnTempoSheet());

    if (topView->hasSubview(_view->splitSheet())) topView->removeSubview(_view->splitSheet());

    _view->learnTempoSheet()->removeSubview(_view->learnTempoImageView());
    _view->learnTempoSheet()->removeSubview(_learnTempoArrowImageView[0]);
    _view->learnTempoSheet()->removeSubview(_learnTempoArrowImageView[1]);

    delete _midiImportViewController;
    delete _audioExportViewController;
    delete _preferencesViewController;
    delete _confirmSetAsPlayedAllViewController;
    delete _confirmEmptyTrashViewController;
    delete _aboutViewController;

    delete _sequenceViewController;
    delete _dbViewController;

    delete _lowerZoneViewController;
    delete _upperZoneViewController;

    delete _sequenceEditor;

    delete _midiImport;
    delete _audioExport;
}

// ---------------------------------------------------------------------------------------------------------------------
std::string TopViewController::folderPath() {
    auto selectedFolder = _dbViewController->selectedFolder();
    if (selectedFolder) return "[" + localizedFolderName(selectedFolder.get()) + "]";
    return "";
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::splitViewPosChanged(SplitViewH* sender, float pos) {
    //_view->dbView()->sequencesView()->listScrollView()->setContentSize(makeSize(std::max(_view->dbView()->sequencesView()->frame().size.width
    //- SCROLL_VIEW_SCROLL_BAR_THICKNESS, 350.0f), _view->dbView()->sequencesView()->listView()->contentHeight()));
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::splitClicked(Button* sender) {
    _isSettingSplitPosition = true;
    _isModalViewPresented = true;

    if (_menuStatesDidChangeFn) _menuStatesDidChangeFn(this);

    View* topView = _view->topView();

    topView->addSubview(_view->splitSheet());
    topView->setDirty();
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::splitStateDidChange(Button* sender, bool state) { _isKeyboardSplitted = state; }

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::splitCancelClicked(Button* sender) { cancelSetSplitPosition(); }

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::cancelLearnTempo() {
    if (!_isLearningTempo) return;

    _learnTempoArrowAnimation->stop();
    delete _learnTempoArrowAnimation;
    View* topView = _view->topView();
    topView->removeSubview(_view->learnTempoSheet());
    topView->setDirty();
    _metronomeArmed = false;
    _isLearningTempo = false;
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::recordClicked(Button* sender) { record(); }

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::rewindClicked(Button* sender) { goToBeginning(); }

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::playClicked(Button* sender) { playPause(); }

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::convertSequenceClicked(Button* sender) { convertSequence(); }

// ---------------------------------------------------------------------------------------------------------------------
std::string TopViewController::stringFromPitch(int pitch) {
    static const char* noteLetters[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    int octave = pitch / 12 - 2;
    int note = pitch % 12;

    return std::string(noteLetters[note]) + std::to_string(octave);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::updateSplitControls() {
    _view->studioView()->isSplittedButton()->setIsEnabled(_isMIDIDeviceAvailable);
    _view->studioView()->splitButton()->setIsEnabled(_isMIDIDeviceAvailable);
    _view->studioView()->isSplittedButton()->setState(_isKeyboardSplitted, false);
    _view->studioView()->splitLabelView()->setTitle(stringFromPitch(_splitPosition));
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::cancelSetSplitPosition() {
    View* topView = _view->topView();
    topView->removeSubview(_view->splitSheet());
    topView->setDirty();

    _isSettingSplitPosition = false;
    _isModalViewPresented = false;

    if (_menuStatesDidChangeFn) _menuStatesDidChangeFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
int TopViewController::zoneForPitch(int pitch) const {
    if (_isKeyboardSplitted) {
        if (pitch < _splitPosition) return 1;
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::noteOn(int channel, int pitch, Float32 velocity) {
    if (_isExportingAudio) return;

    bool playNote = false;

    // If we are setting the split position
    if (_isSettingSplitPosition) {
        MDStudio::Platform::sharedInstance()->invoke([=]() {
            _splitPosition = pitch;
            _isKeyboardSplitted = true;
            updateSplitControls();
            cancelSetSplitPosition();
        });
    } else if (_metronomeArmed) {  // If the studio is armed

        // If the studio is currently awaiting for the tempo
        if (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusAwaitingTempoOrNote) {
            MDStudio::Platform::sharedInstance()->invoke([=]() {
                // We keep the note pitch used as the major tap
                _metronomeLearningPitch = pitch;

                // We notify the studio controller of the first beat
                _studioController->tapFirstBeat();
            });
        } else if (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusLearningTempo) {
            MDStudio::Platform::sharedInstance()->invoke([=]() {
                // The studio is learning the tempo

                // If the pitch is different than the first one
                if (pitch != _metronomeLearningPitch) {
                    // We notify an other beat tap
                    _studioController->tapOtherBeat();
                } else {
                    // We notify a first beat tap
                    _studioController->tapFirstBeat();
                    _metronomeLearningPitch = -1;
                }
            });
        } else {
            // We play the note
            playNote = true;
        }
    } else {
        // We play the note
        playNote = true;
    }

    // If we need to play the note, we do so
    if (playNote) {
        if (channel == TOP_VIEW_CONTROLLER_MASTER_CHANNEL) {
            int zone = zoneForPitch(pitch);
            if (zone == 0) {
                pitch += _upperZoneViewController->transposeValue();
                channel = _upperZoneViewController->channel();
            } else {
                pitch += _lowerZoneViewController->transposeValue();
                channel = _lowerZoneViewController->channel();
            }
        }
        _studioController->keyPressed(pitch, velocity, channel);
        MDStudio::Platform::sharedInstance()->invoke([=]() {
            _view->sequenceView()->pianoRollView()->mainView()->pianoRollEventsView()->setHighlightPitchState(
                channel, pitch, true);
        });
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::noteOff(int channel, int pitch, Float32 velocity) {
    if (_isExportingAudio) return;

    // If the studio is not armed or if not learning the tempo
    if (!_metronomeArmed ||
        !(_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusAwaitingTempoOrNote ||
          _studioController->status() == MelobaseCore::StudioController::StudioControllerStatusLearningTempo)) {
        if (channel == TOP_VIEW_CONTROLLER_MASTER_CHANNEL) {
            // We release the note
            int zone = zoneForPitch(pitch);
            if (zone == 0) {
                pitch += _upperZoneViewController->transposeValue();
                channel = _upperZoneViewController->channel();
            } else {
                pitch += _lowerZoneViewController->transposeValue();
                channel = _lowerZoneViewController->channel();
            }
        }
        _studioController->keyReleased(pitch, velocity, channel);
        MDStudio::Platform::sharedInstance()->invoke([=]() {
            _view->sequenceView()->pianoRollView()->mainView()->pianoRollEventsView()->setHighlightPitchState(
                channel, pitch, false);
        });
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::keyAftertouch(int channel, int pitch, Float32 value) {
    if (_isExportingAudio) return;

    _studioController->keyAftertouch(pitch, value, channel);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::channelAftertouch(int channel, Float32 value) {
    if (_isExportingAudio) return;

    _studioController->channelAftertouch(value, channel);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::programChange(int channel, int program) {
    if (_isExportingAudio) return;

    _studioController->setInstrument(
        STUDIO_INSTRUMENT_FROM_PRESET(0, program),
        channel == TOP_VIEW_CONTROLLER_MASTER_CHANNEL ? _upperZoneViewController->channel() : channel);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::sustainChanged(int channel, Float32 value) {
    if (_isExportingAudio) return;

    _studioController->setSustain(
        value, channel == TOP_VIEW_CONTROLLER_MASTER_CHANNEL ? _upperZoneViewController->channel() : channel);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::pitchBendChanged(int channel, Float32 value) {
    if (_isExportingAudio) return;

    _studioController->setPitchBend(
        value, channel == TOP_VIEW_CONTROLLER_MASTER_CHANNEL ? _upperZoneViewController->channel() : channel);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::modulationChanged(int channel, Float32 value) {
    if (_isExportingAudio) return;

    _studioController->setModulation(
        value, channel == TOP_VIEW_CONTROLLER_MASTER_CHANNEL ? _upperZoneViewController->channel() : channel);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::volumeChanged(int channel, Float32 value) {
    if (_isExportingAudio) return;

    _studioController->setMixerLevel(
        value, channel == TOP_VIEW_CONTROLLER_MASTER_CHANNEL ? _upperZoneViewController->channel() : channel);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::panChanged(int channel, Float32 value) {
    if (_isExportingAudio) return;

    _studioController->setMixerBalance(
        value, channel == TOP_VIEW_CONTROLLER_MASTER_CHANNEL ? _upperZoneViewController->channel() : channel);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::controlChanged(int channel, UInt32 control, UInt32 value) {
    if (_isExportingAudio) return;

    _studioController->setControlValue(
        control, value, channel == TOP_VIEW_CONTROLLER_MASTER_CHANNEL ? _upperZoneViewController->channel() : channel);
}

// =====================================================================================================================
// Studio controller delegates
//

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::studioControllerRecordingDidStart(MelobaseCore::StudioController* studioController) {
    if (_isRecordingNewSequence) {
        cancelLearnTempo();

        // We save right away the modifications if any
        saveSequenceIfModified();

        _view->dbView()->sequencesView()->tableView()->setSelectedRow(-1, false);

        // Set the document name
        Platform::sharedInstance()->setDocumentName(_view->untitledStr() + " " + folderPath());

        _sequenceViewController->arrangementViewController()->setSequence(studioController->sequence());
        _sequenceViewController->pianoRollViewController()->setSequence(studioController->sequence());
    }

    // Show or hide the arrangement view based on data format
    if (_studioController->sequence() &&
        (_studioController->sequence()->data.format == SEQUENCE_DATA_FORMAT_MULTI_TRACK)) {
        _sequenceViewController->setIsArrangementViewVisible(true);
        _sequenceViewController->arrangementViewController()->updateTracks();
    } else {
        _sequenceViewController->setIsArrangementViewVisible(false);
        _sequenceViewController->pianoRollViewController()->setTrackIndex(!_studioController->sequence()->data.tracks.empty() ? 0 : -1);
    }

    // Show the split view
    _view->horizSplitView()->setIsVisible(true);

    // Collapse the sequence list
    _wasDatabaseVisible = _view->dbView()->isVisible();
    _view->horizSplitView()->setLeftPaneVisibility(false);

    // Disable the edition
    _sequenceViewController->pianoRollViewController()->setIsEditionAvailable(false);

    // Update the track controls
    _sequenceViewController->arrangementViewController()->updateTrackControls();
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::studioControllerRecordingDidFinish(MelobaseCore::StudioController* studioController) {
    std::vector<int> armedTrackIndices = _studioController->sequencer()->armedTrackIndices();

    // Enable the edition
    _sequenceViewController->pianoRollViewController()->setIsEditionAvailable(
        _sequenceViewController->pianoRollViewController()->trackIndex() >= 0);

    if (_isRecordingNewSequence || _wasDatabaseVisible) _view->horizSplitView()->setLeftPaneVisibility(true);

    if (!_isRecordingNewSequence) {
        // Only update the tracks in the sequence editor
        _sequenceEditor->undoManager()->beginGroup();
        auto newSequence = MelobaseCore::getMelobaseCoreSequence(_studioController->sequencer()->sequence());
        for (auto i : armedTrackIndices) {
            std::shared_ptr<MelobaseCore::Track> track = _sequenceEditor->sequence()->data.tracks[i];
            _sequenceEditor->setTrackEvents(track, newSequence->data.tracks[i]->clips[0]->events);
        }
        _sequenceEditor->undoManager()->endGroup();

        std::vector<int> emptyArmedTrackIndices;
        _studioController->sequencer()->setArmedTrackIndices(emptyArmedTrackIndices);

        updateControlButtons();

    } else {
        // Set the whole new sequence
        _sequenceEditor->setSequence(studioController->sequence());
        _view->dbView()->sequencesView()->tableView()->setSelectedRow(_lastSelectedRow);
    }

    _sequenceViewController->arrangementViewController()->setSequence(studioController->sequence());
    _sequenceViewController->pianoRollViewController()->setSequence(studioController->sequence());

    if (!_isRecordingNewSequence) {
        // Select the track if we only had a single armed track
        if (armedTrackIndices.size() == 1)
            _sequenceViewController->arrangementViewController()->selectTrack(armedTrackIndices[0]);
        _isRecordingNewSequence = true;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::studioControllerSaveSequences(MelobaseCore::StudioController* studioController) {
    if (_isRecordingNewSequence) {
        _studioController->updateSequence();
        _sequencesDB->addSequence(_studioController->sequence());
        _view->dbView()->sequencesView()->nameSearchField()->setText("");
        _view->dbView()->sequencesView()->filterSegmentedControl()->setSelectedSegment(0);
        int row = rowOfSequenceWithID(_studioController->sequence()->id);
        _view->dbView()->sequencesView()->tableView()->setSelectedRow(row);
        _lastSelectedRow = row;
    }

    _studioController->studio()->playAudioFile(
        MDStudio::Platform::sharedInstance()->resourcesPath() + "/" + "Beep.aiff", 0.2f);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::studioControllerUpdateSequencerStatus(MelobaseCore::StudioController* studioController) {
    switch (studioController->status()) {
        case MelobaseCore::StudioController::StudioControllerStatusPlaying:
            // We remember that this sequence was played in order to eventually increase the play counter
            _lastSequenceWasPlayed = true;
            _view->playButton()->setImage(_view->pauseImage());
            _view->statusImageView()->setImage(_view->playingImage());
            break;

        case MelobaseCore::StudioController::StudioControllerStatusAwaitingTempoOrNote:

            // Since we do not provide any UI to set the pitch bend range, we reset it automatically
            resetPitchBendRange();

            _view->playButton()->setImage(_view->playImage());
            _learnTempoArrowImageView[0]->setIsVisible(true);
            _learnTempoArrowImageView[1]->setIsVisible(false);
            _view->learnTempoLabelView()->setTitle(_view->learnTempoStr(0));
            _view->statusImageView()->setImage(nullptr);
            if (_isReloadSequencePending) {
                _isReloadSequencePending = false;
                _dbViewController->reloadSequences(true);
            }

            break;

        case MelobaseCore::StudioController::StudioControllerStatusLearningTempo:
            _view->statusImageView()->setImage(_view->metronomeLearningImage());
            break;

        case MelobaseCore::StudioController::StudioControllerStatusRecording:
            _view->statusImageView()->setImage(_view->recordingImage());
            break;
    }

    _sequenceViewController->updateControls();
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::studioControllerMajorTick(MelobaseCore::StudioController* studioController) {
    _learnTempoArrowImageView[0]->setIsVisible(true);
    _learnTempoArrowImageView[1]->setIsVisible(true);
    _view->learnTempoLabelView()->setTitle(_view->learnTempoStr(1));
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::studioControllerMinorTick(MelobaseCore::StudioController* studioController) {
    _learnTempoArrowImageView[0]->setIsVisible(true);
    _learnTempoArrowImageView[1]->setIsVisible(true);
    _view->learnTempoLabelView()->setTitle(_view->learnTempoStr(2));
}

// =====================================================================================================================
// Sequences DB delegates
//

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::sequencesDBSequenceAdded(MelobaseCore::SequencesDB* sequencesDB) {
    if (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusPlaying) {
        _isReloadSequencePending = true;
    } else {
        _dbViewController->reloadSequences(true);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::sequencesDBFolderAdded(MelobaseCore::SequencesDB* sequencesDB,
                                               std::shared_ptr<MelobaseCore::SequencesFolder> folder) {
    _dbViewController->reloadFolders(true);
    if (_dbViewController->isAddingNewFolder()) _dbViewController->selectFolderWithID(folder->id, true);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::sequencesDBWillRemoveSequence(MelobaseCore::SequencesDB* sequenceDB,
                                                      std::shared_ptr<MelobaseCore::Sequence> sequence) {
    // If the last sequence is the one being deleted, we discard the changes
    if (_lastSequence) {
        if (_lastSequence->id == sequence->id) {
            _isSequenceModified = false;
            _areMetaDataChanged = false;
            _areDataChanged = false;
            _isSequenceSaved = true;
            _lastSequence = nullptr;
            _lastSequenceWasPlayed = false;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::sequencesDBSequenceRemoved(MelobaseCore::SequencesDB* sequencesDB) {
    _dbViewController->reloadSequences(false);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::sequencesDBFolderRemoved(MelobaseCore::SequencesDB* sequencesDB) {
    _dbViewController->reloadFolders(true);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::sequencesDBSequenceUpdated(MelobaseCore::SequencesDB* sequencesDB) {
    if (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusPlaying) {
        _isReloadSequencePending = true;
    } else {
        _dbViewController->reloadSequences(true);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::sequencesDBFolderUpdated(MelobaseCore::SequencesDB* sequencesDB) {
    _dbViewController->reloadFolders(true);
}

// =====================================================================================================================
// Undo manager delegates
//

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::undoManagerStatesDidChange(MDStudio::UndoManager* sender) {
    if (_menuStatesDidChangeFn) _menuStatesDidChangeFn(this);
}

// =====================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::autoSave() {
    if (_isSequenceModified) {
        if (_lastSequence) {
            if (!_isSequenceSaved) {
                _sequencesDB->undoManager()->disableRegistration();
                _sequencesDB->updateSequences({_lastSequence}, false, _areMetaDataChanged, _areDataChanged,
                                              _areDataChanged);
                _sequencesDB->undoManager()->enableRegistration();
                _isSequenceSaved = true;
            }
        }
    }
    Platform::sharedInstance()->invokeDelayed(
        this, [=] { autoSave(); }, TOP_VIEW_CONTROLLER_AUTO_SAVE_PERIOD);
}

// ---------------------------------------------------------------------------------------------------------------------
// Note: The undo stack will be cleared if the sequence is saved
void TopViewController::saveSequenceIfModified() {
    if (_isSequenceModified) {
        _isSequenceModified = false;
        if (_lastSequence) {
            _sequencesDB->updateSequences({_lastSequence}, false, _areMetaDataChanged, _areDataChanged,
                                          _areDataChanged);
            if (_areMetaDataChanged)
                Platform::sharedInstance()->invoke([=] { _dbViewController->reloadSequences(true); });
            _isSequenceSaved = true;
        }
        _sequencesDB->undoManager()->clear();
        _areMetaDataChanged = false;
        _areDataChanged = false;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::increaseLastSequencePlayCount() {
    // If we did not record a new, we do nothing in order to not reload the sequences
    if (!_isRecordingNewSequence) return;

    // If the last sequence was played
    if (_lastSequenceWasPlayed) {
        _lastSequenceWasPlayed = false;
        // We increase the play counter of the last sequence
        _lastSequence->playCount++;
        _sequencesDB->undoManager()->disableRegistration();
        _sequencesDB->updateSequences({_lastSequence}, false);
        _sequencesDB->undoManager()->enableRegistration();
        Platform::sharedInstance()->invoke([=] { _dbViewController->reloadSequences(true); });
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::setSequence(std::shared_ptr<MelobaseCore::Sequence> sequence) {
    increaseLastSequencePlayCount();

    saveSequenceIfModified();

    std::vector<int> armedTrackIndices;
    _studioController->sequencer()->setArmedTrackIndices(armedTrackIndices);
    _isRecordingNewSequence = true;

    std::vector<int> muteTrackIndices;
    _studioController->sequencer()->setMuteTrackIndices(muteTrackIndices);

    _studioController->sequencer()->setSoloTrackIndex(-1);

    _studioController->setSequence(sequence);
    _sequenceViewController->arrangementViewController()->setSequence(sequence);
    _sequenceViewController->pianoRollViewController()->setSequence(sequence);
    _sequenceViewController->pianoRollViewController()->setIsEditionAvailable(
        sequence ? (_sequenceViewController->pianoRollViewController()->trackIndex() >= 0) : false);

    // Set the document name
    std::string name = _view->untitledStr();
    if (sequence && !sequence->name.empty()) name = sequence->name;

    Platform::sharedInstance()->setDocumentName(name + " " + folderPath());

    goToBeginning();

    _lastSequence = sequence;

    if (_menuStatesDidChangeFn) _menuStatesDidChangeFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::newSequence() {
    std::pair<UInt32, UInt32> timeSignature = {4, 4};
    int bpm = 120;

    if (_studioController->studio()->metronome()->timeSignatures().size() > 0)
        timeSignature = _studioController->studio()->metronome()->timeSignatures()[0].second;

    if (_studioController->studio()->metronome()->bpms().size() > 0)
        bpm = _studioController->studio()->metronome()->bpms()[0].second;

    _newSequenceViewController->setName("");
    _newSequenceViewController->setTimeSignatureNum(timeSignature.first);
    _newSequenceViewController->setTimeSignatureDenum(timeSignature.second);
    _newSequenceViewController->setBPM(bpm);

    _newSequenceViewController->showModal();
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::newSubfolder() { _dbViewController->newSubfolder(); }

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::convertSequence() {
    if (_studioController->sequence()) {
        // We ensure that the undo stack begins at the first edition action
        if (!_isSequenceModified) _sequencesDB->undoManager()->clear();

        if (_studioController->sequence()->data.format == SEQUENCE_DATA_FORMAT_SINGLE_TRACK) {
            _sequenceEditor->convertToMultiTrack();
            _sequenceViewController->pianoRollViewController()->setTrackIndex(-1);
            _sequenceViewController->arrangementViewController()->updateTrackControls();
        } else if (_studioController->sequence()->data.format == SEQUENCE_DATA_FORMAT_MULTI_TRACK) {
            // Deselect the current track
            _sequenceViewController->arrangementViewController()->clearTrackSelection();
            _sequenceEditor->convertToSingleTrack();
            _sequenceViewController->pianoRollViewController()->setTrackIndex(!_studioController->sequence()->data.tracks.empty() ? 0 : -1);
        }
    }

    updateControlButtons();

    if (_menuStatesDidChangeFn) _menuStatesDidChangeFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::deleteSequence() {
    std::shared_ptr<MelobaseCore::Sequence> seq = sequence();
    if (seq) {
        _view->dbView()->sequencesView()->tableView()->setSelectedRow(-1);
        _sequencesDB->removeSequence(seq);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::moveToTrashSequence() {
    _studioController->stop();

    saveSequenceIfModified();

    // If the list of sequences has the focus
    if (_view->dbView()->sequencesView()->tableView()->hasFocus()) {
        // Move to trash the selected sequence(s)

        std::vector<std::shared_ptr<MelobaseCore::Sequence>> selectedSequences = _selectedSequences;

        auto it = selectedSequences.begin();
        for (; it != selectedSequences.end(); ++it) {
            auto sequence = *it;
            sequence->folder = _dbViewController->trashFolder();
        }
        _sequencesDB->updateSequences(selectedSequences);

    } else if (_view->dbView()->foldersView()->treeView()->hasFocus()) {
        // Move to trash the selected folder

        auto selectedFolder = _studioController->currentFolder();
        if (selectedFolder && selectedFolder->id != TRASH_FOLDER_ID && selectedFolder->id != SEQUENCES_FOLDER_ID) {
            selectedFolder->parentID = TRASH_FOLDER_ID;
            selectedFolder->parent = _dbViewController->trashFolder();
            _sequencesDB->updateFolder(selectedFolder);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::promoteAllSequences() {
    _studioController->stop();
    saveSequenceIfModified();
    if (!isDatabaseVisible()) showHideDatabase();
    _sequencesDB->promoteAllSequences(_dbViewController->selectedFolder(), _dbViewController->isRecursive());
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::demoteAllSequences() {
    _studioController->stop();
    saveSequenceIfModified();
    if (!isDatabaseVisible()) showHideDatabase();
    _sequencesDB->demoteAllSequences(_dbViewController->selectedFolder(), _dbViewController->isRecursive());
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::setAsPlayedAllSequences() {
    _studioController->stop();
    saveSequenceIfModified();
    if (!isDatabaseVisible()) showHideDatabase();
    _confirmSetAsPlayedAllViewController->showModal();
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::cleanupSequences() {
    _studioController->stop();
    saveSequenceIfModified();
    if (!isDatabaseVisible()) showHideDatabase();
    _confirmCleanupViewController->showModal();
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::emptyTrash() {
    _studioController->stop();
    saveSequenceIfModified();
    _confirmEmptyTrashViewController->showModal();
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::importSequences(std::vector<std::string> paths) {
    _midiImport->importMIDI(paths, _dbViewController->selectedFolder());
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::exportSequence(std::string path) {
    if (sequence()) {
        _studioController->stop();

        saveSequenceIfModified();

        std::shared_ptr<MDStudio::Sequence> studioSequence = MelobaseCore::getStudioSequence(sequence());
        MDStudio::writeMIDIFile(path, studioSequence);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::exportAudio(std::string path) { _audioExport->exportAudio(path); }

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::setMetronomeSound(bool state) {
    _studioController->setIsMetronomeSoundEnabledDuringPlayback(state);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::playPause() {
    if (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusRecording) {
        _studioController->stopRecordingAndMetronome();
    } else if (!_isLearningTempo) {
        _studioController->playPause();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::stop() {
    if (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusRecording) {
        _studioController->stopRecordingAndMetronome();
    } else if (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusPlaying) {
        _studioController->playPause();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::record() {
    if (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusRecording) {
        _studioController->stopRecordingAndMetronome();
    } else {
        if (!_metronomeArmed) {
            // If we are playing, we stop the playback
            if (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusPlaying)
                _studioController->playPause();
            _studioController->stopAllNotes();

            // Preserve the last selected row in order to restore it if necessary
            _lastSelectedRow = _view->dbView()->sequencesView()->tableView()->selectedRow();

            // Hide the split view
            _view->horizSplitView()->setIsVisible(false);

            // If one or more tracks are armed
            if (_studioController->sequence() && !_studioController->sequencer()->armedTrackIndices().empty()) {
                _isRecordingNewSequence = false;

                // We ensure that the undo stack begins at the first edition action
                if (!_isSequenceModified) _sequencesDB->undoManager()->clear();

                // Select the track
                if (_studioController->sequencer()->armedTrackIndices().size() == 1) {
                    _sequenceViewController->arrangementViewController()->selectTrack(
                        _studioController->sequencer()->armedTrackIndices()[0]);
                } else {
                    _sequenceViewController->arrangementViewController()->clearTrackSelection();
                }

                // Record
                _studioController->startRecording(_dbViewController->selectedFolder());

                // Update the control buttons
                updateControlButtons();
            } else {
                _isRecordingNewSequence = true;

                increaseLastSequencePlayCount();

                if (_metronomeMode.value<int>() == metronomeModeTapTempo) {
                    // Learn the tempo

                    _isLearningTempo = true;
                    _metronomeArmed = true;

                    View* topView = _view->topView();
                    Rect r = _view->learnTempoSheet()->frame();

                    _learnTempoArrowAnimation = new Animation();

                    auto initialPt0 = makePoint(r.size.width / 2.0f + 25.0f, r.size.height - 50.0f);
                    auto initialPt1 = makePoint(r.size.width / 2.0f - 25.0f, r.size.height - 50.0f);

                    std::array<std::shared_ptr<MDStudio::LinearAnimationPath>, 2> learnTempoArrowAnimationPaths;
                    learnTempoArrowAnimationPaths[0] = std::make_shared<MDStudio::LinearAnimationPath>(
                        initialPt0, makePoint(r.size.width - 290.0f, 110.0f), 50.0f, true, [=](Point pt) {
                            MDStudio::Platform::sharedInstance()->invoke([=] {
                                _learnTempoArrowImageView[0]->setFrame(
                                    makeRect(floorf(pt.x - 50.0f), floorf(pt.y - 50.0f), 100.0f, 100.0f));
                                _learnTempoArrowImageView[0]->setDirty();
                            });
                        });
                    learnTempoArrowAnimationPaths[1] = std::make_shared<MDStudio::LinearAnimationPath>(
                        initialPt1, makePoint(290.0f, 110.0f), 50.0f, true, [=](Point pt) {
                            MDStudio::Platform::sharedInstance()->invoke([=] {
                                _learnTempoArrowImageView[1]->setFrame(
                                    makeRect(floorf(pt.x - 50.0f), floorf(pt.y - 50.0f), 100.0f, 100.0f));
                                _learnTempoArrowImageView[1]->setDirty();
                            });
                        });

                    _learnTempoArrowImageView[0]->setFrame(
                        makeRect(floorf(initialPt0.x - 50.0f), floorf(initialPt0.y - 50.0f), 100.0f, 100.0f));
                    _learnTempoArrowImageView[1]->setFrame(
                        makeRect(floorf(initialPt1.x - 50.0f), floorf(initialPt1.y - 50.0f), 100.0f, 100.0f));

                    topView->addSubview(_view->learnTempoSheet());
                    topView->setDirty();

                    _learnTempoArrowAnimation->addPath(std::move(learnTempoArrowAnimationPaths[0]));
                    _learnTempoArrowAnimation->addPath(std::move(learnTempoArrowAnimationPaths[1]));
                    _learnTempoArrowImageView[0]->setIsVisible(true);
                    _learnTempoArrowImageView[1]->setIsVisible(false);
                    _view->learnTempoLabelView()->setTitle(_view->learnTempoStr(0));
                    _learnTempoArrowAnimation->start();
                } else {
                    // Record
                    Platform::sharedInstance()->invoke([=] {
                        if (_metronomeMode.value<int>() == metronomeModeSpecified) {
                            _studioController->studio()->metronome()->setBPMs({{0, _tempo.value<int>()}});
                            _studioController->studio()->metronome()->setTimeSignatures(
                                {{0, {_timeSigNum.value<int>(), _timeSigDenum.value<int>()}}});
                        } else {
                            _studioController->studio()->metronome()->setBPMs({});
                            _studioController->studio()->metronome()->setTimeSignatures({});
                        }

                        _studioController->startRecording(_dbViewController->selectedFolder());
                    });
                }

                // Update the control buttons
                updateControlButtons();
            }
        } else {
            _studioController->stopRecordingAndMetronome();
            cancelLearnTempo();

            // Show the split view
            _view->horizSplitView()->setIsVisible(true);

            // Enable the rewind and play buttons if a sequence is selected
            updateControlButtons();
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::goToBeginning() {
    // If we are playing, we stop the playback
    if (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusPlaying)
        _studioController->playPause();

    _studioController->goToBeginning();
}

// ---------------------------------------------------------------------------------------------------------------------
bool TopViewController::handleEvent(TopView* sender, const UIEvent* event) {
    static std::vector<char> pianoKeys = {'a', 'w', 's', 'e', 'd', 'f', 't', 'g', 'y', 'h', 'u', 'j'};

    if (event->type == CUT_UIEVENT) {
        editCut();
    } else if (event->type == COPY_UIEVENT) {
        editCopy();
    } else if (event->type == PASTE_UIEVENT) {
        editPaste();
    } else if (event->type == KEY_UIEVENT) {
        if (_metronomeArmed && (event->key == KEY_RIGHT)) {
            _studioController->tapFirstBeat();
            return true;
        } else if (_metronomeArmed && (event->key == KEY_LEFT)) {
            _studioController->tapOtherBeat();
            return true;
        } else if (event->key == KEY_ENTER) {
            if (_view->dbView()->sequencesView()->tableView()->hasFocus()) {
                // If a sequence is selected
                int selectedRow = _view->dbView()->sequencesView()->tableView()->selectedRow();
                if (selectedRow >= 0) {
                    // Get the sequence item view
                    std::shared_ptr<SequenceListItemView> sequenceListItemView =
                        std::dynamic_pointer_cast<SequenceListItemView>(
                            _view->dbView()->sequencesView()->tableView()->viewAtRow(selectedRow));
                    sequenceListItemView->startNameEdition();
                }
                return true;
            } else if (_view->dbView()->foldersView()->treeView()->hasFocus()) {
                // If a folder is selected
                std::vector<int> selectedRow = _view->dbView()->foldersView()->treeView()->selectedRow();
                if (selectedRow.size() > 0) {
                    // Get the folder item view
                    std::shared_ptr<FolderListItemView> folderListItemView =
                        std::dynamic_pointer_cast<FolderListItemView>(
                            _view->dbView()->foldersView()->treeView()->viewAtIndexPath(selectedRow));
                    folderListItemView->startNameEdition();
                }
                return true;
            } else if (_view->sequenceView()->arrangementView()->trackHeadersListView()->hasFocus()) {
                // If a track header is selected
                int selectedRow = _view->sequenceView()->arrangementView()->trackHeadersListView()->selectedRow();
                if (selectedRow >= 0) {
                    std::shared_ptr<TrackHeaderView> trackHeaderView =
                        std::dynamic_pointer_cast<TrackHeaderView>(_view->sequenceView()
                                                                       ->arrangementView()
                                                                       ->trackHeadersOverlayListView()
                                                                       ->subviews()[selectedRow]);
                    trackHeaderView->startNameEdition();
                }
                return true;
            }
        } else if (event->characters.size() > 0) {
            char key = std::tolower(event->characters[0]);
            auto f = std::find(pianoKeys.begin(), pianoKeys.end(), key);
            if (f != pianoKeys.end()) {
                if (!event->isARepeat) {
                    unsigned int pitch = static_cast<unsigned int>(f - pianoKeys.begin()) + 12 * _octave;
                    noteOn(TOP_VIEW_CONTROLLER_MASTER_CHANNEL, pitch, _velocity);
                }
                return true;
            } else if (key == 'z') {
                if (!event->isARepeat) {
                    if (_octave > 0) _octave--;
                }
                return true;
            } else if (key == 'x') {
                if (!event->isARepeat) {
                    if (_octave < 9) _octave++;
                }
                return true;
            } else if (key == 'c') {
                if (!event->isARepeat) {
                    if (_velocity >= 0.1f) _velocity -= 0.1f;
                }
                return true;
            } else if (key == 'v') {
                if (!event->isARepeat) {
                    if (_velocity <= 0.9f) _velocity += 0.1f;
                }
                return true;
            } else if (key == 'q') {
                if (!event->isARepeat) {
                    _sustainState = !_sustainState;
                    sustainChanged(TOP_VIEW_CONTROLLER_MASTER_CHANNEL, _sustainState ? 1.0f : 0.0f);
                }
                return true;
            } else if (key == ' ') {
                playPause();
                return true;
            } else if (key == '`' || key == '~') {
                // Start main script if available
                std::string path = MDStudio::Platform::sharedInstance()->dataPath() + "/Main.lua";

                if (isFileExist(path)) {
                    _uiScriptModule = std::make_unique<MDStudio::UIScriptModule>(_view.get());
                    _melobaseCoreScriptModule = std::make_unique<MelobaseCore::MelobaseCoreScriptModule>();
                    _melobaseScriptModule = std::make_unique<MelobaseScriptModule>(_sequencesDB, _sequenceEditor);
                    _mainScript = std::make_unique<MDStudio::Script>();
                    _mainScript->execute(
                        path, {_uiScriptModule.get(), _melobaseCoreScriptModule.get(), _melobaseScriptModule.get()},
                        key == '~');
                }
                return true;
            }
        }
    } else if (event->type == KEY_UP_UIEVENT) {
        if (event->characters.size() > 0) {
            char key = std::tolower(event->characters[0]);
            auto f = std::find(pianoKeys.begin(), pianoKeys.end(), key);
            if (f != pianoKeys.end()) {
                if (!event->isARepeat) {
                    unsigned int pitch = static_cast<unsigned int>(f - pianoKeys.begin()) + 12 * _octave;
                    noteOff(TOP_VIEW_CONTROLLER_MASTER_CHANNEL, pitch, 0.5f);
                }
                return true;
            } else if ((key == 'z') || (key == 'x') || (key == 'c') || (key == 'v') || key == 'q')
                return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::undo() { _sequencesDB->undoManager()->undo(); }

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::redo() { _sequencesDB->undoManager()->redo(); }

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::setSequenceRating(float rating) {
    _studioController->stop();

    saveSequenceIfModified();

    std::vector<std::shared_ptr<MelobaseCore::Sequence>> selectedSequences = _selectedSequences;

    auto it = selectedSequences.begin();
    for (; it != selectedSequences.end(); ++it) {
        auto sequence = *it;
        sequence->rating = rating;
        if (sequence->playCount == 0) sequence->playCount = 1;
    }
    _sequencesDB->updateSequences(selectedSequences);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::updateControlButtons() {
    if ((_studioController->status() != MelobaseCore::StudioController::StudioControllerStatusRecording) &&
        !_isLearningTempo && (_studioController->sequence() != nullptr)) {
        _view->rewindButton()->setIsEnabled(true);
        _view->playButton()->setIsEnabled(true);
        _view->convertSequenceButton()->setIsEnabled(true);
    } else {
        _view->rewindButton()->setIsEnabled(false);
        _view->playButton()->setIsEnabled(false);
        _view->convertSequenceButton()->setIsEnabled(false);
    }

    _view->recordButton()->setIsEnabled(_dbViewController->selectedFolder() &&
                                        !_dbViewController->isSelectedFolderTrash());

    if (_studioController->sequence()) {
        if (_studioController->sequence()->data.format == SEQUENCE_DATA_FORMAT_SINGLE_TRACK) {
            _view->convertSequenceButton()->setImage(_view->convertToMTImage());
        } else if (_studioController->sequence()->data.format == SEQUENCE_DATA_FORMAT_MULTI_TRACK) {
            _view->convertSequenceButton()->setImage(_view->convertToSTImage());
        }
    } else {
        _view->convertSequenceButton()->setImage(_view->convertToMTImage());
    }

    if (_menuStatesDidChangeFn) _menuStatesDidChangeFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::updateSequencer() {
    auto tick = _studioController->studio()->metronome()->tick();
    _studioController->setSequence(_studioController->sequence());
    _studioController->studio()->metronome()->moveToTick(tick);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::setIsMIDIDeviceAvailable(bool isAvailable) {
    _isMIDIDeviceAvailable = isAvailable;
    updateSplitControls();
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::about() { _aboutViewController->showModal(); }

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::showPreferences() { _preferencesViewController->showModal(); }

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::dumpEvents() {
    for (auto e : _studioController->sequence()->data.tracks[0]->clips[0]->events) {
        auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(e);
        std::cout << "Type: " << (int)channelEvent->type() << " Channel: " << (int)channelEvent->channel()
                  << " P1: " << channelEvent->param1() << " P2: " << channelEvent->param2() << std::endl;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::editSelectAll() { _view->responderChain()->selectAll(); }

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::editDelete() {
    auto ret = _sequenceViewController->arrangementViewController()->deleteSelectedTracks();
    if (!ret) ret = _sequenceViewController->pianoRollViewController()->deleteSelectedEvents();

    if (!ret) MDStudio::Platform::sharedInstance()->beep();
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::editQuantize() { _sequenceViewController->pianoRollViewController()->quantizeSelectedEvents(); }

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::editCopy() {
    auto ret = _sequenceViewController->arrangementViewController()->copySelectedTracks();
    if (!ret) ret = _sequenceViewController->pianoRollViewController()->copySelectedEvents();

    if (!ret) MDStudio::Platform::sharedInstance()->beep();
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::editCut() {
    auto ret = _sequenceViewController->arrangementViewController()->cutSelectedTracks();
    if (!ret) ret = _sequenceViewController->pianoRollViewController()->cutSelectedEvents();

    if (!ret) MDStudio::Platform::sharedInstance()->beep();
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::editPaste() {
    auto ret = _sequenceViewController->arrangementViewController()->pasteTracks();
    if (!ret) ret = _sequenceViewController->pianoRollViewController()->pasteEvents();

    if (!ret) MDStudio::Platform::sharedInstance()->beep();
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::arrangementViewControllerWillModifySequence(ArrangementViewController* sender) {
    // We ensure that the undo stack begins at the first edition action
    if (!_isSequenceModified) _sequencesDB->undoManager()->clear();
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::pianoRollViewControllerWillModifySequence(PianoRollViewController* sender) {
    // We ensure that the undo stack begins at the first edition action
    if (!_isSequenceModified) _sequencesDB->undoManager()->clear();

    _isEditionInterationInProgress = true;
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::pianoRollViewControllerDidModifySequence(PianoRollViewController* sender) {
    _isEditionInterationInProgress = false;

    // Update the properties and the event list
    _sequenceViewController->pianoRollViewController()->updateProperties();
    _sequenceViewController->pianoRollViewController()->updateEventList();

    // Update the sequencer
    updateSequencer();
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::pianoRollViewControllerDidSelectEvents(PianoRollViewController* sender) {
    if (_menuStatesDidChangeFn) _menuStatesDidChangeFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::sequenceEditorWillRemoveTrack(MelobaseCore::SequenceEditor* sender,
                                                      std::shared_ptr<MelobaseCore::Track> track) {
    _sequenceViewController->pianoRollViewController()->setTrackIndex(-1);

    // Find the track index
    auto sequence = _studioController->sequence();
    auto it = std::find(sequence->data.tracks.begin(), sequence->data.tracks.end(), track);
    assert(it != sequence->data.tracks.end());
    size_t trackIndex = it - sequence->data.tracks.begin();

    // Make sure that the track is no longer armed at the sequencer level
    auto armedTrackIndices = _studioController->sequencer()->armedTrackIndices();
    armedTrackIndices.erase(
        std::remove_if(armedTrackIndices.begin(), armedTrackIndices.end(), [=](int i) { return i == trackIndex; }),
        armedTrackIndices.end());
    _studioController->sequencer()->setArmedTrackIndices(armedTrackIndices);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::sequenceEditorDidModifySequence(MelobaseCore::SequenceEditor* sender) {
    _isSequenceModified = true;
    _isSequenceSaved = false;
    _areDataChanged = true;

    if (!_isEditionInterationInProgress && !sender->undoManager()->hasRemainingOperationsInGroup()) {
        // Show or hide the arrangement view based on data format
        if (_studioController->sequence() &&
            (_studioController->sequence()->data.format == SEQUENCE_DATA_FORMAT_MULTI_TRACK)) {
            _sequenceViewController->setIsArrangementViewVisible(true);
        } else {
            _sequenceViewController->setIsArrangementViewVisible(false);
            if (_sequenceViewController->pianoRollViewController()->sequence()) {
                _sequenceViewController->pianoRollViewController()->setTrackIndex(!_studioController->sequence()->data.tracks.empty() ? 0 : -1);

                // Clear mute and solo track indices
                _studioController->sequencer()->setMuteTrackIndices({});
                _studioController->sequencer()->setSoloTrackIndex(-1);
            }
        }

        // Update the properties and the event list
        _sequenceViewController->pianoRollViewController()->updateProperties();
        _sequenceViewController->pianoRollViewController()->updateEventList();

        // Update the sequencer
        updateSequencer();
    }

    _sequenceViewController->pianoRollViewController()->updatePianoRoll();
    _sequenceViewController->arrangementViewController()->updateTracks();
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::setMIDIInputPortNames(std::vector<std::string> midiInputPortNames) {
    _midiInputPortNames = midiInputPortNames;
    _preferencesViewController->setMIDIInputPortNames(midiInputPortNames);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::setMIDIOutputPortNames(std::vector<std::string> midiOutputPortNames) {
    _midiOutputPortNames = midiOutputPortNames;
    _preferencesViewController->setMIDIOutputPortNames(midiOutputPortNames);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::modalViewControllerWillAppear(ModalViewController* sender) {
    _isModalViewPresented = true;
    if (_menuStatesDidChangeFn) _menuStatesDidChangeFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::modalViewControllerDidDisappear(ModalViewController* sender) {
    _isModalViewPresented = false;
    if (_menuStatesDidChangeFn) _menuStatesDidChangeFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::modalViewControllerDidReceiveResult(ModalViewController* sender,
                                                            ModalViewController::ResultEnumType result) {
    if (result == ModalViewController::OKResult) {
        if (sender == _confirmSetAsPlayedAllViewController) {
            _sequencesDB->setAsPlayedAllSequences(_dbViewController->selectedFolder(),
                                                  _dbViewController->isRecursive());
        } else if (sender == _confirmCleanupViewController) {
            _sequencesDB->cleanupSequences(_dbViewController->selectedFolder(), _dbViewController->isRecursive());
        } else if (sender == _confirmEmptyTrashViewController) {
            _sequencesDB->emptyTrash();
        } else if (sender == _newSequenceViewController) {
            // First, save the sequence if modified
            saveSequenceIfModified();

            if (!isDatabaseVisible()) showHideDatabase();

            auto sequence = std::shared_ptr<MelobaseCore::Sequence>(new MelobaseCore::Sequence);

            sequence->date = sequence->version = sequence->dataVersion = getTimestamp();
            sequence->data.tickPeriod = 0.001;
            sequence->data.currentPosition = 0;
            sequence->playCount = 0;
            sequence->rating = 0;
            sequence->name = _newSequenceViewController->name();
            sequence->folder = _dbViewController->selectedFolder();

            sequence->data.tracks[0]->channel = SEQUENCE_TRACK_MULTI_CHANNEL;
            sequence->data.tracks[0]->clips[0]->events.push_back(std::make_shared<MelobaseCore::ChannelEvent>(
                CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE, 0, 0, 0, _newSequenceViewController->timeSignatureNum(),
                _newSequenceViewController->timeSignatureDenum()));
            sequence->data.tracks[0]->clips[0]->events.push_back(std::make_shared<MelobaseCore::ChannelEvent>(
                CHANNEL_EVENT_TYPE_META_SET_TEMPO, 0, 0, 0, 60000000 / _newSequenceViewController->bpm(), -1));
            sequence->data.tracks[0]->clips[0]->events.push_back(std::make_shared<MelobaseCore::ChannelEvent>(
                CHANNEL_EVENT_TYPE_META_END_OF_TRACK, 0, 10 * _newSequenceViewController->timeSignatureNum() * 480, 0,
                -1, -1));

            _sequencesDB->addSequence(sequence);

            // Select the sequence
            _view->dbView()->sequencesView()->nameSearchField()->setText("");
            _view->dbView()->sequencesView()->filterSegmentedControl()->setSelectedSegment(0);
            int row = rowOfSequenceWithID(sequence->id);
            _view->dbView()->sequencesView()->tableView()->setSelectedRow(row);
            _lastSelectedRow = row;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::preferencesViewControllerDidChangeAudioOutputSettings(PreferencesViewController* sender) {
    _studioController->studio()->setAudioOutputDevice(sender->audioOutputDeviceName(), sender->audioOutputLatency());
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::preferencesViewControllerDidChangeMIDIDestinationSettings(PreferencesViewController* sender) {
    if (sender->midiDestination() == PreferencesViewController::InternalSynthMIDIDestination) {
        _studioController->studio()->setIsInternalSynthEnabled(true);
        _studioController->studio()->setIsMIDIOutputEnabled(false);
    } else if (sender->midiDestination() == PreferencesViewController::MIDIOutputMIDIDestination) {
        _studioController->studio()->setIsInternalSynthEnabled(false);
        _studioController->studio()->setIsMIDIOutputEnabled(true);
    } else {
        _studioController->studio()->setIsInternalSynthEnabled(false);
        _studioController->studio()->setIsMIDIOutputEnabled(false);
    }
    _view->studioView()->setIsMasterLevelSliderVisible(_studioController->studio()->isInternalSynthEnabled());
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::preferencesViewControllerDidChangeNbVoicesSettings(PreferencesViewController* sender) {
    _studioController->studio()->setNbVoices(sender->nbVoices());
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::preferencesViewControllerDidChangeAutoStopRecordSettings(PreferencesViewController* sender) {
    _studioController->setIsAutoStopRecordEnabled(sender->isAutoStopRecordEnabled());
    _studioController->setAutoStopRecordPeriod(sender->autoStopRecordPeriod());
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::aboutViewControllerAcknowledgementButtonClicked(AboutViewController* sender) {
    _aboutViewController->dismiss();
    MDStudio::Platform::sharedInstance()->invoke([=] { _acknowledgementsViewController->showModal(); });
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::masterLevelValueDidChange(Slider* sender, float pos) {
    float linearLevel = powf(10.0f, pos / 10.0f);
    _studioController->studio()->setMasterMixerLevel(STUDIO_SOURCE_USER, linearLevel);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::audioExportDidStart(MDStudio::AudioExport* sender) {
    _isExportingAudio = true;
    _studioController->stop();
    _studioController->goToBeginning();
    _studioController->sequencer()->studio()->mixer()->stop();
    _studioController->sequencer()->studio()->stopAllNotes(true);

    _wasInternalSynthEnabled = _studioController->sequencer()->studio()->isInternalSynthEnabled();
    _wasMIDIOutputEnabled = _studioController->sequencer()->studio()->isMIDIOutputEnabled();

    _studioController->sequencer()->studio()->setIsInternalSynthEnabled(true);
    _studioController->sequencer()->studio()->setIsMIDIOutputEnabled(false);

    _sequenceViewController->pianoRollViewController()->setIsKeyboardUpdated(false);
    _audioExportViewController->setProgress(0.0f);
    _audioExportViewController->showModal();
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::audioExportDidSetProgress(MDStudio::AudioExport* sender, float progress) {
    _audioExportViewController->setProgress(progress);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::audioExportDidFinish(MDStudio::AudioExport* sender) {
    _audioExportViewController->dismiss();

    _studioController->sequencer()->studio()->setIsInternalSynthEnabled(_wasInternalSynthEnabled);
    _studioController->sequencer()->studio()->setIsMIDIOutputEnabled(_wasMIDIOutputEnabled);

    _studioController->sequencer()->studio()->mixer()->start();
    _sequenceViewController->pianoRollViewController()->setIsKeyboardUpdated(true);
    _isExportingAudio = false;
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::midiImportDidStart(MIDIImport* sender) {
    _midiImportViewController->setProgress(0.0f);
    _midiImportViewController->showModal();
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::midiImportDidSetProgress(MIDIImport* sender, float progress) {
    _midiImportViewController->setProgress(progress);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::midiImportDidFinish(MIDIImport* sender, bool isSuccessful) {
    _midiImportViewController->dismiss();

    if (isSuccessful) {
        _view->dbView()->sequencesView()->filterSegmentedControl()->setSelectedSegment(0);
        _dbViewController->reloadSequences();

        // If only one sequence was imported, select it
        if (sender->paths().size() == 1) {
            int row = rowOfSequenceWithID(sender->lastImportedSequenceID());
            _view->dbView()->sequencesView()->tableView()->setSelectedRow(row);
            _lastSelectedRow = row;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::showHideStudio() {
    _view->studioView()->setIsVisible(!_view->studioView()->isVisible());
    _view->setFrame(_view->frame());
    if (_menuStatesDidChangeFn) _menuStatesDidChangeFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::showHideDatabase() {
    _view->horizSplitView()->setLeftPaneVisibility(!isDatabaseVisible());
    if (_menuStatesDidChangeFn) _menuStatesDidChangeFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::showHideFolders() {
    if (!isDatabaseVisible()) showHideDatabase();
    _dbViewController->showHideFolders();
    if (_menuStatesDidChangeFn) _menuStatesDidChangeFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::showHideControllerEvents() {
    _sequenceViewController->pianoRollViewController()->showHideControllerEvents();
    if (_menuStatesDidChangeFn) _menuStatesDidChangeFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::showHideProperties() {
    _sequenceViewController->pianoRollViewController()->showHideProperties();
    if (_menuStatesDidChangeFn) _menuStatesDidChangeFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::pianoRollViewControllerDidSetPaneVisibility(PianoRollViewController* sender) {
    if (_menuStatesDidChangeFn) _menuStatesDidChangeFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::dbViewControllerSequenceSelectionDidChange(
    DBViewController* sender, std::vector<std::shared_ptr<MelobaseCore::Sequence>> sequences) {
    _selectedSequences = sequences;
    std::shared_ptr<MelobaseCore::Sequence> sequence = nullptr;
    // If we have a single selection, we read the data
    if (sequences.size() == 1) {
        sequence = sequences[0];
        _sequencesDB->readSequenceData(sequence);
    }
    
    _sequenceViewController->pianoRollViewController()->setTrackIndex(-1);
    setSequence(sequence);
    updateControlButtons();

    // Show or hide the arrangement view based on data format
    if (sequence) {
        if (sequence->data.format == SEQUENCE_DATA_FORMAT_MULTI_TRACK) {
            _sequenceViewController->setIsArrangementViewVisible(true);
            _sequenceViewController->pianoRollViewController()->setTrackIndex(-1);
        } else {
            _sequenceViewController->setIsArrangementViewVisible(false);
            _sequenceViewController->pianoRollViewController()->setTrackIndex(!_studioController->sequence()->data.tracks.empty() ? 0 : -1);
        }
    } else {
        _sequenceViewController->setIsArrangementViewVisible(false);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::dbViewControllerFolderSelectionDidChange(
    DBViewController* sender, std::shared_ptr<MelobaseCore::SequencesFolder> folder) {
    _studioController->setCurrentFolder(folder);
    _dbViewController->reloadSequences();

    // Set the document name
    std::string name = _view->untitledStr();
    Platform::sharedInstance()->setDocumentName(name + " " + folderPath());

    updateControlButtons();

    if (_menuStatesDidChangeFn) _menuStatesDidChangeFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::dbViewControllerFoldersWillChange(DBViewController* sender) {
    _studioController->stop();
    saveSequenceIfModified();
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::dbViewControllerSequenceNameDidChange(DBViewController* sender, std::string text) {
    saveSequenceIfModified();
    std::shared_ptr<MelobaseCore::Sequence> sequence = _studioController->sequence();

    if (!sequence) return;

    sequence->name = text;

    // This callback is from a control present in the list that will be reloaded by the following call, so we invoke it
    Platform::sharedInstance()->invoke([=] { _sequencesDB->updateSequences({sequence}); });
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::dbViewControllerSequenceRatingDidChange(DBViewController* sender, float rating) {
    saveSequenceIfModified();
    std::shared_ptr<MelobaseCore::Sequence> sequence = _studioController->sequence();

    if (!sequence) return;

    sequence->rating = rating;
    if (sequence->playCount == 0) sequence->playCount = 1;

    // This callback is from a control present in the list that will be reloaded by the following call, so we invoke it
    Platform::sharedInstance()->invoke([=] { _sequencesDB->updateSequences({sequence}); });
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::dbViewControllerDidSetPaneVisibility(DBViewController* sender) {
    auto foldersPane = _view->dbView()->splitView()->subviews()[0];
    if (foldersPane->isVisible()) {
        _view->horizSplitView()->setSplitPos(_view->horizSplitView()->splitPos() + foldersPane->rect().size.width);
    } else {
        _view->horizSplitView()->setSplitPos(_view->horizSplitView()->splitPos() - foldersPane->rect().size.width);
    }

    if (_menuStatesDidChangeFn) _menuStatesDidChangeFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::zoneViewControllerChannelDidChange(ZoneViewController* sender, int channel) {
    if ((sender == _upperZoneViewController) &&
        _sequenceViewController->pianoRollViewController()->currentZone() == 1) {
        _sequenceViewController->pianoRollViewController()->setCurrentMultiChannel(channel);
    } else if ((sender == _lowerZoneViewController) &&
               _sequenceViewController->pianoRollViewController()->currentZone() == 0) {
        _sequenceViewController->pianoRollViewController()->setCurrentMultiChannel(channel);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::pianoRollViewControllerDidSelectZone(PianoRollViewController* sender, int zone) {
    int channel;
    if (_sequenceViewController->pianoRollViewController()->currentZone() == 1) {
        channel = _upperZoneViewController->channel();
    } else {
        channel = _lowerZoneViewController->channel();
    }
    _sequenceViewController->pianoRollViewController()->setCurrentMultiChannel(channel);
}

// ---------------------------------------------------------------------------------------------------------------------
bool TopViewController::canMoveToTrashSequence() {
    if (_isModalViewPresented) return false;

    if (_view->dbView()->sequencesView()->tableView()->hasFocus()) {
        return _selectedSequences.size() > 0;
    }

    if (_view->dbView()->foldersView()->treeView()->hasFocus()) {
        auto selectedFolder = _dbViewController->selectedFolder();

        if (selectedFolder) {
            if ((selectedFolder->id != SEQUENCES_FOLDER_ID) && !_dbViewController->isSelectedFolderTrash()) return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
int TopViewController::rowOfSequenceWithID(UInt64 sequenceID) {
    for (int row = 0; row < _view->dbView()->sequencesView()->tableView()->nbRows(); ++row) {
        auto view = _view->dbView()->sequencesView()->tableView()->viewAtRow(row);
        auto sequenceListItemView = dynamic_pointer_cast<SequenceListItemView>(view);
        if (sequenceListItemView->sequence()->id == sequenceID) return row;
    }

    return -1;
}

// ---------------------------------------------------------------------------------------------------------------------
void TopViewController::resetPitchBendRange() {
    auto studio = _studioController->studio();
    for (int channel = 0; channel < STUDIO_MAX_CHANNELS; ++channel) {
        if (studio->pitchBendFactor(channel) != 1.0f) {
            studio->setControlValue(STUDIO_SOURCE_USER, STUDIO_CONTROL_REG_PARAM_NUM_MSB, 0, channel);
            studio->setControlValue(STUDIO_SOURCE_USER, STUDIO_CONTROL_REG_PARAM_NUM_LSB, 0, channel);
            studio->setControlValue(STUDIO_SOURCE_USER, STUDIO_CONTROL_DATA_ENTRY_MSB, 2, channel);
            studio->setControlValue(STUDIO_SOURCE_USER, STUDIO_CONTROL_DATA_ENTRY_LSB, 0, channel);
        }
    }
}
