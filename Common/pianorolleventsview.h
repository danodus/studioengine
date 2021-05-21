//
//  pianorolleventsview.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2014-06-28.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#ifndef PIANOROLLEVENTSVIEW_H
#define PIANOROLLEVENTSVIEW_H

#include <view.h>
#include <scrollview.h>
#include <melobasecore_sequence.h>
#include <studio.h>
#include <image.h>

#include <functional>
#include <array>

struct EventRect {
    MDStudio::Rect rect;
    std::shared_ptr<MelobaseCore::ChannelEvent> channelEvent;
};

class PianoRollEventsView : public MDStudio::View
{
public:
    typedef std::function<void(PianoRollEventsView *sender, int *timeDivision, std::vector<unsigned int> *totalNbTicks, std::vector<unsigned int> *eotTickCount, bool *areAbsTicks)> SequenceInfosFnType;
    typedef std::function<unsigned int(PianoRollEventsView *sender, int track)> NbEventsFnType;
    typedef std::function<void(PianoRollEventsView *sender, int track, int index, std::shared_ptr<MelobaseCore::Event> *event)> EventAtIndexFnType;
    typedef std::function<unsigned int(PianoRollEventsView* sender)> NbAnnotationsFnType;
    typedef std::function<void(PianoRollEventsView* sender, int index, std::shared_ptr<MelobaseCore::SequenceAnnotation>* annotation)> AnnotationAtIndexFnType;
    typedef std::function<void(PianoRollEventsView *sender, unsigned int cursorTickPos)> DidSetCursorTickPosFnType;
    typedef std::function<void(PianoRollEventsView *sender)> DidFinishSettingCursorTickPosFnType;
    typedef std::function<void(PianoRollEventsView *sender)> WillSetSelectionRegionFnType;
    typedef std::function<void(PianoRollEventsView *sender)> DidSetSelectionRegionFnType;
    typedef std::function<void(PianoRollEventsView *sender, bool areCombined, bool isAddingNote)> DidSelectEventsFnType;
    typedef std::function<void(PianoRollEventsView *sender)> PreviousMeasureFnType;
    typedef std::function<void(PianoRollEventsView *sender)> NextMeasureFnType;
    typedef std::function<void(PianoRollEventsView *sender)> DeleteEventsFnType;
    typedef std::function<void(PianoRollEventsView *sender)> WillMoveEvents;
    typedef std::function<void(PianoRollEventsView *sender, int deltaTicks, int deltaPitch, int deltaValue, bool isResizing, bool isAddingNote)> MoveEventsFnType;
    typedef std::function<void(PianoRollEventsView *sender)> DidMoveEvents;
    typedef std::function<void(PianoRollEventsView *sender, unsigned int tickPos, int pitch)> AddNoteEventFnType;
    typedef std::function<void(PianoRollEventsView *sender, unsigned int tickPos, int value)> AddSustainEventFnType;
    typedef std::function<void(PianoRollEventsView *sender, unsigned int tickPos)> AddProgramChangeEventFnType;
    typedef std::function<void(PianoRollEventsView *sender, unsigned int tickPos, int bpm)> AddTempoEventFnType;
    typedef std::function<void(PianoRollEventsView *sender, unsigned int tickPos, int value)> AddPitchBendEventFnType;
    typedef std::function<void(PianoRollEventsView *sender, unsigned int tickPos, int value)> AddModulationEventFnType;
    typedef std::function<void(PianoRollEventsView *sender, unsigned int tickPos, int value)> AddMixerLevelEventFnType;
    typedef std::function<void(PianoRollEventsView *sender, unsigned int tickPos, int value)> AddMixerBalanceEventFnType;
    typedef std::function<void(PianoRollEventsView *sender, unsigned int tickPos, int control, int value)> AddControlChangeEventFnType;
    typedef std::function<void(PianoRollEventsView *sender, unsigned int tickPos, int pitch, int value)> AddKeyAftertouchEventFnType;
    typedef std::function<void(PianoRollEventsView *sender, unsigned int tickPos, int value)> AddChannelAftertouchEventFnType;
    typedef std::function<void(PianoRollEventsView *sender, unsigned int tickPos)> AddSysexEventFnType;
    typedef std::function<void(PianoRollEventsView *sender, unsigned int tickPos, int type)> AddMetaEventFnType;
    typedef std::function<void(PianoRollEventsView *sender, unsigned int tickPos)> AddTimeSignatureEventFnType;
    typedef std::function<void(PianoRollEventsView *sender, bool state)> SetSelectionStateFnType;
    typedef std::function<void(PianoRollEventsView *sender, bool state)> DidSetFocusStateFnType;
    
