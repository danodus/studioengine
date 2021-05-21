//
//  pianorolleventsview.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2014-06-28.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#include "pianorolleventsview.h"

#include <drawcontext.h>
#include <draw.h>
#include <responderchain.h>

#include "helpers.h"

#include <platform.h>

#include <algorithm>

#define PIANO_ROLL_EVENTS_VIEW_NB_RESIZE_HANDLE_WIDTH   6.0f

#define PIANO_ROLL_EVENTS_VIEW_MAX_NB_TICKS             (480*4*20000)

// ---------------------------------------------------------------------------------------------------------------------
PianoRollEventsView::PianoRollEventsView(std::string name, void *owner, MDStudio::Studio *studio, int trackIndex, UInt8 trackChannel, double eventTickWidth, float eventHeight, bool isShowingControllerEvents, MDStudio::ScrollView *mainScrollView) : _studio(studio), _trackIndex(trackIndex), _trackChannel(trackChannel), _eventTickWidth(eventTickWidth), _eventHeight(eventHeight), _isShowingControllerEvents(isShowingControllerEvents), _mainScrollView(mainScrollView), View(name, owner)
{
    _cursorTickPos = 0;
    _selectionRect = MDStudio::makeZeroRect();
    _lastSelectionPt = MDStudio::makeZeroPoint();
    _isCaptured = false;
    _hasFocus = false;
    _mode = ArrowMode;
    _controllerEventsMode = SustainControllerEventsMode;
    _controlChange = 0;
    _metaType = 0;
    _isMovingEvents = false;
    _isResizingEvents = false;
    _isMouseInside = false;
    _moveEventsRefTickPos = 0;
    
    for (int channel = 0; channel < STUDIO_MAX_CHANNELS; ++channel) {
        _visibleChannels[channel] = true;

        for (int pitch = 0; pitch < 128; ++pitch) {
            _highlightPitchStates[channel][pitch] = false;
        }
    }
    
    _highlightChannel = 0;
    _isMovingCursor = false;
    
    _endOfTrackImage = std::make_shared<MDStudio::Image>("EndOfTrack@2x.png");
}

