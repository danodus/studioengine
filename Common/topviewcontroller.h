//
//  topviewcontroller.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2014-07-21.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#ifndef TOPVIEWCONTROLLER_H
#define TOPVIEWCONTROLLER_H

#include <animation.h>
#include <audioexport.h>
#include <button.h>
#include <image.h>
#include <labelview.h>
#include <melobasecorescriptmodule.h>
#include <sequenceeditor.h>
#include <sequencesdb.h>
#include <server.h>
#include <sheet.h>
#include <slider.h>
#include <studio.h>
#include <studiocontroller.h>
#include <uiscriptmodule.h>

#include <memory>
#include <thread>

#include "aboutviewcontroller.h"
#include "audioexportviewcontroller.h"
#include "dbviewcontroller.h"
#include "melobasescriptmodule.h"
#include "midiimport.h"
#include "midiimportviewcontroller.h"
#include "modalviewcontroller.h"
#include "newsequenceviewcontroller.h"
#include "preferencesviewcontroller.h"
#include "sequenceviewcontroller.h"
#include "topview.h"
#include "zoneviewcontroller.h"

class TopViewController {
   public:
    typedef std::function<void(TopViewController* sender)> MenuStatesDidChangeFnType;

    static constexpr int metronomeModeNone = 0;
    static constexpr int metronomeModeTapTempo = 1;
    static constexpr int metronomeModeSpecified = 2;

   private:
    std::shared_ptr<TopView> _view;

    bool _isFadeInEnabled;
    MDStudio::Animation _loadingAnimation;

    MelobaseCore::SequencesDB* _sequencesDB;
    MelobaseCore::Server* _server;

    std::shared_ptr<MDStudio::ImageView> _learnTempoArrowImageView[2];

    MDStudio::Animation* _learnTempoArrowAnimation;

    MelobaseCore::StudioController* _studioController;

    ZoneViewController* _upperZoneViewController;
    ZoneViewController* _lowerZoneViewController;

    DBViewController* _dbViewController;
    SequenceViewController* _sequenceViewController;

    AboutViewController* _aboutViewController;
    ModalViewController* _confirmCleanupViewController;
    ModalViewController* _confirmEmptyTrashViewController;
    ModalViewController* _confirmSetAsPlayedAllViewController;
    ModalViewController* _acknowledgementsViewController;

    PreferencesViewController* _preferencesViewController;
    AudioExportViewController* _audioExportViewController;
    MIDIImportViewController* _midiImportViewController;
    NewSequenceViewController* _newSequenceViewController;

    MDStudio::AudioExport* _audioExport;
    MIDIImport* _midiImport;

    MDStudio::Property _metronomeMode{"metronomeMode", metronomeModeTapTempo};
    MDStudio::Property _timeSigNum{"timeSigNum"};
    MDStudio::Property _timeSigDenum{"timeSigDenum"};
    MDStudio::Property _tempo{"tempo"};

    bool _isRecordingNewSequence;
    bool _isLearningTempo;
    std::atomic<bool> _isSettingSplitPosition;
    int _splitPosition;
    bool _isKeyboardSplitted;
    bool _lastSequenceWasPlayed;
    std::shared_ptr<MelobaseCore::Sequence> _lastSequence;
    std::atomic<bool> _metronomeArmed;
    int _metronomeLearningPitch;
    bool _isReloadSequencePending;
    bool _isModalViewPresented;

    // Audio export
    bool _wasInternalSynthEnabled;
    bool _wasMIDIOutputEnabled;

    std::shared_ptr<MDStudio::Image> _learnTempoArrowImage;

    Float32 _velocity;
    int _octave;
    bool _sustainState;

    int _lastSelectedRow;

    bool _isMIDIDeviceAvailable;
    std::vector<std::string> _midiInputPortNames, _midiOutputPortNames;

    std::atomic<bool> _isExportingAudio;

    bool _wasDatabaseVisible;

    MenuStatesDidChangeFnType _menuStatesDidChangeFn;