    typedef enum { ArrowMode, SelectionMode, DrawingMode, MoveMode, ResizeMode } ModeEnum;
    
    typedef enum { SustainControllerEventsMode, ProgramChangesControllerEventsMode, TempoControllerEventsMode, PitchBendControllerEventsMode, ModulationControllerEventsMode, EndOfTrackTimeSignatureControllerEventsMode, MixerLevelControllerEventsMode, MixerBalanceControllerEventsMode, KeyAftertouchControllerEventsMode, ChannelAftertouchControllerEventsMode, SysexControllerEventsMode, ControlChangeControllerEventsMode, MetaControllerEventsMode } ControllerEventsModeEnum;

private:
    
    void drawFilledRoundRectTopRight(MDStudio::Rect rect, float radius, MDStudio::Color fillColor);
    void drawFilledRoundRectTopLeft(MDStudio::Rect rect, float radius, MDStudio::Color fillColor);
    
    MDStudio::Studio *_studio;
    
    void updateEventRects(int trackIndex, bool areCombined, bool areChannelEventsSkipped);
    void updateSelectedEvents(int trackIndex, bool areCombined, bool areChannelEventsSkipped);

    void drawAnnotations();
    
    double _eventTickWidth;
    float _eventHeight;
    unsigned int _cursorTickPos;
    MDStudio::Rect _selectionRect;
    MDStudio::Point _lastSelectionPt;
    bool _isCaptured;
    bool _hasFocus;
    ModeEnum _mode;
    ControllerEventsModeEnum _controllerEventsMode;
    int _controlChange;
    int _metaType;
    bool _isMovingEvents;
    bool _isResizingEvents;
    bool _isMouseInside;
    unsigned int _moveEventsRefTickPos;
    MDStudio::Point _moveEventsRefPos;
    int _moveEventsRefPitch;
    std::array<bool, STUDIO_MAX_CHANNELS> _visibleChannels;
    std::shared_ptr<MDStudio::Image> _endOfTrackImage;

    // Data source
    SequenceInfosFnType _sequenceInfosFn = nullptr;
    NbEventsFnType _nbEventsFn = nullptr;
    EventAtIndexFnType _eventAtIndexFn = nullptr;
    NbAnnotationsFnType _nbAnnotationsFn = nullptr;
    AnnotationAtIndexFnType _annotationAtIndexFn = nullptr;

    DidSetCursorTickPosFnType _didSetCursorTickPosFn = nullptr;
    DidFinishSettingCursorTickPosFnType _didFinishSettingCursorTickPosFn = nullptr;
    WillSetSelectionRegionFnType _willSetSelectionRegionFn = nullptr;
    DidSetSelectionRegionFnType _didSetSelectionRegionFn = nullptr;
    DidSelectEventsFnType _didSelectEventsFn = nullptr;

    PreviousMeasureFnType _previousMeasureFn = nullptr;
    NextMeasureFnType _nextMeasureFn = nullptr;
    DeleteEventsFnType _deleteEventsFn = nullptr;
    WillMoveEvents _willMoveEventsFn = nullptr;
    MoveEventsFnType _moveEventsFn = nullptr;
    DidMoveEvents _didMoveEventsFn = nullptr;
    AddNoteEventFnType _addNoteEventFn = nullptr;
    AddSustainEventFnType _addSustainEventFn = nullptr;
    AddProgramChangeEventFnType _addProgramChangeEventFn = nullptr;
    AddTempoEventFnType _addTempoEventFn = nullptr;
    AddPitchBendEventFnType _addPitchBendEventFn = nullptr;
    AddModulationEventFnType _addModulationEventFn = nullptr;
    AddMixerLevelEventFnType _addMixerLevelEventFn = nullptr;
    AddMixerBalanceEventFnType _addMixerBalanceEventFn = nullptr;
    AddTimeSignatureEventFnType _addTimeSignatureEventFn = nullptr;
    AddControlChangeEventFnType _addControlChangeEventFn = nullptr;
    AddKeyAftertouchEventFnType _addKeyAftertouchEventFn = nullptr;
    AddChannelAftertouchEventFnType _addChannelAftertouchEventFn = nullptr;
    AddSysexEventFnType _addSysexEventFn = nullptr;
    AddMetaEventFnType _addMetaEventFn = nullptr;
    SetSelectionStateFnType _setSelectionStateFn = nullptr;
    DidSetFocusStateFnType _didSetFocusStateFn = nullptr;
    
