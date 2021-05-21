//
//  pianorollviewcontroller.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-01-09.
//  Copyright (c) 2015-2021 Daniel Cliche. All rights reserved.
//

#ifndef PIANOROLLVIEWCONTROLLER_H
#define PIANOROLLVIEWCONTROLLER_H

#include <segmentedcontrol.h>
#include <sequence.h>
#include <sequenceeditor.h>
#include <sequencesdb.h>

#include <atomic>
#include <memory>

#include "pianorollpropertiesviewcontroller.h"
#include "pianorollview.h"
#include "studiocontroller.h"

class PianoRollViewController {
   public:
    typedef std::function<void(PianoRollViewController* sender)> WillModifySequenceFnType;
    typedef std::function<void(PianoRollViewController* sender)> DidModifySequenceFnType;
    typedef std::function<void(PianoRollViewController* sender)> DidSelectEventsFnType;
    typedef std::function<void(PianoRollViewController* sender)> DidSetPaneVisibilityFnType;
    typedef std::function<void(PianoRollViewController* sender, int zone)> DidSelectZoneFnType;
    typedef std::function<void(PianoRollViewController* sender, unsigned int cursorTickPos)> DidSetCursorTickPosFnType;
    typedef std::function<void(PianoRollViewController* sender)> WillModifyAnnotationsFnType;
    typedef std::function<void(PianoRollViewController* sender)> DidModifyAnnotationsFnType;

   private:
    PianoRollEventsView::ControllerEventsModeEnum modes[12] = {
        PianoRollEventsView::ControlChangeControllerEventsMode,
        PianoRollEventsView::ProgramChangesControllerEventsMode,
        PianoRollEventsView::PitchBendControllerEventsMode,
        PianoRollEventsView::KeyAftertouchControllerEventsMode,
        PianoRollEventsView::ChannelAftertouchControllerEventsMode,
        PianoRollEventsView::SysexControllerEventsMode,
        PianoRollEventsView::MetaControllerEventsMode};

    MelobaseCore::StudioController* _studioController;
    MelobaseCore::SequenceEditor* _sequenceEditor;

    int _currentChannel, _currentMultiChannel;
    int _trackIndex;

    int _timeDivision;
    std::vector<unsigned int> _totalTickCounts, _eotTickCounts;
    float _measureWidth;

    std::shared_ptr<PianoRollView> _pianoRollView;

    std::shared_ptr<MelobaseCore::Sequence> _sequence;

    PianoRollPropertiesViewController* _pianoRollPropertiesViewController;

    bool _isKeyboardUpdated;

    int _previousDeltaTicks;
    int _previousDeltaPitch;
    int _previousDeltaValue;

    bool _isSelectingFromEventsList;

    int _oldEditionSelectedSegment;

    // Subscribed notifications
    std::shared_ptr<MDStudio::Studio::didPlayNoteFnType> _studioDidPlayNoteFn;
    std::shared_ptr<MDStudio::Studio::didReleaseNoteFnType> _studioDidReleaseNoteFn;
    std::shared_ptr<MDStudio::Studio::didSetInstrumentFnType> _studioDidSetInstrumentFn;

    WillModifySequenceFnType _willModifySequenceFn = nullptr;
    DidModifySequenceFnType _didModifySequenceFn = nullptr;
    DidSelectEventsFnType _didSelectEventsFn = nullptr;
    DidSetPaneVisibilityFnType _didSetPaneVisibilityFn = nullptr;
    DidSelectZoneFnType _didSelectZoneFn = nullptr;
    DidSetCursorTickPosFnType _didSetCursorTickPosFn = nullptr;
    WillModifyAnnotationsFnType _willModifyAnnotationsFn = nullptr;
    DidModifyAnnotationsFnType _didModifyAnnotationsFn = nullptr;

    void addAnnotations(std::vector<std::shared_ptr<MelobaseCore::SequenceAnnotation>> annotation);
    void removeAnnotations(std::vector<std::shared_ptr<MelobaseCore::SequenceAnnotation>> annotation);
    bool goToPreviousAnnotation(bool isCheckingOnly = false);
    bool goToNextAnnotation(bool isCheckingOnly = false);

