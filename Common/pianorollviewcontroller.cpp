//
//  pianorollviewcontroller.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-01-09.
//  Copyright (c) 2015-2021 Daniel Cliche. All rights reserved.
//

#include "pianorollviewcontroller.h"

#include <math.h>
#include <pasteboard.h>
#include <platform.h>
#include <responderchain.h>

#include <algorithm>

#include "helpers.h"

#define EDITION_MODE_CURSOR_SEGMENT 0
#define EDITION_MODE_SELECTION_SEGMENT 1
#define EDITION_MODE_DRAWING_SEGMENT 2
#define EDITION_MODE_MOVE_SEGMENT 3
#define EDITION_MODE_RESIZE_SEGMENT 4

static const char* controllerMessages[128] = {"000 - Bank Select",
                                              "*001 - Modulation (M)",
                                              "002 - Breath Control",
                                              "003 - Undefined",
                                              "004 - Foot Controller",
                                              "005 - Portamento Time",
                                              "006 - Data Entry MSB",
                                              "*007 - Channel Volume (Vol)",
                                              "008 - Balance",
                                              "009 - Undefined",
                                              "*010 - Pan (Pan)",
                                              "011 - Expression Ctrl",
                                              "012 - Effect Ctrl 1",
                                              "013 - Effect Ctrl 2",
                                              "014 - Undefined",
                                              "015 - Undefined",
                                              "016 - General Ctrl 1",
                                              "017 - General Ctrl 2",
                                              "018 - General Ctrl 3",
                                              "019 - General Ctrl 4",
                                              "020 - Undefined",
                                              "021 - Undefined",
                                              "022 - Undefined",
                                              "023 - Undefined",
                                              "024 - Undefined",
                                              "025 - Undefined",
                                              "026 - Undefined",
                                              "027 - Undefined",
                                              "028 - Undefined",
                                              "029 - Undefined",
                                              "030 - Undefined",
                                              "031 - Undefined",
                                              "032 - Bank Select",
                                              "033 - Modulation",
                                              "034 - Breath Ctrl",
                                              "035 - Undefined",
                                              "036 - Foot Ctrl",
                                              "037 - Portamento Time",
                                              "038 - Data Entry LSB",
                                              "039 - Channel Volume",
                                              "040 - Balance",
                                              "041 - Undefined",
                                              "042 - Pan",
                                              "043 - Expression Ctrl",
                                              "044 - Effect Ctrl 1",
                                              "045 - Effect Ctrl 2",
                                              "046 - Undefined",
                                              "047 - Undefined",
                                              "048 - General Ctrl 1",
                                              "049 - General Ctrl 2",
                                              "050 - General Ctrl 3",
                                              "051 - General Ctrl 4",
                                              "052 - Undefined",
                                              "053 - Undefined",
                                              "054 - Undefined",
                                              "055 - Undefined",
                                              "056 - Undefined",
                                              "057 - Undefined",
                                              "058 - Undefined",
                                              "059 - Undefined",
                                              "060 - Undefined",
                                              "061 - Undefined",
                                              "062 - Undefined",
                                              "063 - Undefined",
                                              "*064 - Sustain (S)",
                                              "065 - Portamento",
                                              "066 - Sustenuto",
                                              "067 - Soft Pedal",
                                              "068 - Legato",
                                              "069 - Hold 2",
                                              "070 - Sound Ctrl 1",
                                              "071 - Sound Ctrl 2",
                                              "072 - Sound Ctrl 3",
                                              "073 - Sound Ctrl 4",
                                              "074 - Sound Ctrl 5",
                                              "075 - Sound Ctrl 6",
                                              "076 - Sound Ctrl 7",
                                              "077 - Sound Ctrl 8",
                                              "078 - Sound Ctrl 9",
                                              "079 - Sound Ctrl 10",
                                              "080 - General Ctrl 5",
                                              "081 - General Ctrl 6",
                                              "082 - General Ctrl 7",
                                              "083 - General Ctrl 8",
                                              "084 - Portamento",
                                              "085 - Undefined",
                                              "086 - Undefined",
                                              "087 - Undefined",
                                              "088 - Undefined",
                                              "089 - Undefined",
                                              "090 - Undefined",
                                              "*091 - Reverb",
                                              "092 - Effect 2",
                                              "*093 - Chorus",
                                              "094 - Effect 4",
                                              "095 - Effect 5",
                                              "096 - Data Entry +1",
                                              "097 - Data Entry -1",
                                              "098 - NR Param LSB",
                                              "099 - NR Param MSB",
                                              "100 - R Param LSB",
                                              "101 - R Param MSB",
                                              "102 - Undefined",
                                              "103 - Undefined",
                                              "104 - Undefined",
                                              "105 - Undefined",
                                              "106 - Undefined",
                                              "107 - Undefined",
                                              "108 - Undefined",
                                              "109 - Undefined",
                                              "110 - Undefined",
                                              "111 - Undefined",
                                              "112 - Undefined",
                                              "113 - Undefined",
                                              "114 - Undefined",
                                              "115 - Undefined",
                                              "116 - Undefined",
                                              "117 - Undefined",
                                              "118 - Undefined",
                                              "119 - Undefined",
                                              "120 - All Sound Off",
                                              "121 - Reset All Ctrls",
                                              "122 - Local Ctrl On/Off",
                                              "123 - All Notes Off",
                                              "124 - Omni Mode Off",
                                              "125 - Omni Mode On",
                                              "126 - Poly Mode On/Off",
                                              "127 - Poly Mode On"};

static const char* metaTypes[128] = {"000 - Sequence Number",
                                     "001 - Text Event",
                                     "002 - Copyright Notice",
                                     "003 - Sequence/Track Name",
                                     "004 - Instrument Name",
                                     "005 - Lyric",
                                     "006 - Marker",
                                     "007 - Cue Point",
                                     "008 - Undefined",
                                     "009 - Undefined",
                                     "010 - Undefined",
                                     "011 - Undefined",
                                     "012 - Undefined",
                                     "013 - Undefined",
                                     "014 - Undefined",
                                     "015 - Undefined",
                                     "016 - Undefined",
                                     "017 - Undefined",
                                     "018 - Undefined",
                                     "019 - Undefined",
                                     "020 - Undefined",
                                     "021 - Undefined",
                                     "022 - Undefined",
                                     "023 - Undefined",
                                     "024 - Undefined",
                                     "025 - Undefined",
                                     "026 - Undefined",
                                     "027 - Undefined",
                                     "028 - Undefined",
                                     "029 - Undefined",
                                     "030 - Undefined",
                                     "031 - Undefined",
                                     "032 - MIDI Channel Prefix",
                                     "033 - Prefix Port",
                                     "034 - Undefined",
                                     "035 - Undefined",
                                     "036 - Undefined",
                                     "037 - Undefined",
                                     "038 - Undefined",
                                     "039 - Undefined",
                                     "040 - Undefined",
                                     "041 - Undefined",
                                     "042 - Undefined",
                                     "043 - Undefined",
                                     "044 - Undefined",
                                     "045 - Undefined",
                                     "046 - Undefined",
                                     "047 - End of Track (EOT)",
                                     "048 - Undefined",
                                     "049 - Undefined",
                                     "050 - Undefined",
                                     "051 - Undefined",
                                     "052 - Undefined",
                                     "053 - Undefined",
                                     "054 - Undefined",
                                     "055 - Undefined",
                                     "056 - Undefined",
                                     "057 - Undefined",
                                     "058 - Undefined",
                                     "059 - Undefined",
                                     "060 - Undefined",
                                     "061 - Undefined",
                                     "062 - Undefined",
                                     "063 - Undefined",
                                     "064 - Undefined",
                                     "065 - Undefined",
                                     "066 - Undefined",
                                     "067 - Undefined",
                                     "068 - Undefined",
                                     "069 - Undefined",
                                     "070 - Undefined",
                                     "071 - Undefined",
                                     "072 - Undefined",
                                     "073 - Undefined",
                                     "074 - Undefined",
                                     "075 - Undefined",
                                     "076 - Undefined",
                                     "077 - Undefined",
                                     "078 - Undefined",
                                     "079 - Undefined",
                                     "080 - Undefined",
                                     "*081 - Tempo (T)",
                                     "082 - Undefined",
                                     "083 - Undefined",
                                     "084 - SMPTE Offset",
                                     "085 - Undefined",
                                     "086 - Undefined",
                                     "087 - Undefined",
                                     "*088 - Time Signature (TS)",
                                     "089 - Key Signature",
                                     "090 - Undefined",
                                     "091 - Undefined",
                                     "092 - Undefined",
                                     "093 - Undefined",
                                     "094 - Undefined",
                                     "095 - Undefined",
                                     "096 - Undefined",
                                     "097 - Undefined",
                                     "098 - Undefined",
                                     "099 - Undefined",
                                     "100 - Undefined",
                                     "101 - Undefined",
                                     "102 - Undefined",
                                     "103 - Undefined",
                                     "104 - Undefined",
                                     "105 - Undefined",
                                     "106 - Undefined",
                                     "107 - Undefined",
                                     "108 - Undefined",
                                     "109 - Undefined",
                                     "110 - Undefined",
                                     "111 - Undefined",
                                     "112 - Undefined",
                                     "113 - Undefined",
                                     "114 - Undefined",
                                     "115 - Undefined",
                                     "116 - Undefined",
                                     "117 - Undefined",
                                     "118 - Undefined",
                                     "119 - Undefined",
                                     "120 - Undefined",
                                     "121 - Undefined",
                                     "122 - Undefined",
                                     "123 - Undefined",
                                     "124 - Undefined",
                                     "125 - Undefined",
                                     "126 - Undefined",
                                     "127 - Sequencer Specific"};

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::setPianoRollEventsViewFns(std::shared_ptr<PianoRollEventsView> pianoRollEventsView) {
    using namespace std::placeholders;

    pianoRollEventsView->setSequenceInfosFn(
        std::bind(&PianoRollViewController::sequenceInfos, this, _1, _2, _3, _4, _5));
    pianoRollEventsView->setNbEventsFn(std::bind(&PianoRollViewController::nbEvents, this, _1, _2));
    pianoRollEventsView->setEventAtIndexFn(std::bind(&PianoRollViewController::eventAtIndex, this, _1, _2, _3, _4));
    pianoRollEventsView->setNbAnnotationsFn(std::bind(&PianoRollViewController::nbAnnotations, this, _1));
    pianoRollEventsView->setAnnotationAtIndexFn(
        std::bind(&PianoRollViewController::annotationAtIndex, this, _1, _2, _3));
    pianoRollEventsView->setDidSetCursorTickPosFn(
        std::bind(&PianoRollViewController::didSetCursorTickPos, this, _1, _2));
    pianoRollEventsView->setDidFinishSettingCursorTickPosFn(
        std::bind(&PianoRollViewController::didFinishSettingCursorTickPos, this, _1));
    pianoRollEventsView->setWillSetSelectionRegionFn(
        std::bind(&PianoRollViewController::willSetSelectionRegion, this, _1));
    pianoRollEventsView->setDidSetSelectionRegionFn(
        std::bind(&PianoRollViewController::didSetSelectionRegion, this, _1));
    pianoRollEventsView->setPreviousMeasureFn(std::bind(&PianoRollViewController::previousMeasure, this, _1));
    pianoRollEventsView->setNextMeasureFn(std::bind(&PianoRollViewController::nextMeasure, this, _1));
    pianoRollEventsView->setDeleteEventsFn(std::bind(&PianoRollViewController::deleteEvents, this, _1));
    pianoRollEventsView->setWillMoveEventsFn(std::bind(&PianoRollViewController::willMoveEvents, this, _1));
    pianoRollEventsView->setMoveEventsFn(std::bind(&PianoRollViewController::moveEvents, this, _1, _2, _3, _4, _5, _6));
    pianoRollEventsView->setDidMoveEventsFn(std::bind(&PianoRollViewController::didMoveEvents, this, _1));
    pianoRollEventsView->setAddNoteEventFn(std::bind(&PianoRollViewController::addNoteEvent, this, _1, _2, _3));
    pianoRollEventsView->setAddSustainEventFn(std::bind(&PianoRollViewController::addSustainEvent, this, _1, _2, _3));
    pianoRollEventsView->setAddProgramChangeEventFn(
        std::bind(&PianoRollViewController::addProgramChangeEvent, this, _1, _2));
    pianoRollEventsView->setAddTempoEventFn(std::bind(&PianoRollViewController::addTempoEvent, this, _1, _2, _3));
    pianoRollEventsView->setAddPitchBendEventFn(
        std::bind(&PianoRollViewController::addPitchBendEvent, this, _1, _2, _3));
    pianoRollEventsView->setAddModulationEventFn(
        std::bind(&PianoRollViewController::addModulationEvent, this, _1, _2, _3));
    pianoRollEventsView->setAddMixerLevelEventFn(
        std::bind(&PianoRollViewController::addMixerLevelEvent, this, _1, _2, _3));
    pianoRollEventsView->setAddMixerBalanceEventFn(
        std::bind(&PianoRollViewController::addMixerBalanceEvent, this, _1, _2, _3));
    pianoRollEventsView->setAddControlChangeEventFn(
        std::bind(&PianoRollViewController::addControlChangeEvent, this, _1, _2, _3, _4));
    pianoRollEventsView->setAddKeyAftertouchEventFn(
        std::bind(&PianoRollViewController::addKeyAftertouchEvent, this, _1, _2, _3, _4));
    pianoRollEventsView->setAddChannelAftertouchEventFn(
        std::bind(&PianoRollViewController::addChannelAftertouchEvent, this, _1, _2, _3));
    pianoRollEventsView->setAddSysexEventFn(std::bind(&PianoRollViewController::addSysexEvent, this, _1, _2));
    pianoRollEventsView->setAddMetaEventFn(std::bind(&PianoRollViewController::addMetaEvent, this, _1, _2, _3));
    pianoRollEventsView->setDidSelectEventsFn(std::bind(&PianoRollViewController::didSelectEvents, this, _1, _2, _3));
    pianoRollEventsView->setSetSelectionStateFn(std::bind(&PianoRollViewController::setSelectionState, this, _1, _2));
    pianoRollEventsView->setAddTimeSignatureEventFn(
        std::bind(&PianoRollViewController::addTimeSignatureEvent, this, _1, _2));
}