    std::vector<EventRect> _eventRects[STUDIO_MAX_CHANNELS];
    
    std::vector<std::shared_ptr<MelobaseCore::Event>> _selectedEvents;
    std::shared_ptr<MelobaseCore::Event> _activeEvent;
    
    bool _isShowingControllerEvents;
    
    bool _highlightPitchStates[STUDIO_MAX_CHANNELS][128];
    MDStudio::ScrollView *_mainScrollView;
    
    int _highlightChannel;
    
    bool _isMovingCursor;
    
    int _trackIndex;
    UInt8 _trackChannel;
    
    UInt8 rechannelize(UInt8 channel) { return _trackChannel == SEQUENCE_TRACK_MULTI_CHANNEL ? channel : _trackChannel; }
    
    void getMeasureTicks(int timeDivision, unsigned int totalNbTicks, bool areAbsTicks, std::vector<unsigned int> *measureTicks, std::vector<int> *numerators);

    void selectAll() override;
    
    MDStudio::Rect makeBoundingBox(const std::vector<MDStudio::Point> &points) const;
    
public:
    PianoRollEventsView(std::string name, void *owner, MDStudio::Studio *studio, int trackIndex, UInt8 trackChannel, double eventTickWidth, float eventHeight, bool isShowingControllerEvents, MDStudio::ScrollView *mainScrollView = nullptr);
    ~PianoRollEventsView();
    
    bool handleEvent(const MDStudio::UIEvent *event) override;
    void resignFirstResponder() override;
    
    void draw() override;
    
    void setEventTickWidth(double eventTickWidth) { _eventTickWidth = eventTickWidth; setDirty(); }
    void setEventHeight(float eventHeight) { _eventHeight = eventHeight; setDirty(); }
    
    double eventTickWidth() { return _eventTickWidth; }
    float eventHeight() { return _eventHeight; }
    
    void setCursorTickPos(unsigned int cursorTickPos, bool isDelegateNotified = true);
    unsigned int cursorTickPos() { return _cursorTickPos; }
    void setSelectionRect(MDStudio::Rect selectionRect) { _selectionRect = selectionRect; setDirty(); }

    bool isCaptured() { return _isCaptured; }
    bool hasFocus() { return _hasFocus; }
    
    std::vector<std::shared_ptr<MelobaseCore::Event>> selectedEvents();
    std::shared_ptr<MelobaseCore::Event> activeEvent() { return _activeEvent; }
    
    void setMode(ModeEnum mode) { _mode = mode; setDirty(); }
    ModeEnum mode() { return _mode; }
    
    void setControllerEventsMode(ControllerEventsModeEnum controllerEventsMode) { _controllerEventsMode = controllerEventsMode; setDirty(); }
    ControllerEventsModeEnum controllerEventsMode() { return _controllerEventsMode; }
    void setControlChange(int controlChange) { _controlChange = controlChange; setDirty(); }
    void setMetaType(int metaType) { _metaType = metaType; setDirty(); }
    void clearEventSelection(bool isDelegateNotified = true);
    void resetSelectionRegion();
    
    MDStudio::Rect selectionRegion() { return _selectionRect; }
    MDStudio::Point lastSelectionPt() { return _lastSelectionPt; }
    MDStudio::Rect selectedEventsFrame();
    
    void setVisibleChannels(std::array<bool, STUDIO_MAX_CHANNELS> visibleChannels);
    std::array<bool, STUDIO_MAX_CHANNELS> visibleChannels() { return _visibleChannels; }
    
    void selectEvents(const std::vector<std::shared_ptr<MelobaseCore::Event>> &events, bool isDelegateNotified = true);
    void selectAllEvents(bool isDelegateNotified = true);
    
    void setHighlightPitchState(int channel, int pitch, bool state);
    void setHighlightChannel(int channel) { _highlightChannel = channel; }
    
    void setAddedEvent(std::shared_ptr<MelobaseCore::Event> addedEvent);
    
    void setTrackIndex(int trackIndex);
    void setTrackChannel(UInt8 channel);
    