// ---------------------------------------------------------------------------------------------------------------------
PianoRollEventsView::~PianoRollEventsView()
{
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsView::updateEventRects(int trackIndex, bool areCombined, bool areChannelEventsSkipped)
{
    if (!areCombined) {
        for (int i = 0; i < STUDIO_MAX_CHANNELS; ++i)
            _eventRects[i].clear();
    }
    
    if (!_sequenceInfosFn || !_nbEventsFn || !_eventAtIndexFn || (trackIndex < 0))
        return;
    
    unsigned int nbEvents = _nbEventsFn(this, trackIndex);
    unsigned int currentTickCount = 0;
    
    std::vector<unsigned int> totalNbTicks, eotTickCounts;
    int timeDivision = 480;
    bool areAbsTicks = true;
    
    // Get sequence infos
    _sequenceInfosFn(this, &timeDivision, &totalNbTicks, &eotTickCounts, &areAbsTicks);
    
    if ((totalNbTicks.size() == 0) || (totalNbTicks[trackIndex] > PIANO_ROLL_EVENTS_VIEW_MAX_NB_TICKS) || (timeDivision == 0))
        return;
    
    const float margin = 10.0f;
    float height = bounds().size.height - (2.0f * margin);
    
    int noteOnEventRectIndices[STUDIO_MAX_CHANNELS][128];
    UInt32 noteOnTickCounts[STUDIO_MAX_CHANNELS][128];
    for (int channel = 0; channel < STUDIO_MAX_CHANNELS; ++channel)
        for (int pitch = 0; pitch < 128; ++pitch)
            noteOnEventRectIndices[channel][pitch] = -1;
    
    // Draw the events
    for (unsigned int i = 0; i < nbEvents; i++) {
        std::shared_ptr<MelobaseCore::Event> event;
        _eventAtIndexFn(this, trackIndex, i, &event);
        auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(event);
        if (areAbsTicks) {
            currentTickCount = channelEvent->tickCount();
        } else {
            currentTickCount += channelEvent->tickCount();
        }
        
        if (areChannelEventsSkipped && (channelEvent->type() != CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE) && (channelEvent->type() != CHANNEL_EVENT_TYPE_META_SET_TEMPO))
            continue;
        
        switch (channelEvent->type()) {
            case CHANNEL_EVENT_TYPE_NOTE:
            {
                if (!_isShowingControllerEvents ) {
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth, (channelEvent->param1() - 12) * _eventHeight, channelEvent->length() * _eventTickWidth, _eventHeight);
                    EventRect eventRect;
                    eventRect.rect = r;
                    eventRect.channelEvent = channelEvent;
                    _eventRects[rechannelize(channelEvent->channel())].push_back(eventRect);
                    noteOnEventRectIndices[rechannelize(channelEvent->channel())][channelEvent->param1()] = (int)(_eventRects[rechannelize(channelEvent->channel())].size() - 1);
                    noteOnTickCounts[rechannelize(channelEvent->channel())][channelEvent->param1()] = currentTickCount;
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_NOTE_OFF:
            {
                if (!_isShowingControllerEvents ) {
                    int noteOnEventRectIndice = noteOnEventRectIndices[rechannelize(channelEvent->channel())][channelEvent->param1()];
                    if (noteOnEventRectIndice >= 0) {
                        auto noteOnEventRect = &_eventRects[rechannelize(channelEvent->channel())][noteOnEventRectIndice];
                        // Adjust the rect
                        UInt32 length = currentTickCount - noteOnTickCounts[rechannelize(channelEvent->channel())][channelEvent->param1()];
                        noteOnEventRect->rect.size.width = length * _eventTickWidth;
                        noteOnEventRectIndices[rechannelize(channelEvent->channel())][channelEvent->param1()] = -1;
                    }
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE:
            {
                if (_isShowingControllerEvents && ((_controllerEventsMode == EndOfTrackTimeSignatureControllerEventsMode) || (_controllerEventsMode == MetaControllerEventsMode && _metaType == 88))) {
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth, 0.0f, 40.0f, bounds().size.height);
                    EventRect eventRect;
                    eventRect.rect = r;
                    eventRect.channelEvent = channelEvent;
                    _eventRects[rechannelize(channelEvent->channel())].push_back(eventRect);
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_SUSTAIN:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == SustainControllerEventsMode)) {
                    float value = static_cast<float>(channelEvent->param1());
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth - 5.0f, normalize(value, 0.0f, 128.0f) * height - 5.0f + margin, 10.0f, 10.0f);
                    EventRect eventRect;
                    eventRect.rect = r;
                    eventRect.channelEvent = channelEvent;
                    _eventRects[rechannelize(channelEvent->channel())].push_back(eventRect);
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_PROGRAM_CHANGE:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == ProgramChangesControllerEventsMode)) {
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth, 0.0f, 20.0f, bounds().size.height);
                    EventRect eventRect;
                    eventRect.rect = r;
                    eventRect.channelEvent = channelEvent;
                    _eventRects[rechannelize(channelEvent->channel())].push_back(eventRect);
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_SYSTEM_EXCLUSIVE:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == SysexControllerEventsMode)) {
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth, 0.0f, 20.0f, bounds().size.height);
                    EventRect eventRect;
                    eventRect.rect = r;
                    eventRect.channelEvent = channelEvent;
                    _eventRects[rechannelize(channelEvent->channel())].push_back(eventRect);
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_META_GENERIC:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == MetaControllerEventsMode)) {
                    if (channelEvent->param1() == _metaType) {
                        MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth, 0.0f, 20.0f, bounds().size.height);
                        EventRect eventRect;
                        eventRect.rect = r;
                        eventRect.channelEvent = channelEvent;
                        _eventRects[rechannelize(channelEvent->channel())].push_back(eventRect);
                    }
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_META_SET_TEMPO:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == TempoControllerEventsMode)) {
                    float bpm = 60000000.0f / (float)(channelEvent->param1());
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth - 5.0f, normalize(bpm, 30.0f, 300.0f) * height - 5.0f + margin, 10.0f, 10.0f);
                    EventRect eventRect;
                    eventRect.rect = r;
                    eventRect.channelEvent = channelEvent;
                    _eventRects[rechannelize(channelEvent->channel())].push_back(eventRect);
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_PITCH_BEND:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == PitchBendControllerEventsMode)) {
                    Float32 multiplier = channelEvent->param2() > 0 ? (Float32)channelEvent->param2() : 1.0f;
                    float value = multiplier * (((Float32)channelEvent->param1() - 0.5f) * 2.0f / 16383.0f - 1.0f);
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth - 5.0f, normalize(value, -2.0f, 2.0f) * height - 5.0f + margin, 10.0f, 10.0f);
                    EventRect eventRect;
                    eventRect.rect = r;
                    eventRect.channelEvent = channelEvent;
                    _eventRects[rechannelize(channelEvent->channel())].push_back(eventRect);
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_MODULATION:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == ModulationControllerEventsMode)) {
                    float value = static_cast<float>(channelEvent->param1());
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth - 5.0f, normalize(value, 0.0f, 128.0f) * height - 5.0f + margin, 10.0f, 10.0f);
                    EventRect eventRect;
                    eventRect.rect = r;
                    eventRect.channelEvent = channelEvent;
                    _eventRects[rechannelize(channelEvent->channel())].push_back(eventRect);
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_META_END_OF_TRACK:
            {
                if (_isShowingControllerEvents && ((_controllerEventsMode == EndOfTrackTimeSignatureControllerEventsMode) || (_controllerEventsMode == MetaControllerEventsMode && _metaType == 47))) {
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth - 20.0f, 0.0f, 20.0f, bounds().size.height);
                    EventRect eventRect;
                    eventRect.rect = r;
                    eventRect.channelEvent = channelEvent;
                    _eventRects[rechannelize(channelEvent->channel())].push_back(eventRect);
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_MIXER_LEVEL_CHANGE:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == MixerLevelControllerEventsMode)) {
                    float value = static_cast<float>(channelEvent->param1());
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth - 5.0f, normalize(value, 0.0f, (channelEvent->param2() == 0) ? 128.0f : 100) * height - 5.0f + margin, 10.0f, 10.0f);
                    EventRect eventRect;
                    eventRect.rect = r;
                    eventRect.channelEvent = channelEvent;
                    _eventRects[rechannelize(channelEvent->channel())].push_back(eventRect);
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_MIXER_BALANCE_CHANGE:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == MixerBalanceControllerEventsMode)) {
                    float value = static_cast<float>(channelEvent->param1());
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth - 5.0f, ((channelEvent->param2() == 0) ? normalize(value, 0.0f, 128.0f) : normalize(value, -100.0f, 100.0f)) * height - 5.0f + margin, 10.0f, 10.0f);
                    EventRect eventRect;
                    eventRect.rect = r;
                    eventRect.channelEvent = channelEvent;
                    _eventRects[rechannelize(channelEvent->channel())].push_back(eventRect);
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_CONTROL_CHANGE:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == ControlChangeControllerEventsMode)) {
                    if (channelEvent->param1() == _controlChange) {
                        float value = static_cast<float>(channelEvent->param2());
                        MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth - 5.0f, normalize(value, 0.0f, 128.0f) * height - 5.0f + margin, 10.0f, 10.0f);
                        EventRect eventRect;
                        eventRect.rect = r;
                        eventRect.channelEvent = channelEvent;
                        _eventRects[rechannelize(channelEvent->channel())].push_back(eventRect);
                    }
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_KEY_AFTERTOUCH:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == KeyAftertouchControllerEventsMode)) {
                    float value = static_cast<float>(channelEvent->param2());
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth - 5.0f, normalize(value, 0.0f, 128.0f) * height - 5.0f + margin, 10.0f, 10.0f);
                    EventRect eventRect;
                    eventRect.rect = r;
                    eventRect.channelEvent = channelEvent;
                    _eventRects[rechannelize(channelEvent->channel())].push_back(eventRect);
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_CHANNEL_AFTERTOUCH:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == ChannelAftertouchControllerEventsMode)) {
                    float value = static_cast<float>(channelEvent->param1());
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth - 5.0f, normalize(value, 0.0f, 128.0f) * height - 5.0f + margin, 10.0f, 10.0f);
                    EventRect eventRect;
                    eventRect.rect = r;
                    eventRect.channelEvent = channelEvent;
                    _eventRects[rechannelize(channelEvent->channel())].push_back(eventRect);
                }
                break;
            }

        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsView::updateSelectedEvents(int trackIndex, bool areCombined, bool areChannelEventsSkipped)
{
    if (!areCombined)
        _selectedEvents.clear();
    
    if (!_sequenceInfosFn || !_nbEventsFn || !_eventAtIndexFn || (trackIndex < 0))
        return;
    
    unsigned int nbEvents = _nbEventsFn(this, trackIndex);
    unsigned int currentTickCount = 0;
    
    int timeDivision = 480;
    std::vector<unsigned int> totalNbTicks, eotTickCounts;
    bool areAbsTicks = true;
    
    // Get sequence infos
    _sequenceInfosFn(this, &timeDivision, &totalNbTicks, &eotTickCounts, &areAbsTicks);
    
    if ((totalNbTicks.size() == 0) || (totalNbTicks[trackIndex] > PIANO_ROLL_EVENTS_VIEW_MAX_NB_TICKS))
        return;
    
    const float margin = 10.0f;
    float height = bounds().size.height - (2.0f * margin);
    
    // Draw the events
    for (unsigned int i = 0; i < nbEvents; i++) {
        std::shared_ptr<MelobaseCore::Event> event;
        _eventAtIndexFn(this, trackIndex, i, &event);
        auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(event);
        if (areAbsTicks) {
            currentTickCount = channelEvent->tickCount();
        } else {
            currentTickCount += channelEvent->tickCount();
        }
        
        if (areChannelEventsSkipped && (channelEvent->type() != CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE) && (channelEvent->type() != CHANNEL_EVENT_TYPE_META_SET_TEMPO))
            continue;
        
        switch (channelEvent->type()) {
            case CHANNEL_EVENT_TYPE_NOTE:
            {
                if (!_isShowingControllerEvents) {
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth, (channelEvent->param1() - 12) * _eventHeight, channelEvent->length() * _eventTickWidth, _eventHeight);
                    bool selected = _visibleChannels[rechannelize(channelEvent->channel())] && isRectInRect(r, _selectionRect);
                    if (selected) {
                        _selectedEvents.push_back(event);
                    }
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE:
            {
                if (_isShowingControllerEvents && ((_controllerEventsMode == EndOfTrackTimeSignatureControllerEventsMode) || (_controllerEventsMode == MetaControllerEventsMode && _metaType == 88))) {
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth, 0.0f, 40.0f, bounds().size.height);
                    bool selected = isRectInRect(r, _selectionRect);
                    if (selected) {
                        _selectedEvents.push_back(event);
                    }
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_SUSTAIN:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == SustainControllerEventsMode)) {
                    float value = static_cast<float>(channelEvent->param1());
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth - 5.0f, normalize(value, 0.0f, 128.0f) * height - 5.0f + margin, 10.0f, 10.0f);
                    bool selected = _visibleChannels[rechannelize(channelEvent->channel())] && isRectInRect(r, _selectionRect);
                    if (selected) {
                        _selectedEvents.push_back(event);
                    }
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_PROGRAM_CHANGE:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == ProgramChangesControllerEventsMode)) {
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth, 0.0f, 20.0f, bounds().size.height);
                    bool selected = _visibleChannels[rechannelize(channelEvent->channel())] && isRectInRect(r, _selectionRect);
                    if (selected) {
                        _selectedEvents.push_back(event);
                    }
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_SYSTEM_EXCLUSIVE:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == SysexControllerEventsMode)) {
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth, 0.0f, 20.0f, bounds().size.height);
                    bool selected = _visibleChannels[rechannelize(channelEvent->channel())] && isRectInRect(r, _selectionRect);
                    if (selected) {
                        _selectedEvents.push_back(event);
                    }
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_META_GENERIC:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == MetaControllerEventsMode)) {
                    if (channelEvent->param1() == _metaType) {
                        MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth, 0.0f, 20.0f, bounds().size.height);
                        bool selected = isRectInRect(r, _selectionRect);
                        if (selected) {
                            _selectedEvents.push_back(event);
                        }
                    }
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_META_SET_TEMPO:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == TempoControllerEventsMode)) {
                    float bpm = 60000000.0f / (float)(channelEvent->param1());
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth - 5.0f, normalize(bpm, 30.0f, 300.0f) * height - 5.0f + margin, 10.0f, 10.0f);
                    bool selected = isRectInRect(r, _selectionRect);
                    if (selected) {
                        _selectedEvents.push_back(event);
                    }
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_PITCH_BEND:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == PitchBendControllerEventsMode)) {
                    Float32 multiplier = channelEvent->param2() > 0 ? (Float32)channelEvent->param2() : 1.0f;
                    float value = multiplier * (((Float32)channelEvent->param1() - 0.5f) * 2.0f / 16383.0f - 1.0f);
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth - 5.0f, normalize(value, -2.0f, 2.0f) * height - 5.0f + margin, 10.0f, 10.0f);
                    bool selected = _visibleChannels[rechannelize(channelEvent->channel())] && isRectInRect(r, _selectionRect);
                    if (selected) {
                        _selectedEvents.push_back(event);
                    }
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_MODULATION:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == ModulationControllerEventsMode)) {
                    float value = static_cast<float>(channelEvent->param1());
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth - 5.0f, normalize(value, 0.0f, 128.0f) * height - 5.0f + margin, 10.0f, 10.0f);
                    bool selected = _visibleChannels[rechannelize(channelEvent->channel())] && isRectInRect(r, _selectionRect);
                    if (selected) {
                        _selectedEvents.push_back(event);
                    }
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_META_END_OF_TRACK:
            {
                if (_isShowingControllerEvents && ((_controllerEventsMode == EndOfTrackTimeSignatureControllerEventsMode) || (_controllerEventsMode == MetaControllerEventsMode && _metaType == 47))) {
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth - 20.0f, 0.0f, 20.0f, bounds().size.height);
                    bool selected = isRectInRect(r, _selectionRect);
                    if (selected) {
                        _selectedEvents.push_back(event);
                    }
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_MIXER_LEVEL_CHANGE:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == MixerLevelControllerEventsMode)) {
                    float value = static_cast<float>(channelEvent->param1());
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth - 5.0f, normalize(value, 0.0f, (channelEvent->param2() == 0) ? 128.0f : 100.0f) * height - 5.0f + margin, 10.0f, 10.0f);
                    bool selected = _visibleChannels[rechannelize(channelEvent->channel())] && isRectInRect(r, _selectionRect);
                    if (selected) {
                        _selectedEvents.push_back(event);
                    }
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_MIXER_BALANCE_CHANGE:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == MixerBalanceControllerEventsMode)) {
                    float value = static_cast<float>(channelEvent->param1());
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth - 5.0f, ((channelEvent->param2() == 0) ? normalize(value, 0.0f, 128.0f) : normalize(value, -100.0f, 100.0f)) * height - 5.0f + margin, 10.0f, 10.0f);
                    bool selected = _visibleChannels[rechannelize(channelEvent->channel())] && isRectInRect(r, _selectionRect);
                    if (selected) {
                        _selectedEvents.push_back(event);
                    }
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_CONTROL_CHANGE:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == ControlChangeControllerEventsMode)) {
                    if (channelEvent->param1() == _controlChange) {
                        float value = static_cast<float>(channelEvent->param2());
                        MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth - 5.0f, normalize(value, 0.0f, 128.0f) * height - 5.0f + margin, 10.0f, 10.0f);
                        bool selected = _visibleChannels[rechannelize(channelEvent->channel())] && isRectInRect(r, _selectionRect);
                        if (selected) {
                            _selectedEvents.push_back(event);
                        }
                    }
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_KEY_AFTERTOUCH:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == KeyAftertouchControllerEventsMode)) {
                    float value = static_cast<float>(channelEvent->param2());
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth - 5.0f, normalize(value, 0.0f, 128.0f) * height - 5.0f + margin, 10.0f, 10.0f);
                    bool selected = _visibleChannels[rechannelize(channelEvent->channel())] && isRectInRect(r, _selectionRect);
                    if (selected) {
                        _selectedEvents.push_back(event);
                    }
                }
                break;
            }
            case CHANNEL_EVENT_TYPE_CHANNEL_AFTERTOUCH:
            {
                if (_isShowingControllerEvents && (_controllerEventsMode == ChannelAftertouchControllerEventsMode)) {
                    float value = static_cast<float>(channelEvent->param1());
                    MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth - 5.0f, normalize(value, 0.0f, 128.0f) * height - 5.0f + margin, 10.0f, 10.0f);
                    bool selected = _visibleChannels[rechannelize(channelEvent->channel())] && isRectInRect(r, _selectionRect);
                    if (selected) {
                        _selectedEvents.push_back(event);
                    }
                }
                break;
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool PianoRollEventsView::handleEvent(const MDStudio::UIEvent *event)
{
    auto eventType = event->type;
    
    // Forward horizontal scroll events to main scroll view
    if (_mainScrollView && (eventType == MDStudio::SCROLL_UIEVENT) && isPointInRect(event->pt, resolvedClippedRect()))
        _mainScrollView->scroll(event->deltaX, 0.0f);
    
    // Detecting entering and leaving the region
    if (eventType == MDStudio::MOUSE_MOVED_UIEVENT) {
        _isMouseInside = isPointInRect(event->pt, resolvedClippedRect());
    }
    
    if (_isMouseInside) {
        if ((eventType == MDStudio::KEY_UIEVENT) && (event->key == KEY_CONTROL)) {
            if (_setSelectionStateFn)
                _setSelectionStateFn(this, true);
        } else if (!_isCaptured && (eventType == MDStudio::KEY_UP_UIEVENT) && (event->key == KEY_CONTROL)) {
            if (_setSelectionStateFn)
                _setSelectionStateFn(this, false);
            
            responderChain()->setCursorInRect(this, MDStudio::Platform::ArrowCursor, resolvedClippedRect());
        }
    }
    
    if (eventType == MDStudio::RIGHT_MOUSE_DOWN_UIEVENT && isPointInRect(event->pt, resolvedClippedRect())) {
        
        responderChain()->makeFirstResponder(this);
        _hasFocus = true;
        if (_didSetFocusStateFn)
            _didSetFocusStateFn(this, _hasFocus);
        
        if (_setSelectionStateFn)
            _setSelectionStateFn(this, true);
        eventType = MDStudio::MOUSE_DOWN_UIEVENT;

        responderChain()->setCursorInRect(this, MDStudio::Platform::CrosshairCursor, resolvedClippedRect());
    }
    
    // Handle the cursor when selecting
    if (_isMouseInside) {
        if (_mode == SelectionMode) {
            responderChain()->setCursorInRect(this, MDStudio::Platform::CrosshairCursor, resolvedClippedRect());
        } else if (_mode == DrawingMode) {
            responderChain()->setCursorInRect(this, MDStudio::Platform::PencilCursor, resolvedClippedRect());
        }
    }
    
    // Handle the cursor when in arrow, move or resize mode
    if (!_isMovingEvents && (_mode == ArrowMode || _mode == MoveMode || _mode == ResizeMode) && (eventType == MDStudio::MOUSE_MOVED_UIEVENT) && isPointInRect(event->pt, resolvedClippedRect())) {
        MDStudio::Point off = resolvedOffset();
        MDStudio::Point pt = MDStudio::makePoint(event->pt.x - clippedRect().origin.x - off.x, event->pt.y - clippedRect().origin.y - off.y);
        bool isCursorSet = false;
        for (int channel = STUDIO_MAX_CHANNELS - 1; channel >= 0; --channel) {
            for (auto eventRect : _eventRects[channel]) {
                if (isPointInRect(pt, eventRect.rect)) {
                    if (((_mode == MoveMode || _mode == ResizeMode) && _selectedEvents.empty()) || std::find(_selectedEvents.begin(), _selectedEvents.end(), eventRect.channelEvent) != _selectedEvents.end()) {
                        if ((_mode == ArrowMode || _mode == ResizeMode) && (eventRect.channelEvent->type() == CHANNEL_EVENT_TYPE_NOTE) && ((_mode == ResizeMode) || ((eventRect.rect.size.width > 2 * PIANO_ROLL_EVENTS_VIEW_NB_RESIZE_HANDLE_WIDTH) && (pt.x > eventRect.rect.origin.x + eventRect.rect.size.width - PIANO_ROLL_EVENTS_VIEW_NB_RESIZE_HANDLE_WIDTH)))) {
                            responderChain()->setCursorInRect(this, MDStudio::Platform::ResizeLeftRightCursor, resolvedClippedRect());
                            isCursorSet = true;
                        } else {
                            if (_mode != ResizeMode) {
                                responderChain()->setCursorInRect(this, MDStudio::Platform::OpenHandCursor, resolvedClippedRect());
                                isCursorSet = true;
                            }
                        }
                    }
                }
            }
        } // for each channel
        
        if (!isCursorSet)
            responderChain()->setCursorInRect(this, (_mode == SelectionMode || _mode == DrawingMode) ? MDStudio::Platform::CrosshairCursor : MDStudio::Platform::ArrowCursor, resolvedClippedRect());
            
    }
    
    if (_hasFocus && eventType == MDStudio::KEY_UIEVENT) {
        switch (event->key) {
            case KEY_LEFT:
                if (_previousMeasureFn) {
                    _previousMeasureFn(this);
                    return true;
                }
            case KEY_RIGHT:
                if (_nextMeasureFn) {
                    _nextMeasureFn(this);
                    return true;
                }
            case KEY_DELETE:
            case KEY_BACKSPACE:
                if (_deleteEventsFn) {
                    _deleteEventsFn(this);
                    return true;
                }
        }
    } else if ((eventType == MDStudio::MOUSE_DOWN_UIEVENT  || eventType == MDStudio::MOUSE_MOVED_UIEVENT) && (_isCaptured || (isPointInRect(event->pt, resolvedClippedRect()) && eventType == MDStudio::MOUSE_DOWN_UIEVENT))) {
        
        MDStudio::Point off = resolvedOffset();
        
        MDStudio::Point pt = MDStudio::makePoint(event->pt.x - clippedRect().origin.x - off.x, event->pt.y - clippedRect().origin.y - off.y);
        
        float p = (event->pt.x - clippedRect().origin.x - off.x) / _eventTickWidth;
        int pitch = pt.y / _eventHeight;
        if (p < 0.0f)
            p = 0.0f;
        if (!_isCaptured && (eventType == MDStudio::MOUSE_DOWN_UIEVENT)) {
            responderChain()->makeFirstResponder(this);
            responderChain()->captureResponder(this);
            _hasFocus = true;
            _isCaptured = true;
            if (_didSetFocusStateFn)
                _didSetFocusStateFn(this, _hasFocus);
            
            if (_mode == DrawingMode) {
                
                if (!_isShowingControllerEvents) {
                    _moveEventsRefPos = pt;
                    _moveEventsRefTickPos = p;
                    _moveEventsRefPitch = pitch;
                    if (_addNoteEventFn)
                        _addNoteEventFn(this, static_cast<unsigned int>(p), pitch + 12);
                } else {
                    // Controller events are shown in this piano roll

                    const float margin = 10.0f;
                    float height = bounds().size.height - (2.0f * margin);
                    
                    float v = pt.y - margin;
                    if (v < 0.0f) {
                        v = 0.0f;
                    } else if (v > height) {
                        v = height;
                    }
                    
                    
                    float normalizedValue = normalize(v, 0.0f, height);

                    switch (_controllerEventsMode) {
                        case SustainControllerEventsMode:
                            if (_addSustainEventFn)
                                _addSustainEventFn(this, static_cast<unsigned int>(p), normalizedValue * 127);
                            break;
                            
                        case ProgramChangesControllerEventsMode:
                            if (_addProgramChangeEventFn)
                                _addProgramChangeEventFn(this, static_cast<unsigned int>(p));
                            break;
                            
                        case SysexControllerEventsMode:
                            if (_addSysexEventFn)
                                _addSysexEventFn(this, static_cast<unsigned int>(p));
                            break;
                        
                        case MetaControllerEventsMode:
                            // If TS event
                            if (_metaType == 88) {
                                if (_addTimeSignatureEventFn)
                                    _addTimeSignatureEventFn(this, static_cast<unsigned int>(p));
                                
                            } else if (_addMetaEventFn) {
                                // Do not allow adding EOT meta event
                                if (_metaType != 47) {
                                    _addMetaEventFn(this, static_cast<unsigned int>(p), _metaType);
                                } else {
                                    MDStudio::Platform::sharedInstance()->beep();
                                }
                            }
                            break;
                            
                        case TempoControllerEventsMode:
                            if (_addTempoEventFn)
                                _addTempoEventFn(this, static_cast<unsigned int>(p), 30.0f + normalizedValue * 270);
                            break;
                            
                        case PitchBendControllerEventsMode:
                            if (_addPitchBendEventFn)
                                _addPitchBendEventFn(this, static_cast<unsigned int>(p), normalizedValue * 16384 - 8192);
                            break;
                            
                        case ModulationControllerEventsMode:
                            if (_addModulationEventFn)
                                _addModulationEventFn(this, static_cast<unsigned int>(p), normalizedValue * 127);
                            break;
                            
                        case MixerLevelControllerEventsMode:
                            if (_addMixerLevelEventFn)
                                _addMixerLevelEventFn(this, static_cast<unsigned int>(p), normalizedValue * 127);
                            break;
                            
                        case MixerBalanceControllerEventsMode:
                            if (_addMixerBalanceEventFn)
                                _addMixerBalanceEventFn(this, static_cast<unsigned int>(p), normalizedValue * 127);
                            break;
                            
                        case EndOfTrackTimeSignatureControllerEventsMode:
                            if (_addTimeSignatureEventFn)
                                _addTimeSignatureEventFn(this, static_cast<unsigned int>(p));
                            break;
                        
                        case ControlChangeControllerEventsMode:
                            if (_addControlChangeEventFn)
                                _addControlChangeEventFn(this, static_cast<unsigned int>(p), _controlChange, normalizedValue * 127);
                            break;
                            
                        case KeyAftertouchControllerEventsMode:
                            if (_addKeyAftertouchEventFn)
                                _addKeyAftertouchEventFn(this, static_cast<unsigned int>(p), 0, normalizedValue * 127);
                            break;
                            
                        case ChannelAftertouchControllerEventsMode:
                            if (_addChannelAftertouchEventFn)
                                _addChannelAftertouchEventFn(this, static_cast<unsigned int>(p), normalizedValue * 127);
                            break;
                            
                        default:
                            break;
                    }  // switch
                } // controller events are shown in this piano roll
                
                
            } else if (_mode == SelectionMode) {
                _selectionRect.origin = pt;
                
                if (_willSetSelectionRegionFn)
                    _willSetSelectionRegionFn(this);
                
            } else if (!_isMovingEvents) {
                // Check if we clicked inside a selected event
                for (int channel = STUDIO_MAX_CHANNELS - 1; channel >= 0; --channel)
                    for (auto eventRect : _eventRects[channel]) {
                        if (isPointInRect(pt, eventRect.rect)) {
                            if (((_mode == MoveMode || _mode == ResizeMode) && _selectedEvents.empty()) || std::find(_selectedEvents.begin(), _selectedEvents.end(), eventRect.channelEvent) != _selectedEvents.end()) {
                                if ((_mode != MoveMode) && ((_mode == ResizeMode) || ((eventRect.channelEvent->type() == CHANNEL_EVENT_TYPE_NOTE) && eventRect.rect.size.width > 2 * PIANO_ROLL_EVENTS_VIEW_NB_RESIZE_HANDLE_WIDTH))) {
                                    _isResizingEvents = (_mode == ResizeMode) || (pt.x > eventRect.rect.origin.x + eventRect.rect.size.width - PIANO_ROLL_EVENTS_VIEW_NB_RESIZE_HANDLE_WIDTH);
                                } else {
                                    _isResizingEvents = false;
                                }
                                _moveEventsRefPos = pt;
                                _moveEventsRefTickPos = p;
                                _moveEventsRefPitch = pitch;
                                
                                if (!_isMovingEvents) {
                                    _isMovingEvents = true;
                                    _activeEvent = eventRect.channelEvent;
                                    if (_willMoveEventsFn)
                                        _willMoveEventsFn(this);
                                }
                                break;
                            }
                        }
                    }
            }
        }

        if (_mode == SelectionMode) {
            _selectionRect.size = MDStudio::makeSize(pt.x - _selectionRect.origin.x, pt.y - _selectionRect.origin.y);
            _lastSelectionPt = pt;
            setDirty();
            if (_didSetSelectionRegionFn)
                _didSetSelectionRegionFn(this);
        } else if (_isMovingEvents) {
            _lastSelectionPt = pt;
            if (_moveEventsFn) {
                
                const float margin = 10.0f;
                float height = bounds().size.height - (2.0f * margin);
                
                int deltaTicks = p - _moveEventsRefTickPos;
                int deltaPitch = !_isShowingControllerEvents ? pitch - _moveEventsRefPitch : 0;
                if ((deltaTicks != 0) || (deltaPitch != 0)) {
                    float relY = pt.y - _moveEventsRefPos.y;
                    float normalizedDeltaValue = normalize(relY, 0.0f, height);
                    int deltaValue = 0;
                    
                    if (_isShowingControllerEvents) {
                        switch (_controllerEventsMode) {
                            case SustainControllerEventsMode:
                                deltaValue = normalizedDeltaValue * 127;
                                break;
                                
                            case TempoControllerEventsMode:
                                deltaValue = normalizedDeltaValue * 300;
                                break;
                                
                            case PitchBendControllerEventsMode:
                                deltaValue = normalizedDeltaValue * 16384;
                                break;
                                
                            case ModulationControllerEventsMode:
                                deltaValue = normalizedDeltaValue * 127;
                                break;

                            case MixerLevelControllerEventsMode:
                                deltaValue = normalizedDeltaValue * 127;
                                break;
                                
                            case MixerBalanceControllerEventsMode:
                                deltaValue = normalizedDeltaValue * 127;
                                break;
                                
                            case ControlChangeControllerEventsMode:
                                deltaValue = normalizedDeltaValue * 127;
                                break;
                                
                            case KeyAftertouchControllerEventsMode:
                                deltaValue = normalizedDeltaValue * 127;
                                break;
                                
                            case ChannelAftertouchControllerEventsMode:
                                deltaValue = normalizedDeltaValue * 127;
                                break;
                                
                            default:
                                break;
                        }
                    }

                    _moveEventsFn(this, deltaTicks, deltaPitch, deltaValue, _isResizingEvents, _mode == DrawingMode);
                }
            }
        } else if (_mode != DrawingMode) {
            _isMovingCursor = true;
            setCursorTickPos(p);
        }
        return true;
    } else if (_isCaptured && ((eventType == MDStudio::MOUSE_UP_UIEVENT) || (eventType == MDStudio::RIGHT_MOUSE_UP_UIEVENT))) {
        responderChain()->releaseResponder(this);
        _isCaptured = false;
        
        if (_mode == SelectionMode) {
            bool areCombined = event->modifierFlags & MODIFIER_FLAG_SHIFT;
            updateSelectedEvents(_trackIndex, areCombined, false);
            if (_trackIndex > 0)
                updateSelectedEvents(0, true, true);
            if (_didSelectEventsFn)
                _didSelectEventsFn(this, areCombined, false);
        }
        
        if (_isMovingEvents) {
            _activeEvent = nullptr;
            _isMovingEvents = false;
            if (_didMoveEventsFn)
                _didMoveEventsFn(this);
        }
        
        if (eventType == MDStudio::RIGHT_MOUSE_UP_UIEVENT) {
            if (_setSelectionStateFn)
                _setSelectionStateFn(this, false);
            responderChain()->setCursorInRect(this, MDStudio::Platform::ArrowCursor, resolvedClippedRect());
        }
        
        if (_isMovingCursor) {
            _isMovingCursor = false;
            if (_didFinishSettingCursorTickPosFn)
                _didFinishSettingCursorTickPosFn(this);
        }
        
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsView::resignFirstResponder()
{
    if (_hasFocus) {
        if (_isCaptured) {
            responderChain()->releaseResponder(this);
            _isCaptured = false;
        }
        _hasFocus = false;
        if (_didSetFocusStateFn)
            _didSetFocusStateFn(this, _hasFocus);

    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsView::drawFilledRoundRectTopRight(MDStudio::Rect rect, float radius, MDStudio::Color fillColor)
{
    MDStudio::DrawContext *dc = drawContext();
    
    float lineWidth = 1.0f / 2.0f;

    dc->pushStates();
    dc->setFillColor(fillColor);
    
    // Top-right
    dc->drawArc(MDStudio::makePoint(rect.origin.x + rect.size.width - radius, rect.origin.y + rect.size.height - radius), radius - lineWidth, 0.0f , M_PI / 2.0f);
    
    // Right
    dc->drawRect(MDStudio::makeRect(rect.origin.x + rect.size.width - radius, rect.origin.y, radius, rect.size.height - radius));
    
    // Center
    dc->drawRect(MDStudio::makeRect(rect.origin.x, rect.origin.y, rect.size.width - radius, rect.size.height));
    
    dc->popStates();
    
    dc->pushStates();
    dc->setStrokeColor(MDStudio::whiteColor);
    
    // Top-right
    dc->drawArc(MDStudio::makePoint(rect.origin.x + rect.size.width - radius, rect.origin.y + rect.size.height - radius), radius - lineWidth, 0.0f , M_PI / 2.0f);
    
    // Left
    dc->drawLine(MDStudio::makePoint(rect.origin.x + lineWidth, rect.origin.y), MDStudio::makePoint(rect.origin.x + lineWidth, rect.origin.y + rect.size.height));
    
    // Top
    dc->drawLine(MDStudio::makePoint(rect.origin.x, rect.origin.y + rect.size.height - lineWidth), MDStudio::makePoint(rect.origin.x + rect.size.width - radius, rect.origin.y + rect.size.height - lineWidth));
    
    // Right
    dc->drawLine(MDStudio::makePoint(rect.origin.x + rect.size.width - lineWidth, rect.origin.y), MDStudio::makePoint(rect.origin.x + rect.size.width - lineWidth, rect.origin.y + rect.size.height - radius));
    
    // Bottom
    dc->drawLine(MDStudio::makePoint(rect.origin.x, rect.origin.y + lineWidth), MDStudio::makePoint(rect.origin.x + rect.size.width, rect.origin.y + lineWidth));
    
    dc->popStates();
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsView::drawFilledRoundRectTopLeft(MDStudio::Rect rect, float radius, MDStudio::Color fillColor)
{
    MDStudio::DrawContext *dc = drawContext();

    float lineWidth = 1.0f / 2.0f;

    dc->pushStates();
    dc->setFillColor(fillColor);

    // Top-left
    dc->drawArc(MDStudio::makePoint(rect.origin.x + radius, rect.origin.y + rect.size.height - radius), radius - lineWidth, M_PI / 2.0f , M_PI / 2.0f);
    
    // Left
    dc->drawRect(MDStudio::makeRect(rect.origin.x, rect.origin.y, radius, rect.size.height - radius));
    
    // Center
    dc->drawRect(MDStudio::makeRect(rect.origin.x + radius, rect.origin.y, rect.size.width - radius, rect.size.height));
    
    dc->popStates();
    
    dc->pushStates();
    dc->setStrokeColor(MDStudio::whiteColor);

    // Top-left
    dc->drawArc(MDStudio::makePoint(rect.origin.x + radius, rect.origin.y + rect.size.height - radius), radius - lineWidth, M_PI / 2.0f , M_PI / 2.0f);

    // Left
    dc->drawLine(MDStudio::makePoint(rect.origin.x + lineWidth, rect.origin.y), MDStudio::makePoint(rect.origin.x + lineWidth, rect.origin.y + rect.size.height - radius));
    
    // Top
    dc->drawLine(MDStudio::makePoint(rect.origin.x + radius, rect.origin.y + rect.size.height - lineWidth), MDStudio::makePoint(rect.origin.x + rect.size.width, rect.origin.y + rect.size.height - lineWidth));
    
    // Right
    dc->drawLine(MDStudio::makePoint(rect.origin.x + rect.size.width - lineWidth, rect.origin.y), MDStudio::makePoint(rect.origin.x + rect.size.width - lineWidth, rect.origin.y + rect.size.height));
    
    // Bottom
    dc->drawLine(MDStudio::makePoint(rect.origin.x, rect.origin.y + lineWidth), MDStudio::makePoint(rect.origin.x + rect.size.width, rect.origin.y + lineWidth));
    
    dc->popStates();
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsView::getMeasureTicks(int timeDivision, unsigned int totalNbTicks, bool areAbsTicks, std::vector<unsigned int> *measureTicks, std::vector<int> *numerators)
{
    unsigned int tick = 0L, refTick = 0L;
    int numerator = 0;
    
    unsigned int nbEvents = _nbEventsFn(this, 0);
    
    
    for (unsigned int i = 0; i < nbEvents; ++i) {
        std::shared_ptr<MelobaseCore::Event> event;
        _eventAtIndexFn(this, 0, i, &event);
        auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(event);
        if (areAbsTicks) {
            tick = channelEvent->tickCount();
        } else {
            tick += channelEvent->tickCount();
        }
        if (channelEvent->type() == CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE) {
            if (numerator) {
                for (unsigned int t = refTick; t < tick; t += timeDivision * numerator) {
                    measureTicks->push_back(t);
                    numerators->push_back(numerator);
                }
            }
            numerator = channelEvent->param1();
            refTick = tick;
        }
    }
    
    if (numerator) {
        for (unsigned int t = refTick; t < totalNbTicks; t += timeDivision * numerator) {
            measureTicks->push_back(t);
            numerators->push_back(numerator);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
MDStudio::Rect PianoRollEventsView::makeBoundingBox(const std::vector<MDStudio::Point> &points) const
{
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::min();
    float maxY = std::numeric_limits<float>::max();
    
    for (auto &pt : points) {
        minX = std::min(minX, pt.x);
        minY = std::min(minY, pt.y);
        maxX = std::max(maxX, pt.x);
        maxY = std::max(maxY, pt.y);
    }
    
    return MDStudio::makeRect(minX, minY, maxX - minX, maxY - minY);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsView::drawAnnotations() {
    MDStudio::DrawContext* dc = drawContext();

    dc->pushStates();
    dc->setFillColor(MDStudio::redColor);
    auto nbAnnotations = _nbAnnotationsFn(this);
    for (unsigned int annotationIndex = 0; annotationIndex < nbAnnotations; annotationIndex++) {
        std::shared_ptr<MelobaseCore::SequenceAnnotation> annotation;
        _annotationAtIndexFn(this, annotationIndex, &annotation);
        dc->drawRect(MDStudio::makeRect(annotation->tickCount * _eventTickWidth, 0.0f, 1.0f, frame().size.height));
    }
    dc->popStates();
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsView::draw()
{
    if (!_sequenceInfosFn || !_nbEventsFn || !_eventAtIndexFn || (_trackIndex < 0))
        return;
    
    MDStudio::DrawContext *dc = drawContext();

    int timeDivision = 480;
    std::vector<unsigned int> totalNbTicks, eotTickCounts;
    bool areAbsTicks = true;

    // Get sequence infos
    _sequenceInfosFn(this, &timeDivision, &totalNbTicks, &eotTickCounts, &areAbsTicks);

    if ((totalNbTicks.size() == 0) || (timeDivision == 0))
        return;
    
    MDStudio::Rect fullRect = MDStudio::makeRect(0.0f, 0.0f, (float)(totalNbTicks[_trackIndex]) * _eventTickWidth, bounds().size.height);
    
    if (totalNbTicks[_trackIndex] <= PIANO_ROLL_EVENTS_VIEW_MAX_NB_TICKS) {
        
        std::vector<unsigned int>measureTicks;
        std::vector<int>numerators;
        
        getMeasureTicks(timeDivision, totalNbTicks[_trackIndex], areAbsTicks, &measureTicks, &numerators);
        
        dc->pushStates();
        dc->setFillColor(_isShowingControllerEvents ? MDStudio::makeColor(0.1f, 0.1f, 0.1f, 1.0f) : MDStudio::makeColor(0.2f, 0.2f, 0.2f, 1.0f));
        dc->drawRect(MDStudio::makeRect(0, 0, totalNbTicks[_trackIndex] * _eventTickWidth, bounds().size.height));
        dc->popStates();
        
        // Draw the measures
        unsigned long nbMeasures = measureTicks.size();
        
        for (unsigned int measure = 0; measure < nbMeasures; measure++) {
            
            unsigned int nbTicksInMeasure = (measure == (nbMeasures - 1) ? totalNbTicks[_trackIndex] : measureTicks[measure + 1]) - measureTicks[measure];
            
            MDStudio::Rect r = MDStudio::makeRect((float)(measureTicks[measure]) * _eventTickWidth, 2, nbTicksInMeasure * _eventTickWidth, bounds().size.height);
            if (!isRectInRect(MDStudio::makeRect(r.origin.x + offset().x, r.origin.y + offset().y, r.size.width, r.size.height), clippedBounds()))
                continue;
            
            dc->pushStates();
            dc->setFillColor(_isShowingControllerEvents ? MDStudio::makeColor(0.1f, 0.1f, 0.1f, 1.0f) : MDStudio::veryDimGrayColor);
            dc->drawRect(r);
            dc->popStates();
            
            if (measure) {
                dc->pushStates();
                dc->setFillColor( _isShowingControllerEvents ? MDStudio::veryDimGrayColor : MDStudio::grayColor);
                dc->drawRect(MDStudio::makeRect(r.origin.x, 2, 2, bounds().size.height));
                dc->popStates();
            }
            
            int numerator = numerators[measure];
            MDStudio::Color veryDarkGray = MDStudio::makeColor(0.15f, 0.15f, 0.15f, 1.0f);


                if (_eventTickWidth > 0.2) {
                    // 1/32
                    for (int num = (measure > 0) ? 0 : 1; num < numerator * 8; num++) {
                        MDStudio::Rect r2 = MDStudio::makeRect((float)(num * timeDivision / 8) * _eventTickWidth + r.origin.x, 2, 1, bounds().size.height);
                        if (r2.origin.x + r2.size.width < fullRect.size.width) {
                            dc->pushStates();
                            dc->setFillColor((num % 4 == 0) ?  MDStudio::blackColor : veryDarkGray);
                            dc->drawRect(r2);
                            dc->popStates();
                        }
                    }
                } else if (_eventTickWidth > 0.07) {
                    // 1/16
                    for (int num = (measure > 0) ? 0 : 1; num < numerator * 4; num++) {
                        MDStudio::Rect r2 = MDStudio::makeRect((float)(num * timeDivision / 4) * _eventTickWidth + r.origin.x, 2, 1, bounds().size.height);
                        if (r2.origin.x + r2.size.width < fullRect.size.width) {
                            dc->pushStates();
                            dc->setFillColor(veryDarkGray);
                            dc->drawRect(r2);
                            dc->popStates();
                        }
                    }
                } else if (_eventTickWidth > 0.04) {
                    // 1/8
                    for (int num = (measure > 0) ? 0 : 1; num < numerator * 2; num++) {
                        MDStudio::Rect r2 = MDStudio::makeRect((float)(num * timeDivision / 2) * _eventTickWidth + r.origin.x, 2, 1, bounds().size.height);
                        if (r2.origin.x + r2.size.width < fullRect.size.width) {
                            dc->pushStates();
                            dc->setFillColor(veryDarkGray);
                            dc->drawRect(r2);
                            dc->popStates();
                        }
                    }
                }

                // 1/4
                for (int num = (measure > 0) ? 0 : 1; num < numerator; num++) {
                    MDStudio::Rect r2 = MDStudio::makeRect((float)(num * timeDivision) * _eventTickWidth + r.origin.x, 2, 1, bounds().size.height);
                    if (r2.origin.x + r2.size.width < fullRect.size.width) {
                        dc->pushStates();
                        dc->setFillColor(_isShowingControllerEvents ? MDStudio::veryDimGrayColor : MDStudio::grayColor);
                        dc->drawRect(r2);
                        dc->popStates();
                    }
                }
        } // for each measure
        
        // Draw the events
        updateEventRects(_trackIndex, false, false);
        if (_trackIndex > 0)
            updateEventRects(0, true, true);
        
        EventRect lastControllerEventRect[STUDIO_MAX_CHANNELS];
        memset(&lastControllerEventRect, 0, sizeof(lastControllerEventRect));

        // Draw median line
        if (_isShowingControllerEvents && (_controllerEventsMode == SustainControllerEventsMode || _controllerEventsMode == PitchBendControllerEventsMode || _controllerEventsMode == ModulationControllerEventsMode || _controllerEventsMode == MixerBalanceControllerEventsMode)) {
            dc->pushStates();
            dc->setStrokeColor(MDStudio::veryDimGrayColor);
            dc->drawLine(MDStudio::makePoint(0.0f, rect().size.height / 2.0f), MDStudio::makePoint(fullRect.size.width , rect().size.height / 2.0f));
            dc->popStates();
        }
        
        for (int channel = STUDIO_MAX_CHANNELS - 1; channel >= 0; --channel) {
            
            // Draw controller lines
            MDStudio::Color channelColor = channelColors[channel];
            channelColor.red *= 0.75f;
            channelColor.green *= 0.75f;
            channelColor.blue *= 0.75f;
            
            for (auto eventRect : _eventRects[channel]) {
                
                bool isMetaEvent = getIsMetaEvent(eventRect.channelEvent);
                
                if (isMetaEvent || _visibleChannels[rechannelize(eventRect.channelEvent->channel())]) {
                    
                    MDStudio::Color eventColor = isMetaEvent ? MDStudio::grayColor : channelColor;
                    
                    if ((eventRect.channelEvent->type() == CHANNEL_EVENT_TYPE_SUSTAIN) || (eventRect.channelEvent->type() == CHANNEL_EVENT_TYPE_META_SET_TEMPO) || (eventRect.channelEvent->type() == CHANNEL_EVENT_TYPE_PITCH_BEND) || (eventRect.channelEvent->type() == CHANNEL_EVENT_TYPE_MODULATION) || (eventRect.channelEvent->type() == CHANNEL_EVENT_TYPE_MIXER_LEVEL_CHANGE) || (eventRect.channelEvent->type() == CHANNEL_EVENT_TYPE_MIXER_BALANCE_CHANGE) || (eventRect.channelEvent->type() == CHANNEL_EVENT_TYPE_CONTROL_CHANGE) || (eventRect.channelEvent->type() == CHANNEL_EVENT_TYPE_KEY_AFTERTOUCH) || (eventRect.channelEvent->type() == CHANNEL_EVENT_TYPE_CHANNEL_AFTERTOUCH)) {
                        if (lastControllerEventRect[rechannelize(eventRect.channelEvent->channel())].channelEvent) {
                            auto pt0 = midRect(lastControllerEventRect[rechannelize(eventRect.channelEvent->channel())].rect);
                            auto pt1 = MDStudio::makePoint(midRectX(eventRect.rect), midRectY(lastControllerEventRect[rechannelize(eventRect.channelEvent->channel())].rect));
                            auto pt2 = midRect(eventRect.rect);
                            
                            auto boundingBox = makeBoundingBox({pt0, pt1, pt2});
                            
                            if (isRectInRect(MDStudio::makeRect(boundingBox.origin.x + offset().x, boundingBox.origin.y + offset().y, boundingBox.size.width, boundingBox.size.height), clippedBounds())) {
                                dc->pushStates();
                                dc->setStrokeColor(eventColor);
                                dc->setStrokeWidth(2.0f);
                                dc->drawLine(pt0, pt1);
                                dc->drawLine(pt1, pt2);
                                dc->popStates();
                            }
                        }
                        lastControllerEventRect[rechannelize(eventRect.channelEvent->channel())] = eventRect;
                    }
                }
            }
            
            // Draw the completion line
            if (lastControllerEventRect[channel].channelEvent) {
                auto eventRect = lastControllerEventRect[channel];
                bool isMetaEvent = getIsMetaEvent(eventRect.channelEvent);
                MDStudio::Color eventColor = isMetaEvent ? MDStudio::grayColor : channelColor;
                auto startPoint = midRect(lastControllerEventRect[channel].rect);
                if (fullRect.size.width > startPoint.x) {
                    auto endPoint = MDStudio::makePoint(fullRect.size.width, midRect(lastControllerEventRect[channel].rect).y);
                    auto boundingBox = makeBoundingBox({startPoint, endPoint});
                    if (isRectInRect(MDStudio::makeRect(boundingBox.origin.x + offset().x, boundingBox.origin.y + offset().y, boundingBox.size.width, boundingBox.size.height), clippedBounds())) {
                        dc->pushStates();
                        dc->setStrokeColor(eventColor);
                        dc->setStrokeWidth(2.0f);
                        dc->drawLine(startPoint, endPoint);
                        dc->popStates();
                    }
                }
            }

            // Draw events
            for (auto eventRect : _eventRects[channel]) {
                
                bool isMetaEvent = getIsMetaEvent(eventRect.channelEvent);
                if (isMetaEvent || _visibleChannels[rechannelize(eventRect.channelEvent->channel())]) {
                    
                    if (isRectInRect(MDStudio::makeRect(eventRect.rect.origin.x + offset().x, eventRect.rect.origin.y + offset().y, eventRect.rect.size.width, eventRect.rect.size.height), clippedBounds())) {
                        MDStudio::Color color = isMetaEvent ? MDStudio::grayColor : channelColor;
                        bool selected = std::find(_selectedEvents.begin(), _selectedEvents.end(), eventRect.channelEvent) != _selectedEvents.end();
                        if (selected)
                            color = isMetaEvent ? MDStudio::lightGrayColor : channelColors[channel];
                        
                        if (eventRect.channelEvent->type() == CHANNEL_EVENT_TYPE_NOTE) {
                            dc->pushStates();
                            dc->setFillColor(color);
                            dc->setStrokeColor(MDStudio::whiteColor);
                            dc->drawRect(eventRect.rect);
                            dc->popStates();
                            
                            // If selected, draw the resize handle
                            if ((_mode == ArrowMode) && selected && (eventRect.rect.size.width > 2.0f * PIANO_ROLL_EVENTS_VIEW_NB_RESIZE_HANDLE_WIDTH)) {
                                MDStudio::Rect r = MDStudio::makeRect(eventRect.rect.origin.x + eventRect.rect.size.width - PIANO_ROLL_EVENTS_VIEW_NB_RESIZE_HANDLE_WIDTH, eventRect.rect.origin.y + 2.0f, PIANO_ROLL_EVENTS_VIEW_NB_RESIZE_HANDLE_WIDTH - 3.0f, eventRect.rect.size.height - 4.0f);
                                dc->pushStates();
                                dc->setFillColor(MDStudio::veryDimGrayColor);
                                dc->drawRect(r);
                                dc->popStates();
                                dc->pushStates();
                                dc->setStrokeColor(MDStudio::whiteColor);
                                dc->drawRect(eventRect.rect);
                                dc->popStates();
                            }
                        } else if (eventRect.channelEvent->type() == CHANNEL_EVENT_TYPE_PROGRAM_CHANGE) {
                            drawFilledRoundRectTopRight(eventRect.rect, 10.0f, color);
                            const MDStudio::Preset *preset = (channel == 9) ? _studio->presetForInstrument(STUDIO_INSTRUMENT_GM_STANDARD_DRUM_KIT) : _studio->presetForInstrument(eventRect.channelEvent->param1());
                            std::string instrumentName;
                            if (preset != nullptr)
                                instrumentName = preset->_name;
                            float textHeight = fontHeight(MDStudio::SystemFonts::sharedInstance()->semiboldFont());
                            dc->pushStates();
                            dc->setStrokeColor(MDStudio::whiteColor);
                            dc->drawText(MDStudio::SystemFonts::sharedInstance()->semiboldFont(), MDStudio::makePoint(eventRect.rect.origin.x + 22.0f - textHeight / 2.0f, 5.0f), instrumentName, 90.0f);
                            dc->popStates();
                        } else if (eventRect.channelEvent->type() == CHANNEL_EVENT_TYPE_SYSTEM_EXCLUSIVE) {
                            drawFilledRoundRectTopRight(eventRect.rect, 10.0f, color);
                            float textHeight = fontHeight(MDStudio::SystemFonts::sharedInstance()->semiboldFont());
                            std::string s;
                            size_t len = eventRect.channelEvent->data().size();
                            if (len > 1) {
                                for (size_t i = 0; i < len - 1; ++i) {
                                    s += toStringHex(eventRect.channelEvent->data()[i]);
                                    if (i < len - 2)
                                        s += " ";
                                }
                            }
                            dc->pushStates();
                            dc->setStrokeColor(MDStudio::whiteColor);
                            dc->drawText(MDStudio::SystemFonts::sharedInstance()->semiboldFont(), MDStudio::makePoint(eventRect.rect.origin.x + 22.0f - textHeight / 2.0f, 5.0f), s, 90.0f);
                            dc->popStates();
                        } else if (eventRect.channelEvent->type() == CHANNEL_EVENT_TYPE_META_GENERIC) {
                            drawFilledRoundRectTopRight(eventRect.rect, 10.0f, color);
                            float textHeight = fontHeight(MDStudio::SystemFonts::sharedInstance()->semiboldFont());
                            std::string s;
                            if (eventRect.channelEvent->param1() >= 1 && eventRect.channelEvent->param1() <= 7) {
                                for (auto c : eventRect.channelEvent->data())
                                    s += c;
                            } else {
                                size_t len = eventRect.channelEvent->data().size();
                                for (size_t i = 0; i < len; ++i) {
                                    s += toStringHex(eventRect.channelEvent->data()[i]);
                                    if (i < len - 1)
                                        s += " ";
                                }
                            }
                            dc->pushStates();
                            dc->setStrokeColor(MDStudio::whiteColor);
                            dc->drawText(MDStudio::SystemFonts::sharedInstance()->semiboldFont(), MDStudio::makePoint(eventRect.rect.origin.x + 22.0f - textHeight / 2.0f, 5.0f), s, 90.0f);
                            dc->popStates();
                        } else if (eventRect.channelEvent->type() == CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE) {
                            drawFilledRoundRectTopRight(eventRect.rect, 5.0f, color);
                            dc->pushStates();
                            dc->setStrokeColor(MDStudio::whiteColor);
                            dc->drawCenteredText(MDStudio::SystemFonts::sharedInstance()->semiboldFont(), eventRect.rect, std::to_string(eventRect.channelEvent->param1()) + "/" + std::to_string(eventRect.channelEvent->param2()));
                            dc->popStates();
                        } else if (eventRect.channelEvent->type() == CHANNEL_EVENT_TYPE_META_END_OF_TRACK) {
                            dc->pushStates();
                            dc->setFillColor(color);
                            drawFilledRoundRectTopLeft(eventRect.rect, 5.0f, color);
                            dc->drawImage(MDStudio::makeCenteredRectInRect(MDStudio::makeRect(eventRect.rect.origin.x + 5.0f, eventRect.rect.origin.y, eventRect.rect.size.width - 5.0f, eventRect.rect.size.height), _endOfTrackImage->size().width, _endOfTrackImage->size().height), _endOfTrackImage);
                            dc->popStates();
                        } else {
                            dc->pushStates();
                            dc->setFillColor(color);
                            dc->setStrokeColor(MDStudio::whiteColor);
                            dc->drawRect(eventRect.rect);
                            dc->popStates();
                        }
                    }
                } // If the channel is visible or if meta event
            }
        }
    } // if we are not beyound the limit number of ticks

    drawAnnotations();
    
    // Draw the end of track grayed-out region
    if (!_isShowingControllerEvents) {
        MDStudio::Color color = MDStudio::makeColor(0.0f, 0.0f, 0.0f, 0.5f);
        float eotOrigin = eotTickCounts[_trackIndex] * _eventTickWidth;
        MDStudio::Rect r = MDStudio::makeRect(eotOrigin, 0.0f, rect().size.width - eotOrigin, rect().size.height);
        dc->pushStates();
        dc->setFillColor(color);
        dc->drawRect(r);
        dc->popStates();
    }
    
    // Draw highlighted pitches
    for (int pitch = 0; pitch < 128; ++pitch) {
        bool isHighlighted = false;
        if (_highlightPitchStates[_highlightChannel][pitch]) {
            isHighlighted = true;
        }
        
        if (isHighlighted) {
            MDStudio::Color color = MDStudio::makeColor(1.0f, 1.0f, 1.0f, 0.3f);
            MDStudio::Rect r = MDStudio::makeRect(-offset().x, static_cast<float>(pitch - 12) * _eventHeight, clippedRect().size.width, _eventHeight);
            dc->pushStates();
            dc->setFillColor(color);
            dc->drawRect(r);
            dc->popStates();
        }
    }
    
    if (totalNbTicks[_trackIndex] > PIANO_ROLL_EVENTS_VIEW_MAX_NB_TICKS) {
        dc->pushStates();
        dc->setFillColor(_isShowingControllerEvents ? MDStudio::makeColor(0.1f, 0.1f, 0.1f, 1.0f) : MDStudio::veryDimGrayColor);
        dc->drawRect(fullRect);
        dc->popStates();
        if (!_isShowingControllerEvents) {
            dc->pushStates();
            dc->setStrokeColor(MDStudio::lightGrayColor);
            dc->drawCenteredText(MDStudio::SystemFonts::sharedInstance()->semiboldFont(), MDStudio::makeRect(-offset().x, -offset().y, clippedRect().size.width, clippedRect().size.height), MDStudio::Platform::sharedInstance()->language() == "fr" ? "La séquence est trop longe pour être affichée" : "The sequence is too long to be displayed.");
            dc->popStates();
        }
    } else {
        // Draw the visible selection rect
        MDStudio::Color color = MDStudio::makeColor(0.5f, 0.5f, 0.5f, 0.25f);
        dc->pushStates();
        dc->setFillColor(color);
        dc->drawRect(_selectionRect);
        dc->popStates();
        color = MDStudio::makeColor(0.75f, 0.75f, 0.75f, 0.25f);
        dc->pushStates();
        dc->setStrokeColor(color);
        dc->drawRect(_selectionRect);
        dc->popStates();
    }
    
    // We draw the cursor
    MDStudio::Rect r = MDStudio::makeRect(_eventTickWidth * (float)_cursorTickPos, 0.0f, pianoRollCursorWidth, bounds().size.height);
    dc->pushStates();
    dc->setFillColor(MDStudio::makeColor(0.0f, 0.5f, 1.0f, 0.6f));
    dc->drawRect(r);
    dc->popStates();
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsView::setCursorTickPos(unsigned int cursorTickPos, bool isDelegateNotified)
{
    if (_trackIndex < 0)
        return;
    
    int timeDivision = 480;

    std::vector<unsigned int> totalNbTicks, eotTickCounts;
    bool areAbsTicks = true;

    // Get sequence infos
    _sequenceInfosFn(this,  &timeDivision, &totalNbTicks, &eotTickCounts, &areAbsTicks);

    unsigned int totalNbTicksCombined = 0;
    if (totalNbTicks.size() > 0)
        totalNbTicksCombined = *std::max_element(totalNbTicks.begin(), totalNbTicks.end());
    
    _cursorTickPos = cursorTickPos < totalNbTicksCombined ? cursorTickPos : totalNbTicksCombined;
    setDirty();
    if (isDelegateNotified && _didSetCursorTickPosFn)
        _didSetCursorTickPosFn(this, _cursorTickPos);
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<MelobaseCore::Event>> PianoRollEventsView::selectedEvents()
{
    return _selectedEvents;
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsView::clearEventSelection(bool isDelegateNotified)
{
    _selectionRect = MDStudio::makeZeroRect();
    _selectedEvents.clear();
    setDirty();
    
    if (isDelegateNotified) {
        if (_didSelectEventsFn)
            _didSelectEventsFn(this, false, false);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsView::resetSelectionRegion()
{
    _selectionRect = MDStudio::makeZeroRect();
    setDirty();
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsView::selectEvents(const std::vector<std::shared_ptr<MelobaseCore::Event>> &events, bool isDelegateNotified)
{
    _selectionRect = MDStudio::makeZeroRect();
    _selectedEvents = events;
    setDirty();

    if (isDelegateNotified && _didSelectEventsFn)
        _didSelectEventsFn(this, false, false);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsView::selectAllEvents(bool isDelegateNotified)
{
    // Get the total nb of ticks
    std::vector<unsigned int> totalNbTicks, eotTickCounts;
    int timeDivision = 480;
    bool areAbsTicks = true;
    _sequenceInfosFn(this, &timeDivision, &totalNbTicks, &eotTickCounts, &areAbsTicks);

    if (totalNbTicks.size() == 0)
        return;
    
    _selectionRect = MDStudio::makeRect(0.0f, 0.0f, totalNbTicks[_trackIndex] * _eventTickWidth, bounds().size.height);

    updateSelectedEvents(_trackIndex, false, false);
    if (_trackIndex > 0)
        updateSelectedEvents(0, true, true);
    if (isDelegateNotified && _didSelectEventsFn)
        _didSelectEventsFn(this, false, false);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsView::setVisibleChannels(std::array<bool, STUDIO_MAX_CHANNELS> visibleChannels)
{
    _visibleChannels = visibleChannels;
    
    // Update the list of selected events in order to remove the events not being visible
    std::vector<std::shared_ptr<MelobaseCore::Event>> selectedEvents;
    for (std::shared_ptr<MelobaseCore::Event> event : _selectedEvents) {
        auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(event);
        if (_visibleChannels[rechannelize(channelEvent->channel())] == true)
            selectedEvents.push_back(event);
    }
    
    _selectedEvents = selectedEvents;
    if (_didSelectEventsFn)
        _didSelectEventsFn(this, false, false);
    
    setDirty();
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsView::setHighlightPitchState(int channel, int pitch, bool state)
{
    _highlightPitchStates[channel][pitch] = state;
    setDirty();
}

// ---------------------------------------------------------------------------------------------------------------------
MDStudio::Rect PianoRollEventsView::selectedEventsFrame()
{
    MDStudio::Rect selectedEventsFrame;

    bool isFirstSelectedEvent = true;
    
    for (int channel = 0; channel < STUDIO_MAX_CHANNELS; ++channel) {
        for (auto eventRect : _eventRects[channel]) {
            if (std::find(_selectedEvents.begin(), _selectedEvents.end(), eventRect.channelEvent) != _selectedEvents.end()) {
                if (isFirstSelectedEvent) {
                    selectedEventsFrame = eventRect.rect;
                    isFirstSelectedEvent = false;
                } else {
                    selectedEventsFrame = makeUnionRect(selectedEventsFrame, eventRect.rect);
                }
            }
        }
    }
    
    return selectedEventsFrame;
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsView::setAddedEvent(std::shared_ptr<MelobaseCore::Event> addedEvent)
{
    _activeEvent = addedEvent;
    _isMovingEvents = true;
    _isResizingEvents = true;
    
    if (_didSelectEventsFn)
        _didSelectEventsFn(this, false, true);
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsView::setTrackIndex(int trackIndex)
{
    if (trackIndex != _trackIndex) {
        clearEventSelection();
        _trackIndex = trackIndex;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsView::setTrackChannel(UInt8 trackChannel)
{
    _trackChannel = trackChannel;
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsView::selectAll()
{
    selectAllEvents();
}