    void pianoKeyPressed(KeyboardV* sender, int pitch);
    void pianoKeyReleased(KeyboardV* sender, int pitch);

    void headerSequenceInfos(PianoRollHeaderView* sender, int* timeDivision, std::vector<unsigned int>* totalNbTicks,
                             std::vector<unsigned int>* eotTickCounts, bool* areTicksAbs);
    unsigned int headerNbEvents(PianoRollHeaderView* sender, int track);
    void headerEventAtIndex(PianoRollHeaderView* sender, int track, int index,
                            std::shared_ptr<MelobaseCore::Event>* event);
    void headerDidSetCursorTickPos(PianoRollHeaderView* sender, unsigned int cursorTickPos);

    void sequenceInfos(PianoRollEventsView* sender, int* timeDivision, std::vector<unsigned int>* totalNbTicks,
                       std::vector<unsigned int>* eotTickCounts, bool* areTicksAbs);

    int nbEvents(PianoRollEventsView* sender, int track);
    void eventAtIndex(PianoRollEventsView* sender, int track, int index, std::shared_ptr<MelobaseCore::Event>* event);
    int nbAnnotations(PianoRollEventsView* sender);
    void annotationAtIndex(PianoRollEventsView* sender, int index,
                           std::shared_ptr<MelobaseCore::SequenceAnnotation>* annotation);
    int eventsListNbEvents(const PianoRollEventsListView* sender, int track);
    void eventsListEventAtIndex(const PianoRollEventsListView* sender, int track, int index,
                                std::shared_ptr<MelobaseCore::Event>* event, bool* isChannelEditionAvail);
    void eventsListDidSelectEvents(PianoRollEventsListView* sender,
                                   std::vector<std::shared_ptr<MelobaseCore::Event>> events);
    void eventsListEventDidChange(PianoRollEventsListView* sender, std::shared_ptr<MelobaseCore::Event> event,
                                  UInt32 tickCount, UInt32 length, UInt8 channel, SInt32 param1, SInt32 param2,
                                  SInt32 param3, std::vector<UInt8> data);
    void didSetCursorTickPos(PianoRollEventsView* sender, unsigned int cursorTickPos);
    void didFinishSettingCursorTickPos(PianoRollEventsView* sender);
    void willSetSelectionRegion(PianoRollEventsView* sender);
    void didSetSelectionRegion(PianoRollEventsView* sender);
    std::vector<unsigned int> getMeasureTicks();
    void previousMeasure(PianoRollEventsView* sender);
    void nextMeasure(PianoRollEventsView* sender);
    void deleteEvents(PianoRollEventsView* sender);
    void willMoveEvents(PianoRollEventsView* sender);
    void moveEvents(PianoRollEventsView* sender, int deltaTicks, int deltaPitch, int deltaValue, bool isResizing,
                    bool isAddingNote);
    void didMoveEvents(PianoRollEventsView* sender);

    unsigned int quantizedResTickCount();
    unsigned int quantizedTickPos(unsigned int tickPos);
    void addNoteEvent(PianoRollEventsView* sender, unsigned int tickPos, int pitch);
    void addSustainEvent(PianoRollEventsView* sender, unsigned int tickPos, int value);
    void addProgramChangeEvent(PianoRollEventsView* sender, unsigned int tickPos);
    void addTempoEvent(PianoRollEventsView* sender, unsigned int tickPos, int bpm);
    void addPitchBendEvent(PianoRollEventsView* sender, unsigned int tickPos, int value);
    void addModulationEvent(PianoRollEventsView* sender, unsigned int tickPos, int value);
    void addMixerLevelEvent(PianoRollEventsView* sender, unsigned int tickPos, int value);
    void addMixerBalanceEvent(PianoRollEventsView* sender, unsigned int tickPos, int value);
    void addTimeSignatureEvent(PianoRollEventsView* sender, unsigned int tickPos);
    void addControlChangeEvent(PianoRollEventsView* sender, unsigned int tickPos, int control, int value);
    void addKeyAftertouchEvent(PianoRollEventsView* sender, unsigned int tickPos, int pitch, int value);
    void addChannelAftertouchEvent(PianoRollEventsView* sender, unsigned int tickPos, int value);
    void addSysexEvent(PianoRollEventsView* sender, unsigned int tickPos);
    void addMetaEvent(PianoRollEventsView* sender, unsigned int tickPos, int type);
    void didSelectEvents(PianoRollEventsView* sender, bool areCombined, bool isAddingNote);
    void setSelectionState(PianoRollEventsView* sender, bool state);