    void setSequenceInfosFn(SequenceInfosFnType sequenceInfosFn) { _sequenceInfosFn = sequenceInfosFn; }
    void setNbEventsFn(NbEventsFnType nbEventsFn) { _nbEventsFn = nbEventsFn; }
    void setEventAtIndexFn(EventAtIndexFnType eventAtIndexFn) { _eventAtIndexFn = eventAtIndexFn; }
    void setNbAnnotationsFn(NbAnnotationsFnType nbAnnotationsFn) { _nbAnnotationsFn = nbAnnotationsFn; }
    void setAnnotationAtIndexFn(AnnotationAtIndexFnType annotationAtIndexFn) { _annotationAtIndexFn = annotationAtIndexFn; }
    void setDidSetCursorTickPosFn(DidSetCursorTickPosFnType didSetCursorTickPosFn) { _didSetCursorTickPosFn = didSetCursorTickPosFn; }
    void setDidFinishSettingCursorTickPosFn(DidFinishSettingCursorTickPosFnType didFinishSettingCursorTickPosFn) { _didFinishSettingCursorTickPosFn = didFinishSettingCursorTickPosFn; }
    void setWillSetSelectionRegionFn(WillSetSelectionRegionFnType willSetSelectionRegionFn) { _willSetSelectionRegionFn = willSetSelectionRegionFn; }
    void setDidSetSelectionRegionFn(DidSetSelectionRegionFnType didSetSelectionRegionFn) { _didSetSelectionRegionFn = didSetSelectionRegionFn; }
    void setPreviousMeasureFn(PreviousMeasureFnType previousMeasureFn) { _previousMeasureFn = previousMeasureFn; }
    void setNextMeasureFn(NextMeasureFnType nextMeasureFn) { _nextMeasureFn = nextMeasureFn; }
    void setDeleteEventsFn(DeleteEventsFnType deleteEventsFn) { _deleteEventsFn = deleteEventsFn; }
    void setDidSelectEventsFn(DidSelectEventsFnType didSelectEventsFn) { _didSelectEventsFn = didSelectEventsFn; }
    void setWillMoveEventsFn(WillMoveEvents willMoveEventsFn) { _willMoveEventsFn = willMoveEventsFn; }
    void setMoveEventsFn(MoveEventsFnType moveEventsFn) { _moveEventsFn = moveEventsFn; }
    void setDidMoveEventsFn(DidMoveEvents didMoveEventsFn) { _didMoveEventsFn = didMoveEventsFn; }
    void setAddNoteEventFn(AddNoteEventFnType addNoteEventFn) { _addNoteEventFn = addNoteEventFn; }
    void setAddSustainEventFn(AddSustainEventFnType addSustainEventFn) { _addSustainEventFn = addSustainEventFn; }
    void setAddProgramChangeEventFn(AddProgramChangeEventFnType addProgramChangeEventFn) { _addProgramChangeEventFn = addProgramChangeEventFn; }
    void setAddTempoEventFn(AddTempoEventFnType addTempoEventFn) { _addTempoEventFn = addTempoEventFn; }
    void setAddPitchBendEventFn(AddPitchBendEventFnType addPitchBendEventFn) { _addPitchBendEventFn = addPitchBendEventFn; }
    void setAddModulationEventFn(AddModulationEventFnType addModulationEventFn) { _addModulationEventFn = addModulationEventFn; }
    void setAddMixerLevelEventFn(AddMixerLevelEventFnType addMixerLevelEventFn) { _addMixerLevelEventFn = addMixerLevelEventFn; }
    void setAddMixerBalanceEventFn(AddMixerBalanceEventFnType addMixerBalanceEventFn) { _addMixerBalanceEventFn = addMixerBalanceEventFn; }
    void setAddTimeSignatureEventFn(AddTimeSignatureEventFnType addTimeSignatureEventFn) { _addTimeSignatureEventFn = addTimeSignatureEventFn; }
    void setAddControlChangeEventFn(AddControlChangeEventFnType addControlChangeEventFn) { _addControlChangeEventFn = addControlChangeEventFn; }
    void setAddKeyAftertouchEventFn(AddKeyAftertouchEventFnType addKeyAftertouchEventFn) { _addKeyAftertouchEventFn = addKeyAftertouchEventFn; }
    void setAddChannelAftertouchEventFn(AddChannelAftertouchEventFnType addChannelAftertouchEventFn) { _addChannelAftertouchEventFn = addChannelAftertouchEventFn; }
    void setAddSysexEventFn(AddSysexEventFnType addSysexEventFn) { _addSysexEventFn = addSysexEventFn; }
    void setAddMetaEventFn(AddMetaEventFnType addMetaEventFn) { _addMetaEventFn = addMetaEventFn; }
    void setSetSelectionStateFn(SetSelectionStateFnType setSelectionStateFn) { _setSelectionStateFn = setSelectionStateFn; }
    void setDidSetFocusStateFn(DidSetFocusStateFnType didSetFocusStateFn) { _didSetFocusStateFn = didSetFocusStateFn; }
};


#endif // PIANOROLLEVENTSVIEW_H