    MelobaseCore::SequenceEditor* _sequenceEditor;
    bool _isSequenceModified, _isSequenceSaved;
    bool _areMetaDataChanged = false;
    bool _areDataChanged = false;

    std::vector<std::shared_ptr<MelobaseCore::Sequence>> _selectedSequences;

    bool _isEditionInterationInProgress;

    std::unique_ptr<MDStudio::UIScriptModule> _uiScriptModule;
    std::unique_ptr<MelobaseCore::MelobaseCoreScriptModule> _melobaseCoreScriptModule;
    std::unique_ptr<MelobaseScriptModule> _melobaseScriptModule;
    std::unique_ptr<MDStudio::Script> _mainScript;

    std::shared_ptr<MDStudio::Studio::didSetTempoFnType> _didSetTempoFn;
    std::shared_ptr<MDStudio::Studio::didSetTimeSignatureFnType> _didSetTimeSignatureFn;

    std::string stringFromPitch(int pitch);
    void updateSplitControls();
    void cancelSetSplitPosition();
    int zoneForPitch(int pitch) const;

    void splitClicked(MDStudio::Button* sender);
    void splitStateDidChange(MDStudio::Button* sender, bool state);
    void splitCancelClicked(MDStudio::Button* sender);
    void recordClicked(MDStudio::Button* sender);
    void rewindClicked(MDStudio::Button* sender);
    void playClicked(MDStudio::Button* sender);
    void convertSequenceClicked(MDStudio::Button* sender);

    void studioControllerSaveSequences(MelobaseCore::StudioController* studioController);
    void studioControllerRecordingDidStart(MelobaseCore::StudioController* studioController);
    void studioControllerRecordingDidFinish(MelobaseCore::StudioController* studioController);
    void studioControllerUpdateSequencerStatus(MelobaseCore::StudioController* studioController);
    void studioControllerMajorTick(MelobaseCore::StudioController* studioController);
    void studioControllerMinorTick(MelobaseCore::StudioController* studioController);

    void sequencesDBSequenceAdded(MelobaseCore::SequencesDB* sequencesDB);
    void sequencesDBFolderAdded(MelobaseCore::SequencesDB* sequencesDB,
                                std::shared_ptr<MelobaseCore::SequencesFolder> folder);
    void sequencesDBWillRemoveSequence(MelobaseCore::SequencesDB* sequenceDB,
                                       std::shared_ptr<MelobaseCore::Sequence> sequence);
    void sequencesDBSequenceRemoved(MelobaseCore::SequencesDB* sequencesDB);
    void sequencesDBFolderRemoved(MelobaseCore::SequencesDB* sequencesDB);
    void sequencesDBSequenceUpdated(MelobaseCore::SequencesDB* sequencesDB);
    void sequencesDBFolderUpdated(MelobaseCore::SequencesDB* sequencesDB);

    void cancelLearnTempo();

    bool handleEvent(TopView* sender, const MDStudio::UIEvent* event);

    void increaseLastSequencePlayCount();

    void undoManagerStatesDidChange(MDStudio::UndoManager* sender);

    void splitViewPosChanged(MDStudio::SplitViewH* sender, float pos);

    void dumpEvents();

    void arrangementViewControllerWillModifySequence(ArrangementViewController* sender);

    void pianoRollViewControllerWillModifySequence(PianoRollViewController* sender);
    void pianoRollViewControllerDidModifySequence(PianoRollViewController* sender);
    void pianoRollViewControllerDidSelectEvents(PianoRollViewController* sender);
    void sequenceEditorWillRemoveTrack(MelobaseCore::SequenceEditor* sender,
                                       std::shared_ptr<MelobaseCore::Track> track);
    void sequenceEditorDidModifySequence(MelobaseCore::SequenceEditor* sender);

    void modalViewControllerWillAppear(ModalViewController* sender);
    void modalViewControllerDidDisappear(ModalViewController* sender);
    void modalViewControllerDidReceiveResult(ModalViewController* sender, ModalViewController::ResultEnumType result);