    void pianoRollEventsScrollViewPosChanged(MDStudio::ScrollView* sender, MDStudio::Point pos);

    void studioControllerDidGoToBeginning(MelobaseCore::StudioController* sender);

    void studioDidPlayNote(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel, int pitch,
                           Float32 velocity);
    void studioDidReleaseNote(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel,
                              int pitch);
    void studioDidSetInstrument(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel,
                                int instrument);

    void editionModeSegmentedControlDidSelectSegment(MDStudio::SegmentedControl* sender, int selectedSegment);
    void zoneSelectionSegmentedControlDidSelectSegment(MDStudio::SegmentedControl* sender, int selectedSegment);

    void controllerSegmentedControlVDidSelectSegment(SegmentedControlV* sender, int selectedSegment);

    void zoomSliderPosChanged(MDStudio::Slider* sender, float pos);

    void visibleChannelButtonStateDidChange(MDStudio::Button* sender, bool state, unsigned int modifierFlags);
    void allVisibleChannelButtonClicked(MDStudio::Button* sender);
    void visibleControllerPaneButtonStateDidChange(MDStudio::Button* sender, bool state);
    void visiblePropertiesPaneButtonStateDidChange(MDStudio::Button* sender, bool state);

    void scrollToVerticalCenter();

    void keyboardScroll(KeyboardV* sender, float deltaY);

    void setPianoRollEventsViewFns(std::shared_ptr<PianoRollEventsView> pianoRollEventsView);

    void pianoRollPropertiesChannelDidChange(PianoRollPropertiesViewController* sender, int channel);
    void pianoRollPropertiesVelocityDidChange(PianoRollPropertiesViewController* sender, int velocity);
    void pianoRollPropertiesProgramDidChange(PianoRollPropertiesViewController* sender, int instrument);
    void pianoRollPropertiesTimeSignatureDidChange(PianoRollPropertiesViewController* sender, int numerator,
                                                   int denominator);
    void pianoRollPropertiesSysexDataDidChange(PianoRollPropertiesViewController* sender, std::vector<UInt8> data);
    void pianoRollPropertiesMetaDataDidChange(PianoRollPropertiesViewController* sender, std::vector<UInt8> data);
    void pianoRollPropertiesPitchDidChange(PianoRollPropertiesViewController* sender, int pitch);

    unsigned int controlChangeComboBoxNbRows(MDStudio::ComboBox* sender);
    std::shared_ptr<MDStudio::View> controlChangeComboBoxViewForRow(MDStudio::ComboBox* sender, int row);
    void controlChangeComboBoxDidSelectRow(MDStudio::ComboBox* sender, int row);
    void controlChangeComboBoxDidHoverRow(MDStudio::ComboBox* sender, int row);
    void controlChangeComboBoxDidSetFocusState(MDStudio::ComboBox* sender, bool state);

    unsigned int metaTypeComboBoxNbRows(MDStudio::ComboBox* sender);
    std::shared_ptr<MDStudio::View> metaTypeComboBoxViewForRow(MDStudio::ComboBox* sender, int row);
    void metaTypeComboBoxDidSelectRow(MDStudio::ComboBox* sender, int row);
    void metaTypeComboBoxDidHoverRow(MDStudio::ComboBox* sender, int row);
    void metaTypeComboBoxDidSetFocusState(MDStudio::ComboBox* sender, bool state);

    void updateControlChange();
    void updateMetaType();
    void updateRuler();
    void updateCursorTickPos(bool isCentered);

    std::vector<std::shared_ptr<MelobaseCore::Event>> selectedEventsWithAssociates();

    void quantizeButtonClicked(MDStudio::Button* sender);

    void setZoom(float zoom);

    void scrollToVisibleRect(PianoRollEventsView* pianoRollEventsView);

    void addControllerEvents(unsigned int tickPos);