// ---------------------------------------------------------------------------------------------------------------------
PianoRollViewController::PianoRollViewController(std::shared_ptr<PianoRollView> pianoRollView,
                                                 MelobaseCore::StudioController* studioController,
                                                 MelobaseCore::SequenceEditor* sequenceEditor)
    : _pianoRollView(pianoRollView), _studioController(studioController), _sequenceEditor(sequenceEditor) {
    using namespace std::placeholders;

    _currentChannel = 0;
    _currentMultiChannel = 0;
    _trackIndex = -1;

    _timeDivision = 480;
    _measureWidth = 100.0;
    _sequence = nullptr;

    _isKeyboardUpdated = true;

    _isSelectingFromEventsList = false;
    _oldEditionSelectedSegment = EDITION_MODE_CURSOR_SEGMENT;

    _pianoRollPropertiesViewController = new PianoRollPropertiesViewController(
        _pianoRollView->pianoRollUtilitiesView()->pianoRollPropertiesView(), _studioController->studio());

    setPianoRollEventsViewFns(_pianoRollView->mainView()->pianoRollEventsView());
    setPianoRollEventsViewFns(_pianoRollView->mainView()->pianoRollControllerEventsView());
    setPianoRollEventsViewFns(_pianoRollView->mainView()->pianoRollMetaEventView());

    _pianoRollView->mainView()->pianoRollEventsView()->setDidSetFocusStateFn(
        [=](PianoRollEventsView* sender, bool state) {
            _pianoRollView->mainView()->pianoRollHeaderView()->setFocusState(state);
        });

    _pianoRollView->mainView()->pianoRollControllerEventsView()->setDidSetFocusStateFn(
        [=](PianoRollEventsView* sender, bool state) {
            _pianoRollView->mainView()->controllerEventsRulerView()->setFocusState(state);
        });

    _pianoRollView->pianoRollUtilitiesView()->pianoRollEventsListView()->setNbEventsFn(
        std::bind(&PianoRollViewController::eventsListNbEvents, this, _1, _2));
    _pianoRollView->pianoRollUtilitiesView()->pianoRollEventsListView()->setEventAtIndexFn(
        std::bind(&PianoRollViewController::eventsListEventAtIndex, this, _1, _2, _3, _4, _5));
    _pianoRollView->pianoRollUtilitiesView()->pianoRollEventsListView()->setDidSelectEventsFn(
        std::bind(&PianoRollViewController::eventsListDidSelectEvents, this, _1, _2));
    _pianoRollView->pianoRollUtilitiesView()->pianoRollEventsListView()->setEventDidChangeFn(
        std::bind(&PianoRollViewController::eventsListEventDidChange, this, _1, _2, _3, _4, _5, _6, _7, _8, _9));
    _pianoRollView->pianoRollUtilitiesView()->pianoRollEventsListView()->tableView()->setDidPressUnhandledKeyFn(
        [=](MDStudio::TableView* tableView, unsigned int key) -> bool {
            if (key == KEY_DELETE) {
                deleteSelectedEvents();
                return true;
            }
            return false;
        });

    _pianoRollView->pianoRollUtilitiesView()->viewSelectionSegmentedControl()->setDidSelectSegmentFn(
        std::bind(&PianoRollViewController::utilitiesViewSelectionDidSelectSegment, this, _1, _2));

    _pianoRollView->mainView()->pianoRollHeaderView()->setSequenceInfosFn(
        std::bind(&PianoRollViewController::headerSequenceInfos, this, _1, _2, _3, _4, _5));
    _pianoRollView->mainView()->pianoRollHeaderView()->setNbEventsFn(
        std::bind(&PianoRollViewController::headerNbEvents, this, _1, _2));
    _pianoRollView->mainView()->pianoRollHeaderView()->setEventAtIndexFn(
        std::bind(&PianoRollViewController::headerEventAtIndex, this, _1, _2, _3, _4));
    _pianoRollView->mainView()->pianoRollHeaderView()->setNbAnnotationsFn(
        [this](PianoRollHeaderView* sender) -> unsigned int {
            if (_sequence) return static_cast<unsigned int>(_sequence->annotations.size());
            return 0;
        });
    _pianoRollView->mainView()->pianoRollHeaderView()->setAnnotationAtIndexFn(
        [this](PianoRollHeaderView* sender, int index, std::shared_ptr<MelobaseCore::SequenceAnnotation>* annotation) {
            if (_sequence) *annotation = _sequence->annotations[index];
        });
    _pianoRollView->mainView()->pianoRollHeaderView()->setDidSetCursorTickPosFn(
        std::bind(&PianoRollViewController::headerDidSetCursorTickPos, this, _1, _2));

    _pianoRollView->mainView()->pianoRollMetaEventView()->setControllerEventsMode(
        PianoRollEventsView::EndOfTrackTimeSignatureControllerEventsMode);

    _pianoRollView->mainView()->pianoRollEventsScrollView()->setPosChangedFn(
        std::bind(&PianoRollViewController::pianoRollEventsScrollViewPosChanged, this, _1, _2));

    _pianoRollView->mainView()->keyboard()->setKeyPressedFn(
        std::bind(&PianoRollViewController::pianoKeyPressed, this, _1, _2));
    _pianoRollView->mainView()->keyboard()->setKeyReleasedFn(
        std::bind(&PianoRollViewController::pianoKeyReleased, this, _1, _2));
    _pianoRollView->mainView()->keyboard()->setScrollFn(
        std::bind(&PianoRollViewController::keyboardScroll, this, _1, _2));

    _studioController->setDidGoToBeginningFn(
        std::bind(&PianoRollViewController::studioControllerDidGoToBeginning, this, _1));

    _studioDidPlayNoteFn = std::shared_ptr<MDStudio::Studio::didPlayNoteFnType>(new MDStudio::Studio::didPlayNoteFnType(
        std::bind(&PianoRollViewController::studioDidPlayNote, this, _1, _2, _3, _4, _5, _6, _7)));
    _studioController->studio()->addDidPlayNoteFn(_studioDidPlayNoteFn);
    _studioDidReleaseNoteFn =
        std::shared_ptr<MDStudio::Studio::didReleaseNoteFnType>(new MDStudio::Studio::didReleaseNoteFnType(
            std::bind(&PianoRollViewController::studioDidReleaseNote, this, _1, _2, _3, _4, _5, _6)));
    _studioController->studio()->addDidReleaseNoteFn(_studioDidReleaseNoteFn);
    _studioDidSetInstrumentFn =
        std::shared_ptr<MDStudio::Studio::didSetInstrumentFnType>(new MDStudio::Studio::didSetInstrumentFnType(
            std::bind(&PianoRollViewController::studioDidSetInstrument, this, _1, _2, _3, _4, _5, _6)));
    _studioController->studio()->addDidSetInstrumentFn(_studioDidSetInstrumentFn);

    _pianoRollView->editionSegmentedControl()->setDidSelectSegmentFn(
        std::bind(&PianoRollViewController::editionModeSegmentedControlDidSelectSegment, this, _1, _2));

    _pianoRollView->zoomSlider()->setPosChangedFn(
        std::bind(&PianoRollViewController::zoomSliderPosChanged, this, _1, _2));

    for (int channel = 0; channel < STUDIO_MAX_CHANNELS; ++channel)
        _pianoRollView->visibleChannelButtons()[channel]->setStateWithModifierDidChangeFn(
            std::bind(&PianoRollViewController::visibleChannelButtonStateDidChange, this, _1, _2, _3));

    _pianoRollView->allVisibleChannelButton()->setClickedFn(
        std::bind(&PianoRollViewController::allVisibleChannelButtonClicked, this, _1));

    _pianoRollView->visibleControllerPaneButton()->setStateDidChangeFn(
        std::bind(&PianoRollViewController::visibleControllerPaneButtonStateDidChange, this, _1, _2));
    _pianoRollView->visiblePropertiesPaneButton()->setStateDidChangeFn(
        std::bind(&PianoRollViewController::visiblePropertiesPaneButtonStateDidChange, this, _1, _2));

    _pianoRollView->quantizeButton()->setClickedFn(
        std::bind(&PianoRollViewController::quantizeButtonClicked, this, _1));
    _pianoRollView->quantizeButton()->setIsEnabled(false);

    _pianoRollView->addFlagButton()->setClickedFn([this](MDStudio::Button* sender) {
        auto tick = _studioController->studio()->metronome()->tick();
        if (std::any_of(_sequence->annotations.begin(), _sequence->annotations.end(),
                        [tick](std::shared_ptr<MelobaseCore::SequenceAnnotation> annotation) {
                            return annotation->tickCount == tick;
                        })) {
            MDStudio::Platform::sharedInstance()->beep();
            return;
        }

        auto annotation = std::make_shared<MelobaseCore::SequenceAnnotation>();
        annotation->tickCount = _studioController->studio()->metronome()->tick();
        addAnnotations({annotation});
    });
    _pianoRollView->addFlagButton()->setIsEnabled(false);

    _pianoRollView->removeFlagButton()->setClickedFn([this](MDStudio::Button* sender) {
        std::vector<std::shared_ptr<MelobaseCore::SequenceAnnotation>> annotationsToRemove;
        auto tick = _studioController->studio()->metronome()->tick();
        std::copy_if(_sequence->annotations.begin(), _sequence->annotations.end(),
                     std::back_inserter(annotationsToRemove),
                     [tick](std::shared_ptr<MelobaseCore::SequenceAnnotation> annotation) {
                         return annotation->tickCount == tick;
                     });
        removeAnnotations(annotationsToRemove);
    });
    _pianoRollView->removeFlagButton()->setIsEnabled(false);

    _pianoRollView->removeAllFlagsButton()->setClickedFn(
        [this](MDStudio::Button* sender) { removeAnnotations(_sequence->annotations); });
    _pianoRollView->removeAllFlagsButton()->setIsEnabled(false);

    _pianoRollView->goToPreviousFlagButton()->setClickedFn(
        [this](MDStudio::Button* sender) { goToPreviousAnnotation(); });
    _pianoRollView->goToPreviousFlagButton()->setIsEnabled(false);

    _pianoRollView->goToNextFlagButton()->setClickedFn([this](MDStudio::Button* sender) { goToNextAnnotation(); });
    _pianoRollView->goToNextFlagButton()->setIsEnabled(false);

    _pianoRollPropertiesViewController->setChannelDidChangeFn(
        std::bind(&PianoRollViewController::pianoRollPropertiesChannelDidChange, this, _1, _2));
    _pianoRollPropertiesViewController->setVelocityDidChangeFn(
        std::bind(&PianoRollViewController::pianoRollPropertiesVelocityDidChange, this, _1, _2));
    _pianoRollPropertiesViewController->setProgramDidChangeFn(
        std::bind(&PianoRollViewController::pianoRollPropertiesProgramDidChange, this, _1, _2));
    _pianoRollPropertiesViewController->setTimeSignatureDidChangeFn(
        std::bind(&PianoRollViewController::pianoRollPropertiesTimeSignatureDidChange, this, _1, _2, _3));
    _pianoRollPropertiesViewController->setSysexDataDidChangeFn(
        std::bind(&PianoRollViewController::pianoRollPropertiesSysexDataDidChange, this, _1, _2));
    _pianoRollPropertiesViewController->setMetaDataDidChangeFn(
        std::bind(&PianoRollViewController::pianoRollPropertiesMetaDataDidChange, this, _1, _2));
    _pianoRollPropertiesViewController->setPitchDidChangeFn(
        std::bind(&PianoRollViewController::pianoRollPropertiesPitchDidChange, this, _1, _2));

    _pianoRollView->mainView()->controllerSegmentedControlV()->setDidSelectSegment(
        std::bind(&PianoRollViewController::controllerSegmentedControlVDidSelectSegment, this, _1, _2));

    _pianoRollView->mainView()->controlChangeComboBox()->setNbRowsFn(
        std::bind(&PianoRollViewController::controlChangeComboBoxNbRows, this, _1));
    _pianoRollView->mainView()->controlChangeComboBox()->setViewForRowFn(
        std::bind(&PianoRollViewController::controlChangeComboBoxViewForRow, this, _1, _2));
    _pianoRollView->mainView()->controlChangeComboBox()->setDidSelectRowFn(
        std::bind(&PianoRollViewController::controlChangeComboBoxDidSelectRow, this, _1, _2));
    _pianoRollView->mainView()->controlChangeComboBox()->setDidHoverRowFn(
        std::bind(&PianoRollViewController::controlChangeComboBoxDidHoverRow, this, _1, _2));
    _pianoRollView->mainView()->controlChangeComboBox()->setDidSetFocusStateFn(
        std::bind(&PianoRollViewController::controlChangeComboBoxDidSetFocusState, this, _1, _2));
    _pianoRollView->mainView()->controlChangeComboBox()->reload();

    _pianoRollView->mainView()->metaTypeComboBox()->setNbRowsFn(
        std::bind(&PianoRollViewController::metaTypeComboBoxNbRows, this, _1));
    _pianoRollView->mainView()->metaTypeComboBox()->setViewForRowFn(
        std::bind(&PianoRollViewController::metaTypeComboBoxViewForRow, this, _1, _2));
    _pianoRollView->mainView()->metaTypeComboBox()->setDidSelectRowFn(
        std::bind(&PianoRollViewController::metaTypeComboBoxDidSelectRow, this, _1, _2));
    _pianoRollView->mainView()->metaTypeComboBox()->setDidHoverRowFn(
        std::bind(&PianoRollViewController::metaTypeComboBoxDidHoverRow, this, _1, _2));
    _pianoRollView->mainView()->metaTypeComboBox()->setDidSetFocusStateFn(
        std::bind(&PianoRollViewController::metaTypeComboBoxDidSetFocusState, this, _1, _2));
    _pianoRollView->mainView()->metaTypeComboBox()->reload();

    _pianoRollView->currentNoteSegmentedControl()->setSelectedSegment(2);

    _pianoRollView->mainView()->zoneSelectionSegmentedControl()->setDidSelectSegmentFn(
        std::bind(&PianoRollViewController::zoneSelectionSegmentedControlDidSelectSegment, this, _1, _2));

    _pianoRollView->editionSegmentedControl()->setSelectedSegment(0);

    _pianoRollView->mainView()->controllerSegmentedControlV()->setSelectedSegment(0);
    _pianoRollView->mainView()->controlChangeComboBox()->setSelectedRow(64);  // Sustain
    _pianoRollView->mainView()->metaTypeComboBox()->setSelectedRow(81);

    setIsEditionAvailable(false);

    updatePianoRoll();
    updateEventList();

    MDStudio::Platform::sharedInstance()->invoke([=] { scrollToVerticalCenter(); });
}