    void preferencesViewControllerDidChangeAudioOutputSettings(PreferencesViewController* sender);
    void preferencesViewControllerDidChangeMIDIDestinationSettings(PreferencesViewController* sender);
    void preferencesViewControllerDidChangeAutoStopRecordSettings(PreferencesViewController* sender);
    void preferencesViewControllerDidChangeNbVoicesSettings(PreferencesViewController* sender);
    void aboutViewControllerAcknowledgementButtonClicked(AboutViewController* sender);

    void masterLevelValueDidChange(MDStudio::Slider* sender, float pos);

    void audioExportDidStart(MDStudio::AudioExport* sender);
    void audioExportDidSetProgress(MDStudio::AudioExport* sender, float progress);
    void audioExportDidFinish(MDStudio::AudioExport* sender);

    void midiImportDidStart(MIDIImport* sender);
    void midiImportDidSetProgress(MIDIImport* sender, float progress);
    void midiImportDidFinish(MIDIImport* sender, bool isSuccessful);

    void pianoRollViewControllerDidSetPaneVisibility(PianoRollViewController* sender);

    void dbViewControllerSequenceSelectionDidChange(DBViewController* sender,
                                                    std::vector<std::shared_ptr<MelobaseCore::Sequence>> sequences);
    void dbViewControllerFolderSelectionDidChange(DBViewController* sender,
                                                  std::shared_ptr<MelobaseCore::SequencesFolder> folder);
    void dbViewControllerFoldersWillChange(DBViewController* sender);
    void dbViewControllerSequenceNameDidChange(DBViewController* sender, std::string text);
    void dbViewControllerSequenceRatingDidChange(DBViewController* sender, float rating);
    void dbViewControllerDidSetPaneVisibility(DBViewController* sender);

    void zoneViewControllerChannelDidChange(ZoneViewController* sender, int channel);
    void pianoRollViewControllerDidSelectZone(PianoRollViewController* sender, int zone);

    int rowOfSequenceWithID(UInt64 sequenceID);
    void updateSequencer();

    std::string folderPath();

    void resetPitchBendRange();

   public:
    TopViewController(std::shared_ptr<TopView> view, MelobaseCore::StudioController* studioController,
                      MelobaseCore::SequencesDB* sequencesDB, MelobaseCore::Server* server,
                      std::string defaultMIDIInputPortName, std::string defaultMIDIOutputPortName,
                      bool isFadeInEnabled = false);
    ~TopViewController();

    MDStudio::Studio* studio() { return _studioController->studio(); }
    MelobaseCore::StudioController* studioController() { return _studioController; }

    // Thread-safe
    void noteOn(int channel, int pitch, Float32 velocity);
    void noteOff(int channel, int pitch, Float32 velocity);
    void keyAftertouch(int channel, int pitch, Float32 value);
    void channelAftertouch(int channel, Float32 value);
    void programChange(int channel, int program);
    void sustainChanged(int channel, Float32 value);
    void pitchBendChanged(int channel, Float32 value);
    void modulationChanged(int channel, Float32 value);
    void volumeChanged(int channel, Float32 value);
    void panChanged(int channel, Float32 value);
    void controlChanged(int channel, UInt32 control, UInt32 value);

    void setSequence(std::shared_ptr<MelobaseCore::Sequence> sequence);
    std::shared_ptr<MelobaseCore::Sequence> sequence() { return _studioController->sequence(); }
    std::shared_ptr<TopView> view() { return _view; }

    void saveSequenceIfModified();
    void autoSave();

    void setMenuStatesDidChangeFn(MenuStatesDidChangeFnType menuStatesDidChange) {
        _menuStatesDidChangeFn = menuStatesDidChange;
    }

    // Menu actions

    void about();
    void showPreferences();

    void newSequence();
    void newSubfolder();
    void convertSequence();
    void deleteSequence();
    void moveToTrashSequence();
    void promoteAllSequences();
    void demoteAllSequences();
    void setAsPlayedAllSequences();
    void cleanupSequences();
    void emptyTrash();
    void importSequences(std::vector<std::string> paths);
    void exportSequence(std::string path);
    void exportAudio(std::string path);
    void setMetronomeSound(bool state);
    void setSequenceRating(float rating);