    unsigned int currentNbEvents(int trackIndex);
    void currentEventAtIndex(int trackIndex, int index, std::shared_ptr<MelobaseCore::Event>* event);

    void setCurrentChannel(int currentChannel, bool isMultiChannel);

    void updateControlChangeMetaComboBoxVisibility();

    void utilitiesViewSelectionDidSelectSegment(MDStudio::SegmentedControl* sender, int selectedSegment);

   public:
    PianoRollViewController(std::shared_ptr<PianoRollView> pianoRollView,
                            MelobaseCore::StudioController* studioController,
                            MelobaseCore::SequenceEditor* sequenceEditor);
    ~PianoRollViewController();

    void setSequence(std::shared_ptr<MelobaseCore::Sequence> sequence);
    std::shared_ptr<MelobaseCore::Sequence> sequence() { return _sequence; }

    void setIsEditionAvailable(bool isEditionAvailable);

    void selectAllEvents();
    bool deleteSelectedEvents();
    void quantizeSelectedEvents();
    bool copySelectedEvents();
    bool cutSelectedEvents();
    bool pasteEvents();

    bool areEventsSelected() { return selectedEventsWithAssociates().size() > 0 ? true : false; };

    void updatePianoRoll();
    void updateProperties();
    void updateEventList();
    void updateFlagControls();

    void showHideControllerEvents();
    void showHideProperties();

    bool areControllerEventsVisible() { return _pianoRollView->visibleControllerPaneButton()->state(); }
    bool arePropertiesVisible() { return _pianoRollView->visiblePropertiesPaneButton()->state(); }

    void setIsKeyboardUpdated(bool isKeyboardUpdated);

    void setCurrentMultiChannel(int currentMultiChannel);

    int currentZone() { return _pianoRollView->mainView()->zoneSelectionSegmentedControl()->selectedSegment(); }

    MDStudio::Point pianoRollEventsScrollViewPos() {
        return _pianoRollView->mainView()->pianoRollEventsScrollView()->pos();
    }
    void setPianoRollEventsScrollViewPos(MDStudio::Point pos) {
        _pianoRollView->mainView()->pianoRollEventsScrollView()->setPos(pos);
    }

    int editionModeIndex() { return _pianoRollView->editionSegmentedControl()->selectedSegment(); }
    void setEditionModeIndex(int modeIndex) {
        if (_pianoRollView->editionSegmentedControl()->isEnabled())
            _pianoRollView->editionSegmentedControl()->setSelectedSegment(modeIndex);
    }

    void setTrackIndex(int trackIndex);
    int trackIndex() { return _trackIndex; }

    void setCursorTickPos(unsigned int tickPos);
    void handleEventRepeat();

    void goToPreviousMeasure();
    void goToNextMeasure();

    void setWillModifySequenceFn(WillModifySequenceFnType willModifySequenceFn) {
        _willModifySequenceFn = willModifySequenceFn;
    }
    void setDidModifySequenceFn(DidModifySequenceFnType didModifySequenceFn) {
        _didModifySequenceFn = didModifySequenceFn;
    }
    void setDidSelectEventsFn(DidSelectEventsFnType didSelectEventsFn) { _didSelectEventsFn = didSelectEventsFn; }
    void setDidSetPaneVisibilityFn(DidSetPaneVisibilityFnType didSetPaneVisibilityFn) {
        _didSetPaneVisibilityFn = didSetPaneVisibilityFn;
    }
    void setDidSelectZoneFn(DidSelectZoneFnType didSelectZoneFn) { _didSelectZoneFn = didSelectZoneFn; }
    void setDidSetCursorTickPosFn(DidSetCursorTickPosFnType didSetCursorTickPosFn) {
        _didSetCursorTickPosFn = didSetCursorTickPosFn;
    }
    void setWillModifyAnnotationsFn(WillModifyAnnotationsFnType willModifyAnnotationsFn) {
        _willModifyAnnotationsFn = willModifyAnnotationsFn;
    }
    void setDidModifyAnnotationsFn(DidModifyAnnotationsFnType didModifyAnnotationsFn) {
        _didModifyAnnotationsFn = didModifyAnnotationsFn;
    }
};

#endif  // PIANOROLLVIEWCONTROLLER