// ---------------------------------------------------------------------------------------------------------------------
PianoRollViewController::~PianoRollViewController() {
    _studioController->studio()->removeDidPlayNoteFn(_studioDidPlayNoteFn);
    _studioController->studio()->removeDidReleaseNoteFn(_studioDidReleaseNoteFn);
    _studioController->studio()->removeDidSetInstrumentFn(_studioDidSetInstrumentFn);

    delete _pianoRollPropertiesViewController;
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::addAnnotations(
    std::vector<std::shared_ptr<MelobaseCore::SequenceAnnotation>> annotations) {
    if (_willModifyAnnotationsFn) _willModifyAnnotationsFn(this);
    _sequenceEditor->undoManager()->pushFn([this, annotations]() { removeAnnotations(annotations); });
    for (auto annotation : annotations) _sequence->annotations.emplace_back(annotation);
    _pianoRollView->setDirty();
    updateFlagControls();
    if (_didModifyAnnotationsFn) _didModifyAnnotationsFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::removeAnnotations(
    std::vector<std::shared_ptr<MelobaseCore::SequenceAnnotation>> annotations) {
    if (_willModifyAnnotationsFn) _willModifyAnnotationsFn(this);
    _sequenceEditor->undoManager()->pushFn([this, annotations]() { addAnnotations(annotations); });
    for (auto annotation : annotations)
        _sequence->annotations.erase(
            std::remove(_sequence->annotations.begin(), _sequence->annotations.end(), annotation),
            _sequence->annotations.end());
    _pianoRollView->setDirty();
    updateFlagControls();
    if (_didModifyAnnotationsFn) _didModifyAnnotationsFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
bool PianoRollViewController::goToPreviousAnnotation(bool isCheckingOnly) {
    if (!_sequence ||
        (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusRecording) ||
        (_trackIndex < 0))
        return false;

    if (!isCheckingOnly) _studioController->stop();

    UInt32 tick = _studioController->studio()->metronome()->tick();

    auto annotations = _sequence->annotations;
    std::sort(annotations.begin(), annotations.end(),
              [](std::shared_ptr<MelobaseCore::SequenceAnnotation> a,
                 std::shared_ptr<MelobaseCore::SequenceAnnotation> b) { return a->tickCount < b->tickCount; });

    auto it = std::find_if(
        annotations.rbegin(), annotations.rend(),
        [tick](std::shared_ptr<MelobaseCore::SequenceAnnotation> annotation) { return tick > annotation->tickCount; }

    );

    if (it != std::rend(annotations)) {
        if (!isCheckingOnly) {
            _studioController->studio()->metronome()->moveToTick((*it)->tickCount);
            updateCursorTickPos(true);
        }
        return true;
    }

    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
bool PianoRollViewController::goToNextAnnotation(bool isCheckingOnly) {
    if (!_sequence ||
        (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusRecording) ||
        (_trackIndex < 0))
        return false;

    if (!isCheckingOnly) _studioController->stop();

    UInt32 tick = _studioController->studio()->metronome()->tick();

    auto annotations = _sequence->annotations;
    std::sort(annotations.begin(), annotations.end(),
              [](std::shared_ptr<MelobaseCore::SequenceAnnotation> a,
                 std::shared_ptr<MelobaseCore::SequenceAnnotation> b) { return a->tickCount < b->tickCount; });

    auto it = std::find_if(
        annotations.begin(), annotations.end(),
        [tick](std::shared_ptr<MelobaseCore::SequenceAnnotation> annotation) { return tick < annotation->tickCount; });

    if (it != std::end(annotations)) {
        if (!isCheckingOnly) {
            _studioController->studio()->metronome()->moveToTick((*it)->tickCount);
            updateCursorTickPos(true);
        }
        return true;
    }

    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::pianoKeyPressed(KeyboardV* sender, int pitch) {
    float currentVelocity = _pianoRollView->mainView()->currentVelocitySlider()->pos() /
                            _pianoRollView->mainView()->currentVelocitySlider()->max();
    _studioController->keyPressed(pitch, currentVelocity, _currentChannel);
    _pianoRollView->mainView()->pianoRollEventsView()->setHighlightPitchState(_currentChannel, pitch, true);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::pianoKeyReleased(KeyboardV* sender, int pitch) {
    _studioController->keyReleased(pitch, 0.5f, _currentChannel);
    _pianoRollView->mainView()->pianoRollEventsView()->setHighlightPitchState(_currentChannel, pitch, false);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::headerSequenceInfos(PianoRollHeaderView* sender, int* timeDivision,
                                                  std::vector<unsigned int>* totalNbTicks,
                                                  std::vector<unsigned int>* eotTickCounts, bool* areTicksAbs) {
    *timeDivision = _timeDivision;
    *totalNbTicks = _totalTickCounts;
    *eotTickCounts = _eotTickCounts;
    *areTicksAbs = _studioController->status() != MelobaseCore::StudioController::StudioControllerStatusRecording;
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::headerDidSetCursorTickPos(PianoRollHeaderView* sender, unsigned int cursorTickPos) {
    _pianoRollView->mainView()->pianoRollEventsView()->setCursorTickPos(cursorTickPos);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::sequenceInfos(PianoRollEventsView* sender, int* timeDivision,
                                            std::vector<unsigned int>* totalNbTicks,
                                            std::vector<unsigned int>* eotTickCounts, bool* areTicksAbs) {
    *timeDivision = _timeDivision;
    *totalNbTicks = _totalTickCounts;
    *eotTickCounts = _eotTickCounts;
    *areTicksAbs = _studioController->status() != MelobaseCore::StudioController::StudioControllerStatusRecording;
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int PianoRollViewController::currentNbEvents(int trackIndex) {
    if (trackIndex >= 0) {
        if (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusRecording) {
            return static_cast<int>(_studioController->sequencer()->sequence()->data.tracks[trackIndex].events.size());
        } else {
            if (_sequence)
                return static_cast<unsigned int>(_sequence->data.tracks[trackIndex]->clips[0]->events.size());
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::currentEventAtIndex(int trackIndex, int index,
                                                  std::shared_ptr<MelobaseCore::Event>* event) {
    if (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusRecording) {
        MDStudio::Event e = _studioController->sequencer()->sequence()->data.tracks[trackIndex].events[index];
        *event = std::make_shared<MelobaseCore::ChannelEvent>(e.type, e.channel, e.tickCount, 0, e.param1, e.param2, 0);
    } else {
        *event = _sequence->data.tracks[trackIndex]->clips[0]->events[index];
    }
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int PianoRollViewController::headerNbEvents(PianoRollHeaderView* sender, int track) {
    return currentNbEvents(track);
}

// ---------------------------------------------------------------------------------------------------------------------
int PianoRollViewController::nbEvents(PianoRollEventsView* sender, int track) { return currentNbEvents(track); }

// ---------------------------------------------------------------------------------------------------------------------
int PianoRollViewController::nbAnnotations(PianoRollEventsView* sender) {
    if (_sequence) return static_cast<unsigned int>(_sequence->annotations.size());
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
int PianoRollViewController::eventsListNbEvents(const PianoRollEventsListView* sender, int track) {
    if (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusRecording) return 0;

    return currentNbEvents(track);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::headerEventAtIndex(PianoRollHeaderView* sender, int track, int index,
                                                 std::shared_ptr<MelobaseCore::Event>* event) {
    currentEventAtIndex(track, index, event);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::eventAtIndex(PianoRollEventsView* sender, int track, int index,
                                           std::shared_ptr<MelobaseCore::Event>* event) {
    currentEventAtIndex(track, index, event);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::annotationAtIndex(PianoRollEventsView* sender, int index,
                                                std::shared_ptr<MelobaseCore::SequenceAnnotation>* annotation) {
    *annotation = _sequence->annotations.at(index);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::eventsListEventAtIndex(const PianoRollEventsListView* sender, int track, int index,
                                                     std::shared_ptr<MelobaseCore::Event>* event,
                                                     bool* isChannelEditionAvail) {
    currentEventAtIndex(track, index, event);
    *isChannelEditionAvail = _sequence->data.tracks[track]->channel == SEQUENCE_TRACK_MULTI_CHANNEL;
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::eventsListDidSelectEvents(PianoRollEventsListView* sender,
                                                        std::vector<std::shared_ptr<MelobaseCore::Event>> events) {
    _isSelectingFromEventsList = true;

    std::vector<std::shared_ptr<MelobaseCore::Event>> mainEvents, metaEvents, controllerEvents;

    for (auto event : events) {
        auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(event);
        switch (channelEvent->type()) {
            case CHANNEL_EVENT_TYPE_NOTE:
                mainEvents.push_back(event);
                break;
            case CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE:
            case CHANNEL_EVENT_TYPE_META_END_OF_TRACK:
                metaEvents.push_back(event);
                break;
            default:
                controllerEvents.push_back(event);
        }
    }
    _pianoRollView->mainView()->pianoRollEventsView()->selectEvents(mainEvents);
    _pianoRollView->mainView()->pianoRollMetaEventView()->selectEvents(metaEvents);
    _pianoRollView->mainView()->pianoRollControllerEventsView()->selectEvents(controllerEvents);
    _isSelectingFromEventsList = false;
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::eventsListEventDidChange(PianoRollEventsListView* sender,
                                                       std::shared_ptr<MelobaseCore::Event> event, UInt32 tickCount,
                                                       UInt32 length, UInt8 channel, SInt32 param1, SInt32 param2,
                                                       SInt32 param3, std::vector<UInt8> data) {
    if (_willModifySequenceFn) _willModifySequenceFn(this);
    _sequenceEditor->updateEvent(_sequence->data.tracks[_trackIndex], event, tickCount, length, channel, param1, param2,
                                 param3, data);
    if (_didModifySequenceFn) _didModifySequenceFn(this);

    MDStudio::Platform::sharedInstance()->invoke([=] {
        if (_pianoRollView->pianoRollUtilitiesView()->pianoRollEventsListView()->tableView()->isInChain() &&
            _pianoRollView->pianoRollUtilitiesView()
                    ->pianoRollEventsListView()
                    ->tableView()
                    ->responderChain()
                    ->capturedResponders()
                    .size() == 0)
            _pianoRollView->pianoRollUtilitiesView()->pianoRollEventsListView()->tableView()->captureFocus();
    });
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::didSetCursorTickPos(PianoRollEventsView* sender, unsigned int cursorTickPos) {
    if (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusRecording) {
        _studioController->stopRecordingAndMetronome();
    } else {
        _studioController->stop();
    }

    if (cursorTickPos == _studioController->studio()->metronome()->tick()) return;

    _studioController->studio()->moveMetronomeToTick(cursorTickPos);

    if (sender == _pianoRollView->mainView()->pianoRollEventsView().get()) {
        _pianoRollView->mainView()->pianoRollControllerEventsView()->setCursorTickPos(cursorTickPos, false);
        _pianoRollView->mainView()->pianoRollMetaEventView()->setCursorTickPos(cursorTickPos, false);
    } else if (sender == _pianoRollView->mainView()->pianoRollControllerEventsView().get()) {
        _pianoRollView->mainView()->pianoRollEventsView()->setCursorTickPos(cursorTickPos, false);
        _pianoRollView->mainView()->pianoRollMetaEventView()->setCursorTickPos(cursorTickPos, false);
    } else {
        // From end of track view
        _pianoRollView->mainView()->pianoRollEventsView()->setCursorTickPos(cursorTickPos, false);
        _pianoRollView->mainView()->pianoRollControllerEventsView()->setCursorTickPos(cursorTickPos, false);
    }

    _pianoRollView->mainView()->scrollToVisibleCursor();

    updateFlagControls();

    if (_didSetCursorTickPosFn) _didSetCursorTickPosFn(this, cursorTickPos);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::didFinishSettingCursorTickPos(PianoRollEventsView* sender) {
    _studioController->studio()->dumpStates(STUDIO_SOURCE_USER);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::willSetSelectionRegion(PianoRollEventsView* sender) {
    if (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusRecording) {
        _studioController->stopRecordingAndMetronome();
    } else {
        _studioController->stop();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::scrollToVisibleRect(PianoRollEventsView* pianoRollEventsView) {
    // Scroll in order to keep the last selection region point visible
    MDStudio::Rect r;
    r.origin = pianoRollEventsView->lastSelectionPt();
    r.size = MDStudio::makeSize(5.0f, 5.0f);

    if (pianoRollEventsView == _pianoRollView->mainView()->pianoRollEventsView().get()) {
        // Horizontal and vertical
        _pianoRollView->mainView()->scrollToVisibleRect(r, false);
    } else if (pianoRollEventsView == _pianoRollView->mainView()->pianoRollControllerEventsView().get()) {
        // Horizontal only
        _pianoRollView->mainView()->scrollToVisibleRect(r, true);
    } else if (pianoRollEventsView == _pianoRollView->mainView()->pianoRollMetaEventView().get()) {
        // Horizontal only
        _pianoRollView->mainView()->scrollToVisibleRect(r, true);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::didSetSelectionRegion(PianoRollEventsView* sender) { scrollToVisibleRect(sender); }

// ---------------------------------------------------------------------------------------------------------------------
std::vector<unsigned int> PianoRollViewController::getMeasureTicks() {
    std::vector<unsigned int> measureTicks;

    if (_trackIndex < 0) return measureTicks;

    unsigned int tick = 0L, refTick = 0L;
    int numerator = 0;

    for (auto event : _sequence->data.tracks[0]->clips[0]->events) {
        auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(event);
        tick = channelEvent->tickCount();
        if (channelEvent->type() == CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE) {
            if (numerator) {
                for (unsigned int t = refTick; t < tick; t += _timeDivision * numerator) {
                    measureTicks.push_back(t);
                }
            }
            numerator = channelEvent->param1();
            refTick = tick;
        }
    }

    if (numerator) {
        for (unsigned int t = refTick; t < _totalTickCounts[_trackIndex]; t += _timeDivision * numerator) {
            measureTicks.push_back(t);
        }
    }

    return measureTicks;
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::previousMeasure(PianoRollEventsView* sender) { goToPreviousMeasure(); }

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::nextMeasure(PianoRollEventsView* sender) { goToNextMeasure(); }

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::deleteEvents(PianoRollEventsView* sender) { deleteSelectedEvents(); }

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::willMoveEvents(PianoRollEventsView* sender) {
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _previousDeltaTicks = 0;
    _previousDeltaPitch = 0;
    _previousDeltaValue = 0;

    _sequenceEditor->undoManager()->beginGroup();
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::moveEvents(PianoRollEventsView* sender, int deltaTicks, int deltaPitch, int deltaValue,
                                         bool isResizing, bool isAddingNote) {
    if (!isResizing && !isAddingNote) {
        std::vector<std::shared_ptr<MelobaseCore::Event>> selectedEvents = selectedEventsWithAssociates();
        if (sender->mode() == PianoRollEventsView::MoveMode) {
            // If no events are selected, we use the active one
            if (selectedEvents.empty()) selectedEvents.push_back(sender->activeEvent());
        }

        // Adjust the delta tick in order to ensure that no event has a negative tick after the move operation.
        if (deltaTicks < 0) {
            for (auto event : selectedEvents) {
                auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(event);
                auto tick = channelEvent->tickCount() - _previousDeltaTicks;
                if (-deltaTicks > tick) deltaTicks = -tick;
            }
        }

        std::vector<std::shared_ptr<MelobaseCore::Event>> selectedChannelEvents;
        std::vector<std::shared_ptr<MelobaseCore::Event>> selectedFirstTrackEvents;

        for (auto event : selectedEvents) {
            auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(event);
            if ((channelEvent->type() != CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE) &&
                (channelEvent->type() != CHANNEL_EVENT_TYPE_META_SET_TEMPO)) {
                selectedChannelEvents.push_back(event);
            } else {
                selectedFirstTrackEvents.push_back(event);
            }
        }

        // Restore position
        _sequenceEditor->moveEvents(_sequence->data.tracks[_trackIndex], selectedChannelEvents, -_previousDeltaTicks,
                                    -_previousDeltaPitch, -_previousDeltaValue, true, true);
        _sequenceEditor->moveEvents(_sequence->data.tracks[0], selectedFirstTrackEvents, -_previousDeltaTicks, 0,
                                    -_previousDeltaValue, true, true);

        // Clear the all the move operations in the group
        _sequenceEditor->undoManager()->clearCurrentGroup();

        if (!_sequenceEditor->canMoveEvents(_sequence->data.tracks[_trackIndex], selectedChannelEvents, deltaTicks,
                                            !isResizing ? deltaPitch : 0, deltaValue) ||
            !_sequenceEditor->canMoveEvents(_sequence->data.tracks[0], selectedFirstTrackEvents, deltaTicks, 0,
                                            deltaValue)) {
            _sequenceEditor->moveEvents(_sequence->data.tracks[_trackIndex], selectedChannelEvents, _previousDeltaTicks,
                                        _previousDeltaPitch, _previousDeltaValue, true, true);
            _sequenceEditor->moveEvents(_sequence->data.tracks[0], selectedFirstTrackEvents, _previousDeltaTicks, 0,
                                        _previousDeltaValue, true, true);
            return;
        }

        _sequenceEditor->moveEvents(_sequence->data.tracks[_trackIndex], selectedChannelEvents, deltaTicks, deltaPitch,
                                    deltaValue, true, true);
        _sequenceEditor->moveEvents(_sequence->data.tracks[0], selectedFirstTrackEvents, deltaTicks, 0, deltaValue,
                                    true, true);

        _previousDeltaTicks = deltaTicks;
        _previousDeltaPitch = deltaPitch;
        _previousDeltaValue = deltaValue;

    } else {
        std::vector<std::shared_ptr<MelobaseCore::Event>> events = selectedEventsWithAssociates();
        if ((sender->mode() == PianoRollEventsView::ResizeMode) || isAddingNote) {
            // If no events are selected, we use the active one
            if (events.empty()) events.push_back(sender->activeEvent());
        }

        // Restore
        _sequenceEditor->resizeEvents(_sequence->data.tracks[_trackIndex], events, -_previousDeltaTicks);

        // Clear the last move operations from the undo group
        _sequenceEditor->undoManager()->clearCurrentGroup(_sequenceEditor->undoManager()->nbOperationsInGroup() - 2);

        if (!_sequenceEditor->canResizeEvents(_sequence->data.tracks[_trackIndex], events, deltaTicks)) {
            _sequenceEditor->resizeEvents(_sequence->data.tracks[_trackIndex], events, _previousDeltaTicks);
            return;
        }

        _sequenceEditor->resizeEvents(_sequence->data.tracks[_trackIndex], events, deltaTicks);

        _previousDeltaTicks = deltaTicks;
    }

    scrollToVisibleRect(sender);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::didMoveEvents(PianoRollEventsView* sender) {
    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::addControllerEvents(unsigned int tickPos) {
    //
    // Get the controller states at the tick position
    //

    bool isInstrumentSet = false;
    bool isMixerLevelSet = false;
    bool isMixerBalanceSet = false;
    bool isReverbSet = false;
    bool isChorusSet = false;

    unsigned int tickCount = 0;
    for (auto event : _sequence->data.tracks[_trackIndex]->clips[0]->events) {
        auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(event);
        if (channelEvent->channel() == _currentChannel) {
            tickCount = channelEvent->tickCount();
            if (tickCount >= tickPos) break;
            if (channelEvent->type() == CHANNEL_EVENT_TYPE_PROGRAM_CHANGE) {
                isInstrumentSet = true;
            } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_MIXER_LEVEL_CHANGE) {
                isMixerLevelSet = true;
            } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_MIXER_BALANCE_CHANGE) {
                isMixerBalanceSet = true;
            } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_CONTROL_CHANGE) {
                if (channelEvent->param1() == 91) {
                    isReverbSet = true;
                } else if (channelEvent->param1() == 93) {
                    isChorusSet = true;
                }
            }
        }
    }

    //
    // Add program change if necessary
    //

    // If instrument not set
    if (!isInstrumentSet) {
        // Add PC at beginning
        int currentInstrument = _studioController->studio()->instrument(_currentChannel);
        auto e = std::shared_ptr<MelobaseCore::ChannelEvent>(new MelobaseCore::ChannelEvent(
            CHANNEL_EVENT_TYPE_PROGRAM_CHANGE, _currentChannel, 0, 0, currentInstrument, 0));
        _sequenceEditor->addEvent(_sequence->data.tracks[_trackIndex], e, true, true);
    }

    //
    // Add mixer level if necessary
    //

    // If mixer level not set
    if (!isMixerLevelSet) {
        // Add mixer balance at beginning
        int currentMixerLevel = (int)roundf(_studioController->studio()->mixerLevel(_currentChannel) * 127);
        auto e = std::shared_ptr<MelobaseCore::Event>(new MelobaseCore::ChannelEvent(
            CHANNEL_EVENT_TYPE_MIXER_LEVEL_CHANGE, _currentChannel, 0, 0, currentMixerLevel, 0));
        _sequenceEditor->addEvent(_sequence->data.tracks[_trackIndex], e, true, true);
    }

    //
    // Add mixer balance if necessary
    //

    // If mixer balance not set
    if (!isMixerBalanceSet) {
        // Add mixer balance at beginning
        int currentMixerBalance =
            (int)roundf(127.0f * ((_studioController->studio()->mixerBalance(_currentChannel) + 1.0f) / 2.0f));
        auto e = std::shared_ptr<MelobaseCore::ChannelEvent>(new MelobaseCore::ChannelEvent(
            CHANNEL_EVENT_TYPE_MIXER_BALANCE_CHANGE, _currentChannel, 0, 0, currentMixerBalance, 0));
        _sequenceEditor->addEvent(_sequence->data.tracks[_trackIndex], e, true, true);
    }

    //
    // Add reverb if necessary
    //

    // If reverb not set
    if (!isReverbSet) {
        // Add reverb at beginning
        int currentReverb = (int)roundf(_studioController->studio()->controlValue(_currentChannel, 91));
        auto e = std::shared_ptr<MelobaseCore::Event>(new MelobaseCore::ChannelEvent(
            CHANNEL_EVENT_TYPE_CONTROL_CHANGE, _currentChannel, 0, 0, 91, currentReverb));
        _sequenceEditor->addEvent(_sequence->data.tracks[_trackIndex], e, true, true);
    }

    //
    // Add chorus if necessary
    //

    // If chorus not set
    if (!isChorusSet) {
        // Add chorus at beginning
        int currentChorus = (int)roundf(_studioController->studio()->controlValue(_currentChannel, 93));
        auto e = std::shared_ptr<MelobaseCore::Event>(new MelobaseCore::ChannelEvent(
            CHANNEL_EVENT_TYPE_CONTROL_CHANGE, _currentChannel, 0, 0, 93, currentChorus));
        _sequenceEditor->addEvent(_sequence->data.tracks[_trackIndex], e, true, true);
    }

    // Note: We do not notify yet that to the delegate that the sequence is modified because the user is resizing the
    // note being added.
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int PianoRollViewController::quantizedResTickCount() {
    unsigned int resTickCount = 480;
    double eventTickWidth = _pianoRollView->mainView()->pianoRollEventsView()->eventTickWidth();

    if (eventTickWidth > 0.2) {
        resTickCount = 480 / 8;
    } else if (eventTickWidth > 0.07) {
        resTickCount = 480 / 4;
    } else if (eventTickWidth > 0.04) {
        resTickCount = 480 / 2;
    }

    return resTickCount;
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int PianoRollViewController::quantizedTickPos(unsigned int tickPos) {
    unsigned int resTickCount = quantizedResTickCount();

    unsigned int t = tickPos / resTickCount;
    unsigned int mod = tickPos % resTickCount;

    if (mod > resTickCount / 2) t++;
    return t * resTickCount;
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::addNoteEvent(PianoRollEventsView* sender, unsigned int tickPos, int pitch) {
    float currentVelocity = _pianoRollView->mainView()->currentVelocitySlider()->pos();
    // int currentNote = _pianoRollView->currentNoteSegmentedControl()->selectedSegment();
    // UInt32 currentNoteTickCounts[] = { 480 / 4, 480 / 2, 480, 480 * 2, 480 * 3, 480 * 4 };
    // UInt32 tickCount = currentNoteTickCounts[0] * (static_cast<float>(denominator) / 4.0f);

    _pianoRollView->mainView()->pianoRollEventsView()->clearEventSelection();

    tickPos = _pianoRollView->quantizeNewEventsButton()->state() ? quantizedTickPos(tickPos) : tickPos;

    auto e1 = std::shared_ptr<MelobaseCore::ChannelEvent>(new MelobaseCore::ChannelEvent(
        CHANNEL_EVENT_TYPE_NOTE, _currentChannel, tickPos, 24, pitch, static_cast<SInt32>(currentVelocity), 64));

    // Ensure that the channel is visible
    if (!_pianoRollView->visibleChannelButtons()[e1->channel()]->state())
        _pianoRollView->visibleChannelButtons()[e1->channel()]->setState(true);

    // Check if no overlap will occur
    std::vector<std::shared_ptr<MelobaseCore::Event>> eventsToAdd({e1});
    if (!_sequenceEditor->canAddEvents(_sequence->data.tracks[_trackIndex], eventsToAdd)) {
        MDStudio::Platform::sharedInstance()->beep();
        return;
    }

    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _previousDeltaTicks = 0;

    _sequenceEditor->undoManager()->beginGroup();

    addControllerEvents(tickPos);

    _sequenceEditor->addEvent(_sequence->data.tracks[_trackIndex], e1, true, true);

    // Add an initial resize event
    _sequenceEditor->resizeEvents(_sequence->data.tracks[_trackIndex], {e1}, 0);

    sender->setAddedEvent(e1);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::addSustainEvent(PianoRollEventsView* sender, unsigned int tickPos, int value) {
    _pianoRollView->mainView()->pianoRollEventsView()->clearEventSelection();

    tickPos = _pianoRollView->quantizeNewEventsButton()->state() ? quantizedTickPos(tickPos) : tickPos;

    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();
    auto e = std::shared_ptr<MelobaseCore::ChannelEvent>(
        new MelobaseCore::ChannelEvent(CHANNEL_EVENT_TYPE_SUSTAIN, _currentChannel, tickPos, 0, value, -1));

    // Ensure that the channel is visible
    if (!_pianoRollView->visibleChannelButtons()[e->channel()]->state())
        _pianoRollView->visibleChannelButtons()[e->channel()]->setState(true);

    _sequenceEditor->addEvent(_sequence->data.tracks[_trackIndex], e, true, true);

    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::addProgramChangeEvent(PianoRollEventsView* sender, unsigned int tickPos) {
    _pianoRollView->mainView()->pianoRollEventsView()->clearEventSelection();

    tickPos = _pianoRollView->quantizeNewEventsButton()->state() ? quantizedTickPos(tickPos) : tickPos;

    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();
    auto e = std::shared_ptr<MelobaseCore::ChannelEvent>(
        new MelobaseCore::ChannelEvent(CHANNEL_EVENT_TYPE_PROGRAM_CHANGE, _currentChannel, tickPos, 0,
                                       _studioController->studio()->instrument(_currentChannel), 0));

    // Ensure that the channel is visible
    if (!_pianoRollView->visibleChannelButtons()[e->channel()]->state())
        _pianoRollView->visibleChannelButtons()[e->channel()]->setState(true);

    _sequenceEditor->addEvent(_sequence->data.tracks[_trackIndex], e, true, true);

    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::addTempoEvent(PianoRollEventsView* sender, unsigned int tickPos, int bpm) {
    // Sanity check
    if (bpm == 0) {
        MDStudio::Platform::sharedInstance()->beep();
        return;
    }

    _pianoRollView->mainView()->pianoRollEventsView()->clearEventSelection();

    tickPos = _pianoRollView->quantizeNewEventsButton()->state() ? quantizedTickPos(tickPos) : tickPos;

    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();

    auto e = std::shared_ptr<MelobaseCore::ChannelEvent>(
        new MelobaseCore::ChannelEvent(CHANNEL_EVENT_TYPE_META_SET_TEMPO, 0, tickPos, 0, 60000000.0f / bpm, -1));

    _sequenceEditor->addEvent(_sequence->data.tracks[0], e, true, true);

    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::addPitchBendEvent(PianoRollEventsView* sender, unsigned int tickPos, int value) {
    _pianoRollView->mainView()->pianoRollEventsView()->clearEventSelection();

    tickPos = _pianoRollView->quantizeNewEventsButton()->state() ? quantizedTickPos(tickPos) : tickPos;

    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();
    auto e = std::shared_ptr<MelobaseCore::ChannelEvent>(
        new MelobaseCore::ChannelEvent(CHANNEL_EVENT_TYPE_PITCH_BEND, _currentChannel, tickPos, 0, value + 8192, 2));

    // Ensure that the channel is visible
    if (!_pianoRollView->visibleChannelButtons()[e->channel()]->state())
        _pianoRollView->visibleChannelButtons()[e->channel()]->setState(true);

    _sequenceEditor->addEvent(_sequence->data.tracks[_trackIndex], e, true, true);

    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::addModulationEvent(PianoRollEventsView* sender, unsigned int tickPos, int value) {
    _pianoRollView->mainView()->pianoRollEventsView()->clearEventSelection();

    tickPos = _pianoRollView->quantizeNewEventsButton()->state() ? quantizedTickPos(tickPos) : tickPos;

    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();
    auto e = std::shared_ptr<MelobaseCore::ChannelEvent>(
        new MelobaseCore::ChannelEvent(CHANNEL_EVENT_TYPE_MODULATION, _currentChannel, tickPos, 0, value, -1));

    // Ensure that the channel is visible
    if (!_pianoRollView->visibleChannelButtons()[e->channel()]->state())
        _pianoRollView->visibleChannelButtons()[e->channel()]->setState(true);

    _sequenceEditor->addEvent(_sequence->data.tracks[_trackIndex], e, true, true);

    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::addMixerLevelEvent(PianoRollEventsView* sender, unsigned int tickPos, int value) {
    _pianoRollView->mainView()->pianoRollEventsView()->clearEventSelection();

    tickPos = _pianoRollView->quantizeNewEventsButton()->state() ? quantizedTickPos(tickPos) : tickPos;

    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();
    auto e = std::shared_ptr<MelobaseCore::ChannelEvent>(
        new MelobaseCore::ChannelEvent(CHANNEL_EVENT_TYPE_MIXER_LEVEL_CHANGE, _currentChannel, tickPos, 0, value, 0));

    // Ensure that the channel is visible
    if (!_pianoRollView->visibleChannelButtons()[e->channel()]->state())
        _pianoRollView->visibleChannelButtons()[e->channel()]->setState(true);

    _sequenceEditor->addEvent(_sequence->data.tracks[_trackIndex], e, true, true);

    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::addMixerBalanceEvent(PianoRollEventsView* sender, unsigned int tickPos, int value) {
    _pianoRollView->mainView()->pianoRollEventsView()->clearEventSelection();

    tickPos = _pianoRollView->quantizeNewEventsButton()->state() ? quantizedTickPos(tickPos) : tickPos;

    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();
    auto e = std::shared_ptr<MelobaseCore::ChannelEvent>(
        new MelobaseCore::ChannelEvent(CHANNEL_EVENT_TYPE_MIXER_BALANCE_CHANGE, _currentChannel, tickPos, 0, value, 0));

    // Ensure that the channel is visible
    if (!_pianoRollView->visibleChannelButtons()[e->channel()]->state())
        _pianoRollView->visibleChannelButtons()[e->channel()]->setState(true);

    _sequenceEditor->addEvent(_sequence->data.tracks[_trackIndex], e, true, true);

    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::addTimeSignatureEvent(PianoRollEventsView* sender, unsigned int tickPos) {
    _pianoRollView->mainView()->pianoRollEventsView()->clearEventSelection();

    tickPos = _pianoRollView->quantizeNewEventsButton()->state() ? quantizedTickPos(tickPos) : tickPos;

    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();

    auto e = std::shared_ptr<MelobaseCore::ChannelEvent>(
        new MelobaseCore::ChannelEvent(CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE, 0, tickPos, 0, 4, 4));

    _sequenceEditor->addEvent(_sequence->data.tracks[0], e, true, true);

    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::addControlChangeEvent(PianoRollEventsView* sender, unsigned int tickPos, int control,
                                                    int value) {
    _pianoRollView->mainView()->pianoRollEventsView()->clearEventSelection();

    tickPos = _pianoRollView->quantizeNewEventsButton()->state() ? quantizedTickPos(tickPos) : tickPos;

    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();
    auto e = std::shared_ptr<MelobaseCore::ChannelEvent>(
        new MelobaseCore::ChannelEvent(CHANNEL_EVENT_TYPE_CONTROL_CHANGE, _currentChannel, tickPos, 0, control, value));

    // Ensure that the channel is visible
    if (!_pianoRollView->visibleChannelButtons()[e->channel()]->state())
        _pianoRollView->visibleChannelButtons()[e->channel()]->setState(true);

    _sequenceEditor->addEvent(_sequence->data.tracks[_trackIndex], e, true, true);

    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::addKeyAftertouchEvent(PianoRollEventsView* sender, unsigned int tickPos, int pitch,
                                                    int value) {
    _pianoRollView->mainView()->pianoRollEventsView()->clearEventSelection();

    tickPos = _pianoRollView->quantizeNewEventsButton()->state() ? quantizedTickPos(tickPos) : tickPos;

    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();
    auto e = std::shared_ptr<MelobaseCore::ChannelEvent>(
        new MelobaseCore::ChannelEvent(CHANNEL_EVENT_TYPE_KEY_AFTERTOUCH, _currentChannel, tickPos, 0, pitch, value));

    // Ensure that the channel is visible
    if (!_pianoRollView->visibleChannelButtons()[e->channel()]->state())
        _pianoRollView->visibleChannelButtons()[e->channel()]->setState(true);

    _sequenceEditor->addEvent(_sequence->data.tracks[_trackIndex], e, true, true);

    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::addChannelAftertouchEvent(PianoRollEventsView* sender, unsigned int tickPos, int value) {
    _pianoRollView->mainView()->pianoRollEventsView()->clearEventSelection();

    tickPos = _pianoRollView->quantizeNewEventsButton()->state() ? quantizedTickPos(tickPos) : tickPos;

    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();
    auto e = std::shared_ptr<MelobaseCore::ChannelEvent>(
        new MelobaseCore::ChannelEvent(CHANNEL_EVENT_TYPE_CHANNEL_AFTERTOUCH, _currentChannel, tickPos, 0, value));

    // Ensure that the channel is visible
    if (!_pianoRollView->visibleChannelButtons()[e->channel()]->state())
        _pianoRollView->visibleChannelButtons()[e->channel()]->setState(true);

    _sequenceEditor->addEvent(_sequence->data.tracks[_trackIndex], e, true, true);

    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::addSysexEvent(PianoRollEventsView* sender, unsigned int tickPos) {
    _pianoRollView->mainView()->pianoRollEventsView()->clearEventSelection();

    tickPos = _pianoRollView->quantizeNewEventsButton()->state() ? quantizedTickPos(tickPos) : tickPos;

    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();
    auto e = std::shared_ptr<MelobaseCore::ChannelEvent>(new MelobaseCore::ChannelEvent(
        CHANNEL_EVENT_TYPE_SYSTEM_EXCLUSIVE, _currentChannel, tickPos, 0, 0, 0, 0, {0xF7}));

    // Ensure that the channel is visible
    if (!_pianoRollView->visibleChannelButtons()[e->channel()]->state())
        _pianoRollView->visibleChannelButtons()[e->channel()]->setState(true);

    _sequenceEditor->addEvent(_sequence->data.tracks[_trackIndex], e, true, true);

    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);

    // Select the event and start the data edition
    _pianoRollView->mainView()->pianoRollControllerEventsView()->selectEvents({e});
    _pianoRollView->visiblePropertiesPaneButton()->setState(true);
    _pianoRollView->pianoRollUtilitiesView()->viewSelectionSegmentedControl()->setSelectedSegment(0);
    _pianoRollView->updateResponderChain();
    _pianoRollView->pianoRollUtilitiesView()->pianoRollPropertiesView()->sysexDataTextField()->startEdition();
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::addMetaEvent(PianoRollEventsView* sender, unsigned int tickPos, int type) {
    _pianoRollView->mainView()->pianoRollEventsView()->clearEventSelection();

    tickPos = _pianoRollView->quantizeNewEventsButton()->state() ? quantizedTickPos(tickPos) : tickPos;

    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();
    auto e = std::shared_ptr<MelobaseCore::ChannelEvent>(
        new MelobaseCore::ChannelEvent(CHANNEL_EVENT_TYPE_META_GENERIC, _currentChannel, tickPos, 0, type, 0));

    // Ensure that the channel is visible
    if (!_pianoRollView->visibleChannelButtons()[e->channel()]->state())
        _pianoRollView->visibleChannelButtons()[e->channel()]->setState(true);

    _sequenceEditor->addEvent(_sequence->data.tracks[_trackIndex], e, true, true);

    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);

    // Select the event and start the data edition
    _pianoRollView->mainView()->pianoRollControllerEventsView()->selectEvents({e});
    _pianoRollView->visiblePropertiesPaneButton()->setState(true);
    _pianoRollView->pianoRollUtilitiesView()->viewSelectionSegmentedControl()->setSelectedSegment(0);
    _pianoRollView->updateResponderChain();
    _pianoRollView->pianoRollUtilitiesView()->pianoRollPropertiesView()->metaDataTextField()->startEdition();
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::didSelectEvents(PianoRollEventsView* sender, bool areCombined, bool isAddingNote) {
    // Reset the selection region on both event views
    _pianoRollView->mainView()->pianoRollEventsView()->resetSelectionRegion();
    _pianoRollView->mainView()->pianoRollControllerEventsView()->resetSelectionRegion();
    _pianoRollView->mainView()->pianoRollMetaEventView()->resetSelectionRegion();

    if (!_isSelectingFromEventsList && !areCombined) {
        // Clear the selected events on the other events view
        if (sender == _pianoRollView->mainView()->pianoRollEventsView().get()) {
            _pianoRollView->mainView()->pianoRollControllerEventsView()->clearEventSelection(false);
            _pianoRollView->mainView()->pianoRollMetaEventView()->clearEventSelection(false);
        } else if (sender == _pianoRollView->mainView()->pianoRollControllerEventsView().get()) {
            _pianoRollView->mainView()->pianoRollEventsView()->clearEventSelection(false);
            _pianoRollView->mainView()->pianoRollMetaEventView()->clearEventSelection(false);
        } else {
            // From meta view
            _pianoRollView->mainView()->pianoRollEventsView()->clearEventSelection(false);
            _pianoRollView->mainView()->pianoRollControllerEventsView()->clearEventSelection(false);
        }
    }

    std::vector<std::shared_ptr<MelobaseCore::Event>> selectedEvents = selectedEventsWithAssociates();

    // Set the selected events to the properties view
    bool isMultiChannel = (_sequence && _trackIndex >= 0)
                              ? (_sequence->data.tracks[_trackIndex]->channel == SEQUENCE_TRACK_MULTI_CHANNEL)
                              : true;
    _pianoRollPropertiesViewController->setEvents(selectedEventsWithAssociates(), isMultiChannel);

    // Set the selected events for the event list view
    if (!_isSelectingFromEventsList && !isAddingNote)
        _pianoRollView->pianoRollUtilitiesView()->pianoRollEventsListView()->selectEvents(selectedEvents, false);

    // Update the quantize button state
    _pianoRollView->quantizeButton()->setIsEnabled(selectedEvents.size() > 0);

    if (_didSelectEventsFn) _didSelectEventsFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::setSelectionState(PianoRollEventsView* sender, bool state) {
    // Go to selection
    if (state) {
        _oldEditionSelectedSegment = _pianoRollView->editionSegmentedControl()->selectedSegment();
        _pianoRollView->editionSegmentedControl()->setSelectedSegment(1);
    } else {
        auto selectedSegment = (_oldEditionSelectedSegment == EDITION_MODE_MOVE_SEGMENT ||
                                _oldEditionSelectedSegment == EDITION_MODE_RESIZE_SEGMENT)
                                   ? _oldEditionSelectedSegment
                                   : EDITION_MODE_CURSOR_SEGMENT;
        _pianoRollView->editionSegmentedControl()->setSelectedSegment(selectedSegment);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::studioDidPlayNote(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick,
                                                int channel, int pitch, Float32 velocity) {
    if (_isKeyboardUpdated) {
        _pianoRollView->mainView()->keyboard()->setKeyPressedState(channel, pitch, true);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::studioDidReleaseNote(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick,
                                                   int channel, int pitch) {
    if (_isKeyboardUpdated) _pianoRollView->mainView()->keyboard()->setKeyPressedState(channel, pitch, false);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::studioDidSetInstrument(MDStudio::Studio* studio, int source, double timestamp,
                                                     UInt32 tick, int channel, int instrument) {
    if (channel == _currentChannel)
        _pianoRollView->mainView()->keyboard()->setIsDrumKit(instrument == STUDIO_INSTRUMENT_GM_STANDARD_DRUM_KIT);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::pianoRollEventsScrollViewPosChanged(MDStudio::ScrollView* sender, MDStudio::Point pos) {
    // Set the horizontal offset for the piano header and the controller events view
    MDStudio::Point offset = _pianoRollView->mainView()->pianoRollHeaderView()->offset();
    offset.x = pos.x;
    _pianoRollView->mainView()->pianoRollHeaderView()->setOffset(offset);
    _pianoRollView->mainView()->pianoRollControllerEventsView()->setOffset(offset);
    _pianoRollView->mainView()->pianoRollMetaEventView()->setOffset(offset);

    // Set the vertical offset for the keyboard
    offset = _pianoRollView->mainView()->keyboard()->offset();
    offset.y = pos.y;
    _pianoRollView->mainView()->keyboard()->setOffset(offset);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::scrollToVerticalCenter() {
    std::shared_ptr<MDStudio::ScrollView> pianoRollScrollView = _pianoRollView->mainView()->pianoRollEventsScrollView();
    std::shared_ptr<MDStudio::View> contentView = pianoRollScrollView->contentView();

    float midContentY = midRectY(contentView->bounds());
    float height = pianoRollScrollView->posMaxV() - pianoRollScrollView->posMinV();

    pianoRollScrollView->setPos(MDStudio::makePoint(pianoRollScrollView->posMinH(), midContentY - height / 2.0f));
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::updatePianoRoll() {
    size_t nbTracks = _sequence ? _sequence->data.tracks.size() : 0;
    _totalTickCounts.resize(nbTracks);
    _eotTickCounts.resize(nbTracks);
    _timeDivision = 480;

    if (_trackIndex >= 0) {
        // Set the current channel based on the track if not a multi-channel track
        if (_sequence && (_sequence->data.tracks.at(_trackIndex)->channel != SEQUENCE_TRACK_MULTI_CHANNEL)) {
            setCurrentChannel(_sequence->data.tracks[_trackIndex]->channel, false);
        } else {
            // Use current multi-channel selection
            setCurrentChannel(_currentMultiChannel, true);
        }

        if (_sequence) {
            _pianoRollView->mainView()->pianoRollEventsView()->setTrackChannel(
                _sequence->data.tracks[_trackIndex]->channel);
            _pianoRollView->mainView()->pianoRollControllerEventsView()->setTrackChannel(
                _sequence->data.tracks[_trackIndex]->channel);
            _pianoRollView->mainView()->pianoRollMetaEventView()->setTrackChannel(
                _sequence->data.tracks[_trackIndex]->channel);
        }

        for (int trackIndex = 0; trackIndex < nbTracks; ++trackIndex) {
            bool isEOTFound = false;

            _totalTickCounts[trackIndex] = 0;
            _eotTickCounts[trackIndex] = 0;

            if (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusRecording) {
                _totalTickCounts[trackIndex] = _studioController->studio()->metronome()->tick();
            } else {
                if (_sequence) {
                    _timeDivision = roundf(60.0f / (_sequence->data.tickPeriod * 125.0f));
                    for (auto event : _sequence->data.tracks[trackIndex]->clips[0]->events) {
                        auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(event);
                        _totalTickCounts[trackIndex] =
                            (channelEvent->tickCount() + channelEvent->length() > _totalTickCounts[trackIndex])
                                ? (channelEvent->tickCount() + channelEvent->length())
                                : _totalTickCounts[trackIndex];
                        if (channelEvent->type() == CHANNEL_EVENT_TYPE_META_END_OF_TRACK) {
                            _eotTickCounts[trackIndex] = channelEvent->tickCount();
                            isEOTFound = true;
                        }
                    }
                }
            }

            if (!isEOTFound) _eotTickCounts[trackIndex] = _totalTickCounts[trackIndex];
        }
    }  // If track index >= 0

    unsigned int totalTickCount = 0, eotTickCount = 0;

    if (nbTracks > 0 && _trackIndex >= 0) {
        totalTickCount = _totalTickCounts[_trackIndex];
        eotTickCount = _eotTickCounts[_trackIndex];
    }

    setZoom(_pianoRollView->zoomSlider()->pos());

    double eventTickWidth = _pianoRollView->mainView()->pianoRollEventsView()->eventTickWidth();
    float eventHeight = _pianoRollView->mainView()->pianoRollEventsView()->eventHeight();
    float contentWidth = eventTickWidth * totalTickCount;
    if (contentWidth <
        _pianoRollView->mainView()->pianoRollEventsScrollView()->rect().size.width - SCROLL_VIEW_SCROLL_BAR_THICKNESS)
        contentWidth = _pianoRollView->mainView()->pianoRollEventsScrollView()->rect().size.width -
                       SCROLL_VIEW_SCROLL_BAR_THICKNESS;
    _pianoRollView->mainView()->pianoRollEventsScrollView()->setContentSize(
        MDStudio::makeSize(contentWidth + pianoRollCursorWidth, 96 * eventHeight));
    _pianoRollView->setDirty();
    _pianoRollView->mainView()->pianoRollControllerEventsView()->setDirty();
    _pianoRollView->mainView()->pianoRollMetaEventView()->setDirty();
    _pianoRollView->mainView()->pianoRollHeaderView()->setDirty();

    // if ((_studioController->status() != MelobaseCore::StudioController::StudioControllerStatusRecording) &&
    // (_studioController->studio()->metronome()->tick() > eotTickCount))
    //    _pianoRollView->mainView()->pianoRollEventsView()->setCursorTickPos(eotTickCount);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::updateProperties() {
    assert(!_sequence || (_trackIndex < 0) || (_trackIndex < _sequence->data.tracks.size()));
    bool isMultiChannel = (_sequence && _trackIndex >= 0)
                              ? (_sequence->data.tracks[_trackIndex]->channel == SEQUENCE_TRACK_MULTI_CHANNEL)
                              : true;
    _pianoRollPropertiesViewController->setEvents(selectedEventsWithAssociates(), isMultiChannel);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::updateEventList() {
    if (_sequence && _trackIndex >= 0) _sequenceEditor->sortEvents(_sequence->data.tracks.at(_trackIndex));
    _pianoRollView->pianoRollUtilitiesView()->pianoRollEventsListView()->reload();
    _pianoRollView->pianoRollUtilitiesView()->pianoRollEventsListView()->selectEvents(selectedEventsWithAssociates(),
                                                                                      false);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::updateFlagControls() {
    if (!_sequence || _trackIndex < 0 ||
        _studioController->status() == MelobaseCore::StudioController::StudioControllerStatusRecording ||
        _studioController->status() == MelobaseCore::StudioController::StudioControllerStatusPlaying) {
        _pianoRollView->addFlagButton()->setIsEnabled(false);
        _pianoRollView->removeFlagButton()->setIsEnabled(false);
        _pianoRollView->removeAllFlagsButton()->setIsEnabled(false);
        _pianoRollView->goToPreviousFlagButton()->setIsEnabled(false);
        _pianoRollView->goToNextFlagButton()->setIsEnabled(false);
        return;
    }

    auto currentTick = _studioController->studio()->metronome()->tick();
    bool anyFlagAtCurrentTick =
        std::any_of(_sequence->annotations.begin(), _sequence->annotations.end(),
                    [currentTick](std::shared_ptr<MelobaseCore::SequenceAnnotation> annotation) {
                        return annotation->tickCount == currentTick;
                    });

    _pianoRollView->addFlagButton()->setIsEnabled(!anyFlagAtCurrentTick);
    _pianoRollView->removeFlagButton()->setIsEnabled(anyFlagAtCurrentTick);
    _pianoRollView->removeAllFlagsButton()->setIsEnabled(!_sequence->annotations.empty());
    _pianoRollView->goToPreviousFlagButton()->setIsEnabled(goToPreviousAnnotation(true));
    _pianoRollView->goToNextFlagButton()->setIsEnabled(goToNextAnnotation(true));
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::setZoom(float zoom) {
    const float base = 100.0f;
    float expZoom = (powf(base, zoom) - 1.0f) / (base - 1.0f);

    _measureWidth = 50.0f + expZoom * 1000.0f;

    unsigned long nbTicksPerMeasure = _timeDivision * 4;
    double eventTickWidth = (double)_measureWidth / (double)nbTicksPerMeasure;

    _pianoRollView->mainView()->pianoRollHeaderView()->setEventTickWidth(eventTickWidth);
    _pianoRollView->mainView()->pianoRollEventsView()->setEventTickWidth(eventTickWidth);
    _pianoRollView->mainView()->pianoRollControllerEventsView()->setEventTickWidth(eventTickWidth);
    _pianoRollView->mainView()->pianoRollMetaEventView()->setEventTickWidth(eventTickWidth);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::setSequence(std::shared_ptr<MelobaseCore::Sequence> sequence) {
    if (sequence == _sequence) return;

    _pianoRollView->mainView()->pianoRollEventsView()->clearEventSelection();
    _sequence = sequence;

    setZoom(_pianoRollView->zoomSlider()->pos());

    updatePianoRoll();
    updateEventList();
    updateFlagControls();
    scrollToVerticalCenter();
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::studioControllerDidGoToBeginning(MelobaseCore::StudioController* sender) {
    _pianoRollView->mainView()->pianoRollEventsView()->setCursorTickPos(
        _studioController->studio()->metronome()->tick(), false);
    _pianoRollView->mainView()->pianoRollControllerEventsView()->setCursorTickPos(
        _studioController->studio()->metronome()->tick(), false);
    _pianoRollView->mainView()->pianoRollMetaEventView()->setCursorTickPos(
        _studioController->studio()->metronome()->tick(), false);
    _pianoRollView->mainView()->scrollToVisibleCursor();

    if (_didSetCursorTickPosFn) _didSetCursorTickPosFn(this, _studioController->studio()->metronome()->tick());
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::selectAllEvents() {
    _pianoRollView->mainView()->pianoRollEventsView()->selectAllEvents();
}

// ---------------------------------------------------------------------------------------------------------------------
bool PianoRollViewController::deleteSelectedEvents() {
    if (!_pianoRollView->mainView()->pianoRollMetaEventView()->hasFocus() &&
        !_pianoRollView->mainView()->pianoRollEventsView()->hasFocus() &&
        !_pianoRollView->mainView()->pianoRollControllerEventsView()->hasFocus() &&
        !_pianoRollView->pianoRollUtilitiesView()->pianoRollEventsListView()->tableView()->hasFocus())
        return false;

    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();

    std::vector<std::shared_ptr<MelobaseCore::Event>> selectedEvents = selectedEventsWithAssociates();
    for (auto it = selectedEvents.begin(); it != selectedEvents.end(); ++it) {
        auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(*it);
        int trackIndex = (channelEvent->type() == CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE ||
                          channelEvent->type() == CHANNEL_EVENT_TYPE_META_SET_TEMPO)
                             ? 0
                             : _trackIndex;
        _sequenceEditor->removeEvent(_sequence->data.tracks[trackIndex], *it, it == selectedEvents.begin(),
                                     it == selectedEvents.end() - 1);
    }

    _sequenceEditor->undoManager()->endGroup();

    _pianoRollView->mainView()->pianoRollMetaEventView()->clearEventSelection();
    _pianoRollView->mainView()->pianoRollEventsView()->clearEventSelection();
    _pianoRollView->mainView()->pianoRollControllerEventsView()->clearEventSelection();

    if (_didModifySequenceFn) _didModifySequenceFn(this);

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<MelobaseCore::Event>> PianoRollViewController::selectedEventsWithAssociates() {
    std::vector<std::shared_ptr<MelobaseCore::Event>> selectedEvents =
        _pianoRollView->mainView()->pianoRollEventsView()->selectedEvents();
    std::vector<std::shared_ptr<MelobaseCore::Event>> selectedControllerEvents =
        _pianoRollView->mainView()->pianoRollControllerEventsView()->selectedEvents();
    std::vector<std::shared_ptr<MelobaseCore::Event>> selectedEndOfTrackEvents =
        _pianoRollView->mainView()->pianoRollMetaEventView()->selectedEvents();
    selectedEvents.insert(selectedEvents.end(), selectedControllerEvents.begin(), selectedControllerEvents.end());
    selectedEvents.insert(selectedEvents.end(), selectedEndOfTrackEvents.begin(), selectedEndOfTrackEvents.end());
    return selectedEvents;
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::quantizeSelectedEvents() {
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();

    UInt32 resTickCount = _timeDivision;
    double eventTickWidth = _pianoRollView->mainView()->pianoRollEventsView()->eventTickWidth();

    if (eventTickWidth > 0.2) {
        resTickCount = _timeDivision / 8;
    } else if (eventTickWidth > 0.07) {
        resTickCount = _timeDivision / 4;
    } else if (eventTickWidth > 0.04) {
        resTickCount = _timeDivision / 2;
    }

    _sequenceEditor->quantizeEvents(_sequence->data.tracks[_trackIndex], selectedEventsWithAssociates(), resTickCount);

    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
bool PianoRollViewController::copySelectedEvents() {
    if (!_pianoRollView->mainView()->pianoRollEventsView()->hasFocus() &&
        !_pianoRollView->mainView()->pianoRollControllerEventsView()->hasFocus() &&
        !_pianoRollView->mainView()->pianoRollMetaEventView()->hasFocus() &&
        !_pianoRollView->pianoRollUtilitiesView()->pianoRollEventsListView()->tableView()->hasFocus())
        return false;

    std::vector<std::shared_ptr<MelobaseCore::Event>> allSelectedEvents = selectedEventsWithAssociates();
    std::vector<std::shared_ptr<MelobaseCore::Event>> events;

    double tickPeriodFactor = _sequence->data.tickPeriod / 0.001;

    for (auto event : allSelectedEvents) {
        auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(event);
        events.push_back(std::make_shared<MelobaseCore::ChannelEvent>(
            channelEvent->type(), channelEvent->channel(), channelEvent->tickCount() * tickPeriodFactor,
            channelEvent->length() * tickPeriodFactor, channelEvent->param1(), channelEvent->param2(),
            channelEvent->param3(), channelEvent->data()));
    }

    MDStudio::Pasteboard::sharedInstance()->setContent(events);

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool PianoRollViewController::cutSelectedEvents() {
    if (!_pianoRollView->mainView()->pianoRollEventsView()->hasFocus() &&
        !_pianoRollView->mainView()->pianoRollControllerEventsView()->hasFocus() &&
        !_pianoRollView->mainView()->pianoRollMetaEventView()->hasFocus() &&
        !_pianoRollView->pianoRollUtilitiesView()->pianoRollEventsListView()->tableView()->hasFocus())
        return false;

    copySelectedEvents();
    deleteSelectedEvents();

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool PianoRollViewController::pasteEvents() {
    if (!_pianoRollView->mainView()->pianoRollEventsView()->hasFocus() &&
        !_pianoRollView->mainView()->pianoRollControllerEventsView()->hasFocus() &&
        !_pianoRollView->mainView()->pianoRollMetaEventView()->hasFocus())
        return false;

    MDStudio::Pasteboard* pasteboard = MDStudio::Pasteboard::sharedInstance();
    if (pasteboard->isContentAvailable()) {
        if (pasteboard->content().is<std::vector<std::shared_ptr<MelobaseCore::Event>>>()) {
            std::vector<std::shared_ptr<MelobaseCore::Event>> events =
                pasteboard->content().as<std::vector<std::shared_ptr<MelobaseCore::Event>>>();

            if (events.size() > 0) {
                // Sort the events based on their absolute ticks
                std::sort(events.begin(), events.end(),
                          [](std::shared_ptr<MelobaseCore::Event> a, std::shared_ptr<MelobaseCore::Event> b) {
                              auto channelEventA = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(a);
                              auto channelEventB = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(b);
                              return channelEventA->tickCount() < channelEventB->tickCount();
                          });

                int channel = -1;
                bool isMoreThanOneChannel = false;

                double tickPeriodFactor = 0.001 / _sequence->data.tickPeriod;

                // Adjust the tick counts and get the channel for the non-meta events
                auto firstChannelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(events[0]);
                UInt32 adjustTickCount = firstChannelEvent->tickCount() * tickPeriodFactor;
                for (auto& event : events) {
                    auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(event);
                    if (!getIsMetaEvent(channelEvent)) {
                        if (channel == -1) {
                            channel = channelEvent->channel();
                        } else {
                            if (!isMoreThanOneChannel && (channelEvent->channel() != channel))
                                isMoreThanOneChannel = true;
                        }
                    }
                    channelEvent->setTickCount(channelEvent->tickCount() +
                                               _pianoRollView->mainView()->pianoRollEventsView()->cursorTickPos() -
                                               adjustTickCount);
                }

                _pianoRollView->mainView()->pianoRollEventsView()->clearEventSelection();

                std::vector<std::shared_ptr<MelobaseCore::Event>> eventsToAdd, eventsToAddFirstTrack;
                for (auto event : events) {
                    auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(event);
                    auto e = std::make_shared<MelobaseCore::ChannelEvent>(
                        channelEvent->type(),
                        (getIsMetaEvent(channelEvent) || isMoreThanOneChannel) ? channelEvent->channel()
                                                                               : _currentChannel,
                        channelEvent->tickCount() * tickPeriodFactor, channelEvent->length() * tickPeriodFactor,
                        channelEvent->param1(), channelEvent->param2(), channelEvent->param3(), channelEvent->data());

                    if (e->type() == CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE ||
                        e->type() == CHANNEL_EVENT_TYPE_META_SET_TEMPO) {
                        eventsToAddFirstTrack.push_back(e);
                    } else {
                        eventsToAdd.push_back(e);
                    }

                    // Ensure that the channel is visible
                    if (!getIsMetaEvent(e)) {
                        if (!_pianoRollView->visibleChannelButtons()[e->channel()]->state())
                            _pianoRollView->visibleChannelButtons()[e->channel()]->setState(true);
                    }
                }

                // Check if all the events could be added
                if (!_sequenceEditor->canAddEvents(_sequence->data.tracks[_trackIndex], eventsToAdd)) {
                    MDStudio::Platform::sharedInstance()->beep();
                    return true;
                }

                if (!_sequenceEditor->canAddEvents(_sequence->data.tracks[0], eventsToAddFirstTrack)) {
                    MDStudio::Platform::sharedInstance()->beep();
                    return true;
                }

                if (_willModifySequenceFn) _willModifySequenceFn(this);

                // Add the events
                _sequenceEditor->undoManager()->beginGroup();

                for (auto it = eventsToAdd.begin(); it != eventsToAdd.end(); ++it) {
                    _sequenceEditor->addEvent(_sequence->data.tracks[_trackIndex], *it, it == eventsToAdd.begin(),
                                              it == eventsToAdd.end() - 1);
                }
                for (auto it = eventsToAddFirstTrack.begin(); it != eventsToAddFirstTrack.end(); ++it) {
                    _sequenceEditor->addEvent(_sequence->data.tracks[0], *it, it == eventsToAddFirstTrack.begin(),
                                              it == eventsToAddFirstTrack.end() - 1);
                }

                _sequenceEditor->undoManager()->endGroup();

                // Select pasted events
                std::vector<std::shared_ptr<MelobaseCore::Event>> eventsToSelect{eventsToAdd};
                eventsToSelect.insert(std::end(eventsToSelect), std::begin(eventsToAddFirstTrack),
                                      std::end(eventsToAddFirstTrack));
                _pianoRollView->pianoRollUtilitiesView()->pianoRollEventsListView()->selectEvents(eventsToSelect);

                if (_didModifySequenceFn) _didModifySequenceFn(this);

            }  // If we have at least one event
        }
    }
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::editionModeSegmentedControlDidSelectSegment(MDStudio::SegmentedControl* sender,
                                                                          int selectedSegment) {
    const static PianoRollEventsView::ModeEnum modes[] = {
        PianoRollEventsView::ArrowMode, PianoRollEventsView::SelectionMode, PianoRollEventsView::DrawingMode,
        PianoRollEventsView::MoveMode, PianoRollEventsView::ResizeMode};
    auto mode = modes[selectedSegment];

    _pianoRollView->mainView()->pianoRollEventsView()->setMode(mode);
    _pianoRollView->mainView()->pianoRollControllerEventsView()->setMode(mode);
    _pianoRollView->mainView()->pianoRollMetaEventView()->setMode(mode);

    // bool isDrawingMode = mode == PianoRollEventsView::DrawingMode;
    //_pianoRollView->currentNoteSegmentedControl()->setIsVisible(isDrawingMode);
    //_pianoRollView->quantizeNewEventsButton()->setIsVisible(isDrawingMode);

    _pianoRollView->editionView()->setDirty();
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::zoneSelectionSegmentedControlDidSelectSegment(MDStudio::SegmentedControl* sender,
                                                                            int selectedSegment) {
    if (_didSelectZoneFn) _didSelectZoneFn(this, selectedSegment);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::updateControlChangeMetaComboBoxVisibility() {
    int selectedSegment = _pianoRollView->mainView()->controllerSegmentedControlV()->selectedSegment();

    PianoRollEventsView::ControllerEventsModeEnum mode = modes[selectedSegment];

    if (_pianoRollView->mainView()->pianoRollControllerEventsView()->isVisible()) {
        if (mode == PianoRollEventsView::ControlChangeControllerEventsMode) {
            _pianoRollView->mainView()->controlChangeComboBox()->setIsVisible(true);
            _pianoRollView->mainView()->metaTypeComboBox()->setIsVisible(false);
            updateControlChange();
        } else if (mode == PianoRollEventsView::MetaControllerEventsMode) {
            _pianoRollView->mainView()->controlChangeComboBox()->setIsVisible(false);
            _pianoRollView->mainView()->metaTypeComboBox()->setIsVisible(true);
            updateMetaType();
        } else {
            _pianoRollView->mainView()->controlChangeComboBox()->setIsVisible(false);
            _pianoRollView->mainView()->metaTypeComboBox()->setIsVisible(false);
            _pianoRollView->mainView()->pianoRollControllerEventsView()->setControllerEventsMode(mode);
        }
    } else {
        _pianoRollView->mainView()->controlChangeComboBox()->setIsVisible(false);
        _pianoRollView->mainView()->metaTypeComboBox()->setIsVisible(false);
        _pianoRollView->mainView()->pianoRollControllerEventsView()->setControllerEventsMode(mode);
    }

    _pianoRollView->mainView()->setFrame(_pianoRollView->mainView()->frame());
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::controllerSegmentedControlVDidSelectSegment(SegmentedControlV* sender,
                                                                          int selectedSegment) {
    updateControlChangeMetaComboBoxVisibility();
    updateRuler();
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::zoomSliderPosChanged(MDStudio::Slider* sender, float pos) {
    if (!_sequence) return;

    MDStudio::Point pt = _pianoRollView->mainView()->pianoRollEventsScrollView()->pos();
    pt.x += (_pianoRollView->mainView()->pianoRollEventsScrollView()->posMaxH() -
             _pianoRollView->mainView()->pianoRollEventsScrollView()->posMinH()) /
            2.0f;
    float relHMidPos = pt.x / _pianoRollView->mainView()->pianoRollEventsView()->bounds().size.width;

    updatePianoRoll();

    pt.x = relHMidPos * _pianoRollView->mainView()->pianoRollEventsView()->bounds().size.width;
    pt.x -= (_pianoRollView->mainView()->pianoRollEventsScrollView()->posMaxH() -
             _pianoRollView->mainView()->pianoRollEventsScrollView()->posMinH()) /
            2.0f;
    _pianoRollView->mainView()->pianoRollEventsScrollView()->setPos(pt);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::visibleChannelButtonStateDidChange(MDStudio::Button* sender, bool state,
                                                                 unsigned int modifierFlags) {
    // Find the sender channel
    int senderChannel = 0;
    for (; senderChannel < STUDIO_MAX_CHANNELS; ++senderChannel) {
        auto button = _pianoRollView->visibleChannelButtons()[senderChannel];
        if (sender == button.get()) break;
    }
    // Find the last visible channel for range selection
    int lastSelectedChannel = -1;
    for (int channel = senderChannel; channel >= 0; --channel) {
        bool state = _pianoRollView->mainView()->pianoRollEventsView()->visibleChannels()[channel];
        if (state) {
            lastSelectedChannel = channel;
            break;
        }
    }

    std::array<bool, STUDIO_MAX_CHANNELS> visibleChannels;
    for (int channel = 0; channel < STUDIO_MAX_CHANNELS; ++channel) {
        auto button = _pianoRollView->visibleChannelButtons()[channel];
        if (modifierFlags & MODIFIER_FLAG_SHIFT) {
            // Range selection
            visibleChannels[channel] =
                button->state() || ((channel >= lastSelectedChannel) && (channel <= senderChannel));
            button->setState(visibleChannels[channel], false);
        } else if (modifierFlags & MODIFIER_FLAG_COMMAND) {
            // Single selection
            visibleChannels[channel] = (sender == button.get());
            button->setState(visibleChannels[channel], false);
        } else {
            // Multiple selection
            visibleChannels[channel] = button->state();
        }
    }
    _pianoRollView->mainView()->pianoRollEventsView()->setVisibleChannels(visibleChannels);
    _pianoRollView->mainView()->pianoRollControllerEventsView()->setVisibleChannels(visibleChannels);
    _pianoRollView->mainView()->pianoRollMetaEventView()->setVisibleChannels(visibleChannels);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::allVisibleChannelButtonClicked(MDStudio::Button* sender) {
    bool areAllChannelsVisible = true;
    for (int channel = 0; channel < STUDIO_MAX_CHANNELS; ++channel)
        if (!_pianoRollView->visibleChannelButtons()[channel]->state()) areAllChannelsVisible = false;

    // If all the channels are selected, set all to false otherwise set all to true
    for (int channel = 0; channel < STUDIO_MAX_CHANNELS; ++channel) {
        auto button = _pianoRollView->visibleChannelButtons()[channel];
        button->setState(!areAllChannelsVisible, true, 0);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::visibleControllerPaneButtonStateDidChange(MDStudio::Button* sender, bool state) {
    MDStudio::Rect frameTop = _pianoRollView->mainView()->pianoRollEventsScrollView()->frame();
    MDStudio::Rect frameBottom = _pianoRollView->mainView()->pianoRollControllerEventsView()->frame();
    MDStudio::Rect frameKeyboard = _pianoRollView->mainView()->keyboard()->frame();

    if (state) {
        frameTop.origin.y += PIANO_ROLL_MAIN_VIEW_CONTROLLER_HEIGHT;
        frameTop.size.height -= PIANO_ROLL_MAIN_VIEW_CONTROLLER_HEIGHT;
        frameBottom.size.height += PIANO_ROLL_MAIN_VIEW_CONTROLLER_HEIGHT;
        frameKeyboard.origin.y += PIANO_ROLL_MAIN_VIEW_CONTROLLER_HEIGHT;
        frameKeyboard.size.height -= PIANO_ROLL_MAIN_VIEW_CONTROLLER_HEIGHT;
    } else {
        frameTop.origin.y -= PIANO_ROLL_MAIN_VIEW_CONTROLLER_HEIGHT;
        frameTop.size.height += PIANO_ROLL_MAIN_VIEW_CONTROLLER_HEIGHT;
        frameBottom.size.height -= PIANO_ROLL_MAIN_VIEW_CONTROLLER_HEIGHT;
        frameKeyboard.origin.y -= PIANO_ROLL_MAIN_VIEW_CONTROLLER_HEIGHT;
        frameKeyboard.size.height += PIANO_ROLL_MAIN_VIEW_CONTROLLER_HEIGHT;
    }

    _pianoRollView->mainView()->controllerSegmentedControlV()->setIsVisible(state);
    _pianoRollView->mainView()->pianoRollControllerEventsView()->setIsVisible(state);
    _pianoRollView->mainView()->controllerEventsRulerView()->setIsVisible(state);

    updateControlChangeMetaComboBoxVisibility();

    _pianoRollView->setDirty();

    if (_didSetPaneVisibilityFn) _didSetPaneVisibilityFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::visiblePropertiesPaneButtonStateDidChange(MDStudio::Button* sender, bool state) {
    _pianoRollView->horizSplitView()->setRightPaneVisibility(state);

    if (_didSetPaneVisibilityFn) _didSetPaneVisibilityFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::setIsEditionAvailable(bool isEditionAvailable) {
    if (isEditionAvailable) {
        _pianoRollView->editionSegmentedControl()->setIsEnabled(true);
    } else {
        _pianoRollView->editionSegmentedControl()->setSelectedSegment(0);
        _pianoRollView->editionSegmentedControl()->setIsEnabled(false);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::keyboardScroll(KeyboardV* sender, float deltaY) {
    MDStudio::Point pos = _pianoRollView->mainView()->pianoRollEventsScrollView()->pos();
    pos.y += deltaY;
    _pianoRollView->mainView()->pianoRollEventsScrollView()->setPos(pos);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::pianoRollPropertiesChannelDidChange(PianoRollPropertiesViewController* sender,
                                                                  int channel) {
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();
    _sequenceEditor->setChannelOfEvents(_sequence->data.tracks[_trackIndex], selectedEventsWithAssociates(), channel);
    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::pianoRollPropertiesVelocityDidChange(PianoRollPropertiesViewController* sender,
                                                                   int velocity) {
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();
    _sequenceEditor->setVelocityOfEvents(_sequence->data.tracks[_trackIndex], selectedEventsWithAssociates(), velocity);
    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::pianoRollPropertiesProgramDidChange(PianoRollPropertiesViewController* sender,
                                                                  int instrument) {
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();
    _sequenceEditor->setProgramOfEvents(_sequence->data.tracks[_trackIndex], selectedEventsWithAssociates(),
                                        instrument);
    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::pianoRollPropertiesTimeSignatureDidChange(PianoRollPropertiesViewController* sender,
                                                                        int numerator, int denominator) {
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();
    _sequenceEditor->setTimeSignatureOfEvents(_sequence->data.tracks[_trackIndex], selectedEventsWithAssociates(),
                                              numerator, denominator);
    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::pianoRollPropertiesSysexDataDidChange(PianoRollPropertiesViewController* sender,
                                                                    std::vector<UInt8> data) {
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();
    _sequenceEditor->setSysexDataOfEvents(_sequence->data.tracks[_trackIndex], selectedEventsWithAssociates(), data);
    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::pianoRollPropertiesMetaDataDidChange(PianoRollPropertiesViewController* sender,
                                                                   std::vector<UInt8> data) {
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();
    _sequenceEditor->setMetaDataOfEvents(_sequence->data.tracks[_trackIndex], selectedEventsWithAssociates(), data);
    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::pianoRollPropertiesPitchDidChange(PianoRollPropertiesViewController* sender, int pitch) {
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    _sequenceEditor->undoManager()->beginGroup();
    _sequenceEditor->setPitchOfEvents(_sequence->data.tracks[_trackIndex], selectedEventsWithAssociates(), pitch);
    _sequenceEditor->undoManager()->endGroup();

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::quantizeButtonClicked(MDStudio::Button* sender) { quantizeSelectedEvents(); }

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::showHideControllerEvents() {
    _pianoRollView->visibleControllerPaneButton()->setState(!_pianoRollView->visibleControllerPaneButton()->state());
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::showHideProperties() {
    _pianoRollView->visiblePropertiesPaneButton()->setState(!_pianoRollView->visiblePropertiesPaneButton()->state());
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::setIsKeyboardUpdated(bool isKeyboardUpdated) {
    _isKeyboardUpdated = isKeyboardUpdated;
    if (!_isKeyboardUpdated) {
        _pianoRollView->mainView()->keyboard()->resetKeyPressedStates();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::setCurrentChannel(int currentChannel, bool isMultiChannel) {
    _currentChannel = currentChannel;

    MDStudio::Color color = channelColors[_currentChannel];
    color.red *= 0.75f;
    color.green *= 0.75f;
    color.blue *= 0.75f;
    _pianoRollView->mainView()->keyboardChannelBoxView()->setBorderColor(color);
    _pianoRollView->mainView()->keyboardChannelBoxView()->setFillColor(color);

    _pianoRollView->mainView()->keyboard()->setCurrentChannel(currentChannel);
    _pianoRollView->mainView()->keyboard()->setIsDrumKit(_studioController->studio()->instrument(currentChannel) ==
                                                         STUDIO_INSTRUMENT_GM_STANDARD_DRUM_KIT);

    _pianoRollView->mainView()->pianoRollEventsView()->setHighlightChannel(currentChannel);

    // If this is not a multi-channel track, make sure that the channel is visible

    if (!isMultiChannel) {
        int index = 0;
        for (auto button : _pianoRollView->visibleChannelButtons()) {
            if (index == currentChannel) {
                if (!button->state()) button->setState(true);
            }
            ++index;
            button->setIsEnabled(false);
        }
        _pianoRollView->allVisibleChannelButton()->setIsEnabled(false);
        _pianoRollView->mainView()->zoneSelectionSegmentedControl()->setIsEnabled(false);
    } else {
        for (auto button : _pianoRollView->visibleChannelButtons()) button->setIsEnabled(true);
        _pianoRollView->allVisibleChannelButton()->setIsEnabled(true);
        _pianoRollView->mainView()->zoneSelectionSegmentedControl()->setIsEnabled(true);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::setCurrentMultiChannel(int currentMultiChannel) {
    _currentMultiChannel = currentMultiChannel;
    updatePianoRoll();
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::setTrackIndex(int trackIndex) {
    if (_trackIndex == trackIndex) return;

    _pianoRollView->mainView()->pianoRollEventsView()->clearEventSelection();

    _trackIndex = trackIndex;

    UInt8 trackChannel = _trackIndex >= 0 ? _sequence->data.tracks[_trackIndex]->channel : SEQUENCE_TRACK_MULTI_CHANNEL;

    _pianoRollView->mainView()->pianoRollHeaderView()->setTrackIndex(trackIndex);
    _pianoRollView->mainView()->pianoRollEventsView()->setTrackIndex(trackIndex);
    _pianoRollView->mainView()->pianoRollEventsView()->setTrackChannel(trackChannel);
    _pianoRollView->mainView()->pianoRollControllerEventsView()->setTrackIndex(trackIndex);
    _pianoRollView->mainView()->pianoRollControllerEventsView()->setTrackChannel(trackChannel);
    _pianoRollView->mainView()->pianoRollMetaEventView()->setTrackIndex(trackIndex);
    _pianoRollView->mainView()->pianoRollMetaEventView()->setTrackChannel(trackChannel);
    _pianoRollView->pianoRollUtilitiesView()->pianoRollEventsListView()->setTrackIndex(trackIndex);
    _sequenceEditor->setSequence(_sequence);
    updatePianoRoll();
    updateEventList();
    updateFlagControls();

    _pianoRollView->mainView()->pianoRollEventsView()->setCursorTickPos(
        _studioController->studio()->metronome()->tick(), false);
    _pianoRollView->mainView()->pianoRollControllerEventsView()->setCursorTickPos(
        _studioController->studio()->metronome()->tick(), false);
    _pianoRollView->mainView()->pianoRollMetaEventView()->setCursorTickPos(
        _studioController->studio()->metronome()->tick(), false);

    setIsEditionAvailable(_trackIndex >= 0 &&
                          ((_sequence->data.format == SEQUENCE_DATA_FORMAT_SINGLE_TRACK) || (_trackIndex > 0)));
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::setCursorTickPos(unsigned int tickPos) {
    _pianoRollView->mainView()->pianoRollEventsView()->setCursorTickPos(
        _studioController->studio()->metronome()->tick(), false);
    _pianoRollView->mainView()->pianoRollControllerEventsView()->setCursorTickPos(
        _studioController->studio()->metronome()->tick(), false);
    _pianoRollView->mainView()->pianoRollMetaEventView()->setCursorTickPos(
        _studioController->studio()->metronome()->tick(), false);
    if (!_pianoRollView->mainView()->pianoRollEventsScrollView()->isScrollingH())
        _pianoRollView->mainView()->scrollToCenteredCursor();
    updateFlagControls();
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::handleEventRepeat() {
    if (_pianoRollView->mainView()->pianoRollEventsView()->isCaptured()) {
        MDStudio::UIEvent lastEvent = _pianoRollView->responderChain()->lastEvent();
        if (lastEvent.type == MDStudio::MOUSE_MOVED_UIEVENT)
            _pianoRollView->mainView()->pianoRollEventsView()->handleEvent(&lastEvent);
    } else if (_pianoRollView->mainView()->pianoRollControllerEventsView()->isCaptured()) {
        MDStudio::UIEvent lastEvent = _pianoRollView->responderChain()->lastEvent();
        if (lastEvent.type == MDStudio::MOUSE_MOVED_UIEVENT)
            _pianoRollView->mainView()->pianoRollControllerEventsView()->handleEvent(&lastEvent);
    } else if (_pianoRollView->mainView()->pianoRollMetaEventView()->isCaptured()) {
        MDStudio::UIEvent lastEvent = _pianoRollView->responderChain()->lastEvent();
        if (lastEvent.type == MDStudio::MOUSE_MOVED_UIEVENT)
            _pianoRollView->mainView()->pianoRollMetaEventView()->handleEvent(&lastEvent);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int PianoRollViewController::controlChangeComboBoxNbRows(MDStudio::ComboBox* sender) { return 128; }

// ---------------------------------------------------------------------------------------------------------------------
unsigned int PianoRollViewController::metaTypeComboBoxNbRows(MDStudio::ComboBox* sender) { return 128; }

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MDStudio::View> PianoRollViewController::controlChangeComboBoxViewForRow(MDStudio::ComboBox* sender,
                                                                                         int row) {
    auto listItemView = make_shared<MDStudio::ListItemView>(
        "controlChangeListItemView", this,
        controllerMessages[row][0] == '*' ? controllerMessages[row] + 1 : controllerMessages[row]);
    listItemView->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    listItemView->setTextColor(controllerMessages[row][0] == '*' ? MDStudio::whiteColor : MDStudio::silverColor);
    return listItemView;
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MDStudio::View> PianoRollViewController::metaTypeComboBoxViewForRow(MDStudio::ComboBox* sender,
                                                                                    int row) {
    auto listItemView = make_shared<MDStudio::ListItemView>(
        "metaTypeListItemView", this, metaTypes[row][0] == '*' ? metaTypes[row] + 1 : metaTypes[row]);
    listItemView->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    listItemView->setTextColor(metaTypes[row][0] == '*' ? MDStudio::whiteColor : MDStudio::silverColor);
    return listItemView;
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::updateControlChange() {
    int row = _pianoRollView->mainView()->controlChangeComboBox()->selectedRow();
    if (row == 1) {
        // Modulation
        _pianoRollView->mainView()->pianoRollControllerEventsView()->setControllerEventsMode(
            PianoRollEventsView::ModulationControllerEventsMode);
    } else if (row == 7) {
        // Volume
        _pianoRollView->mainView()->pianoRollControllerEventsView()->setControllerEventsMode(
            PianoRollEventsView::MixerLevelControllerEventsMode);
    } else if (row == 10) {
        // Pan
        _pianoRollView->mainView()->pianoRollControllerEventsView()->setControllerEventsMode(
            PianoRollEventsView::MixerBalanceControllerEventsMode);
    } else if (row == 64) {
        // Sustain
        _pianoRollView->mainView()->pianoRollControllerEventsView()->setControllerEventsMode(
            PianoRollEventsView::SustainControllerEventsMode);
    } else {
        _pianoRollView->mainView()->pianoRollControllerEventsView()->setControllerEventsMode(
            PianoRollEventsView::ControlChangeControllerEventsMode);
        _pianoRollView->mainView()->pianoRollControllerEventsView()->setControlChange(row);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::updateMetaType() {
    int row = _pianoRollView->mainView()->metaTypeComboBox()->selectedRow();
    if (row == 81) {
        // Tempo
        _pianoRollView->mainView()->pianoRollControllerEventsView()->setControllerEventsMode(
            PianoRollEventsView::TempoControllerEventsMode);
    } else {
        _pianoRollView->mainView()->pianoRollControllerEventsView()->setControllerEventsMode(
            PianoRollEventsView::MetaControllerEventsMode);
        _pianoRollView->mainView()->pianoRollControllerEventsView()->setMetaType(row);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::updateRuler() {
    int selectedSegment = _pianoRollView->mainView()->controllerSegmentedControlV()->selectedSegment();
    assert(selectedSegment >= 0);
    PianoRollEventsView::ControllerEventsModeEnum mode = modes[selectedSegment];
    switch (mode) {
        case PianoRollEventsView::ProgramChangesControllerEventsMode:
        case PianoRollEventsView::SysexControllerEventsMode:
            _pianoRollView->mainView()->controllerEventsRulerView()->setRange(0, 0, 0);
            break;
        case PianoRollEventsView::TempoControllerEventsMode:
            _pianoRollView->mainView()->controllerEventsRulerView()->setRange(30, 10, 30);
            break;
        case PianoRollEventsView::PitchBendControllerEventsMode:
            _pianoRollView->mainView()->controllerEventsRulerView()->setRange(-8192, 9, 16384 / 8);
            break;
        case PianoRollEventsView::SustainControllerEventsMode:
        case PianoRollEventsView::ModulationControllerEventsMode:
        case PianoRollEventsView::MixerLevelControllerEventsMode:
        case PianoRollEventsView::MixerBalanceControllerEventsMode:
        case PianoRollEventsView::KeyAftertouchControllerEventsMode:
        case PianoRollEventsView::ChannelAftertouchControllerEventsMode:
        case PianoRollEventsView::ControlChangeControllerEventsMode:
            _pianoRollView->mainView()->controllerEventsRulerView()->setRange(0, 9, 128 / 8);
            break;
        case PianoRollEventsView::MetaControllerEventsMode:
            if (_pianoRollView->mainView()->pianoRollControllerEventsView()->controllerEventsMode() ==
                PianoRollEventsView::TempoControllerEventsMode) {
                _pianoRollView->mainView()->controllerEventsRulerView()->setRange(30, 10, 30);
            } else {
                _pianoRollView->mainView()->controllerEventsRulerView()->setRange(0, 0, 0);
            }
            break;

        default:
            break;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::controlChangeComboBoxDidSelectRow(MDStudio::ComboBox* sender, int row) {
    for (unsigned int i = 0; i < sender->rowViews().size(); ++i) {
        std::shared_ptr<MDStudio::View> view = sender->rowViews()[i];
        std::shared_ptr<MDStudio::ListItemView> listItemView = (std::static_pointer_cast<MDStudio::ListItemView>)(view);
        listItemView->setIsHighlighted(i == row);
    }

    sender->close();
    sender->setTitle(controllerMessages[row][0] == '*' ? controllerMessages[row] + 1 : controllerMessages[row]);

    updateControlChange();
    updateRuler();
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::metaTypeComboBoxDidSelectRow(MDStudio::ComboBox* sender, int row) {
    for (unsigned int i = 0; i < sender->rowViews().size(); ++i) {
        std::shared_ptr<MDStudio::View> view = sender->rowViews()[i];
        std::shared_ptr<MDStudio::ListItemView> listItemView = (std::static_pointer_cast<MDStudio::ListItemView>)(view);
        listItemView->setIsHighlighted(i == row);
    }

    sender->close();
    sender->setTitle(metaTypes[row][0] == '*' ? metaTypes[row] + 1 : metaTypes[row]);

    updateMetaType();
    updateRuler();
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::controlChangeComboBoxDidHoverRow(MDStudio::ComboBox* sender, int row) {
    for (unsigned int i = 0; i < sender->rowViews().size(); ++i) {
        std::shared_ptr<MDStudio::View> view = sender->rowViews()[i];
        std::shared_ptr<MDStudio::ListItemView> listItemView = (std::static_pointer_cast<MDStudio::ListItemView>)(view);
        listItemView->setIsHovering(i == row);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::metaTypeComboBoxDidHoverRow(MDStudio::ComboBox* sender, int row) {
    for (unsigned int i = 0; i < sender->rowViews().size(); ++i) {
        std::shared_ptr<MDStudio::View> view = sender->rowViews()[i];
        std::shared_ptr<MDStudio::ListItemView> listItemView = (std::static_pointer_cast<MDStudio::ListItemView>)(view);
        listItemView->setIsHovering(i == row);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::controlChangeComboBoxDidSetFocusState(MDStudio::ComboBox* sender, bool state) {
    for (unsigned int i = 0; i < sender->rowViews().size(); ++i) {
        std::shared_ptr<MDStudio::View> view = sender->rowViews()[i];
        std::shared_ptr<MDStudio::ListItemView> listItemView = (std::static_pointer_cast<MDStudio::ListItemView>)(view);
        listItemView->setFocusState(state);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::metaTypeComboBoxDidSetFocusState(MDStudio::ComboBox* sender, bool state) {
    for (unsigned int i = 0; i < sender->rowViews().size(); ++i) {
        std::shared_ptr<MDStudio::View> view = sender->rowViews()[i];
        std::shared_ptr<MDStudio::ListItemView> listItemView = (std::static_pointer_cast<MDStudio::ListItemView>)(view);
        listItemView->setFocusState(state);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::updateCursorTickPos(bool isCentered) {
    _pianoRollView->mainView()->pianoRollEventsView()->setCursorTickPos(
        _studioController->studio()->metronome()->tick(), false);
    _pianoRollView->mainView()->pianoRollControllerEventsView()->setCursorTickPos(
        _studioController->studio()->metronome()->tick(), false);
    _pianoRollView->mainView()->pianoRollMetaEventView()->setCursorTickPos(
        _studioController->studio()->metronome()->tick(), false);
    if (isCentered) {
        _pianoRollView->mainView()->scrollToCenteredCursor();
    } else {
        _pianoRollView->mainView()->scrollToVisibleCursor();
    }
    updateFlagControls();

    if (_didSetCursorTickPosFn) _didSetCursorTickPosFn(this, _studioController->studio()->metronome()->tick());
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::goToPreviousMeasure() {
    if (!_sequence ||
        (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusRecording) ||
        (_trackIndex < 0))
        return;

    _studioController->stop();

    std::vector<unsigned int> measureTicks = getMeasureTicks();

    if (measureTicks.size() == 0) {
        _studioController->studio()->metronome()->moveToTick(0);
    } else {
        unsigned int currentTick = _studioController->studio()->metronome()->tick();

        // Find the index of the current measure
        size_t i = 0;
        for (auto tick : measureTicks) {
            if (tick >= currentTick) break;
            i++;
        }

        if (i > 0) i--;

        _studioController->studio()->metronome()->moveToTick(measureTicks[i]);
    }

    updateCursorTickPos(false);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::goToNextMeasure() {
    if (!_sequence ||
        (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusRecording) ||
        (_trackIndex < 0))
        return;

    _studioController->stop();

    std::vector<unsigned int> measureTicks = getMeasureTicks();
    if (measureTicks.size() == 0) {
        _studioController->studio()->metronome()->moveToTick(_totalTickCounts[_trackIndex]);
    } else {
        unsigned int currentTick = _studioController->studio()->metronome()->tick();

        if (currentTick >= measureTicks[measureTicks.size() - 1]) {
            _studioController->studio()->metronome()->moveToTick(
                std::max(_totalTickCounts[_trackIndex], measureTicks[measureTicks.size() - 1]));
        } else {
            // Find the index of the current measure
            size_t i = 0;
            for (auto tick : measureTicks) {
                if (tick >= currentTick) break;
                i++;
            }

            if ((i < (measureTicks.size() - 1)) && (currentTick == measureTicks[i])) i++;
            _studioController->studio()->metronome()->moveToTick(measureTicks[i]);
        }
    }

    updateCursorTickPos(false);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollViewController::utilitiesViewSelectionDidSelectSegment(MDStudio::SegmentedControl* sender,
                                                                     int selectedSegment) {
    _pianoRollView->pianoRollUtilitiesView()->pianoRollPropertiesView()->setIsVisible(selectedSegment == 0);
    _pianoRollView->pianoRollUtilitiesView()->pianoRollEventsListView()->setIsVisible(selectedSegment == 1);

    if (selectedSegment == 1) updateEventList();
}