    // View
    void showHideStudio();
    void showHideDatabase();
    void showHideFolders();
    void showHideControllerEvents();
    void showHideProperties();

    // Edition
    void editSelectAll();
    void editDelete();
    void editQuantize();
    void editCopy();
    void editCut();
    void editPaste();

    // Controls
    void goToBeginning();
    void playPause();
    void stop();
    void record();

    // Undo/redo
    void undo();
    void redo();

    // Menu states
    bool canUndo() { return !_isModalViewPresented && _sequencesDB->undoManager()->canUndo(); }
    bool canRedo() { return !_isModalViewPresented && _sequencesDB->undoManager()->canRedo(); }
    bool canGoToBeginning() { return !_isModalViewPresented && _view->rewindButton()->isEnabled(); }
    bool canPlay() { return !_isModalViewPresented && _view->playButton()->isEnabled(); }
    bool canRecord() { return !_isModalViewPresented && _view->recordButton()->isEnabled(); }
    bool canSetSequenceRating() { return !_isModalViewPresented && (_selectedSequences.size() > 0); }
    bool canMoveToTrashSequence();
    bool canExportSequence() { return !_isModalViewPresented && _studioController->sequence() != nullptr; }
    bool canExportAudio() { return !_isModalViewPresented && _studioController->sequence() != nullptr; }
    bool canCreateNewSequence() {
        return (!_isModalViewPresented && _dbViewController->selectedFolder() &&
                !_dbViewController->isSelectedFolderTrash());
    }
    bool canCreateNewSubfolder() {
        return (!_isModalViewPresented && _dbViewController->selectedFolder() &&
                !_dbViewController->isSelectedFolderTrash());
    }
    bool canConvertSequence() { return !_isModalViewPresented && _studioController->sequence() != nullptr; }
    bool canPromoteAllSequences() { return !_isModalViewPresented; }
    bool canDemoteAllSequences() { return !_isModalViewPresented; }
    bool canSetAsPlayedAllSequences() { return !_isModalViewPresented; }
    bool canCleanupSequences() { return !_isModalViewPresented; }
    bool canEmptyTrash() { return !_isModalViewPresented; }
    bool canEditSelectAll() { return !_isModalViewPresented; }
    bool canEditQuantize() {
        return !_isModalViewPresented && _sequenceViewController->pianoRollViewController()->areEventsSelected();
    }
    bool canEditDelete() { return !_isModalViewPresented; }
    bool canShowAbout() { return !_isModalViewPresented; }
    bool canShowPreferences() { return !_isModalViewPresented; }

    bool isSequenceMultiTrack() {
        return _studioController->sequence()
                   ? (_studioController->sequence()->data.format == SEQUENCE_DATA_FORMAT_MULTI_TRACK)
                   : false;
    }
    bool isStudioVisible() { return _view->studioView()->isVisible(); }
    bool isDatabaseVisible() { return _view->dbView()->isVisible(); }
    bool areFoldersVisible() { return _dbViewController->areFoldersVisible(); }
    bool areControllerEventsVisible() {
        return _sequenceViewController->pianoRollViewController()->areControllerEventsVisible();
    }
    bool arePropertiesVisible() { return _sequenceViewController->pianoRollViewController()->arePropertiesVisible(); }

    void updateControlButtons();
    void setIsMIDIDeviceAvailable(bool isAvailable);
    void setMIDIInputPortNames(std::vector<std::string> midiInputPortNames);
    void setMIDIOutputPortNames(std::vector<std::string> midiOutputPortNames);

    std::string midiInputPortName() { return _preferencesViewController->midiInputPortName(); }
    std::string midiOutputPortName() { return _preferencesViewController->midiOutputPortName(); }

    std::string audioOutputDeviceName() { return _preferencesViewController->audioOutputDeviceName(); }
    double audioOutputLatency() { return _preferencesViewController->audioOutputLatency(); }
};

#endif  // TOPVIEWCONTROLLER_H
