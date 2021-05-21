//
//  trackclipsview.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-09-10.
//  Copyright © 2016-2021 Daniel Cliche. All rights reserved.
//

#include "trackclipsview.h"

#include <draw.h>
#include <responderchain.h>

#include "helpers.h"

#include <platform.h>

#include <algorithm>

#define TRACK_CLIPS_VIEW_MAX_NB_TICKS             (480*4*20000)

// ---------------------------------------------------------------------------------------------------------------------
TrackClipsView::TrackClipsView(std::string name, void *owner, MDStudio::Studio *studio, int trackIndex, UInt8 trackChannel, double eventTickWidth, float eventHeight, MDStudio::ScrollView *mainScrollView) : _studio(studio), _trackIndex(trackIndex), _trackChannel(trackChannel), _eventTickWidth(eventTickWidth), _eventHeight(eventHeight), _mainScrollView(mainScrollView), View(name, owner)
{
    _cursorTickPos = 0;
    _isCaptured = false;
    
    for (int channel = 0; channel < STUDIO_MAX_CHANNELS; ++channel) {
        _visibleChannels[channel] = true;
    }
    
    _isMovingCursor = false;

    _flagImage = std::make_shared<MDStudio::Image>("RedFlag@2x.png");
}

// ---------------------------------------------------------------------------------------------------------------------
TrackClipsView::~TrackClipsView()
{
}

// ---------------------------------------------------------------------------------------------------------------------
void TrackClipsView::updateEventRects(int trackIndex, bool areCombined, bool areChannelEventsSkipped)
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
    
    if ((totalNbTicks.size() == 0) || (totalNbTicks[_trackIndex] > TRACK_CLIPS_VIEW_MAX_NB_TICKS) || (timeDivision == 0))
        return;
    
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
                MDStudio::Rect r = MDStudio::makeRect(currentTickCount * _eventTickWidth, (channelEvent->param1() - 12) * _eventHeight, channelEvent->length() * _eventTickWidth, _eventHeight);
                TrackClipsEventRect eventRect;
                eventRect.rect = r;
                eventRect.channelEvent = channelEvent;
                _eventRects[rechannelize(channelEvent->channel())].push_back(eventRect);
                noteOnEventRectIndices[rechannelize(channelEvent->channel())][channelEvent->param1()] = (int)(_eventRects[rechannelize(channelEvent->channel())].size() - 1);
                noteOnTickCounts[rechannelize(channelEvent->channel())][channelEvent->param1()] = currentTickCount;
                break;
            }
            case CHANNEL_EVENT_TYPE_NOTE_OFF:
            {
                int noteOnEventRectIndice = noteOnEventRectIndices[rechannelize(channelEvent->channel())][channelEvent->param1()];
                if (noteOnEventRectIndice >= 0) {
                    auto noteOnEventRect = &_eventRects[rechannelize(channelEvent->channel())][noteOnEventRectIndice];
                    // Adjust the rect
                    UInt32 length = currentTickCount - noteOnTickCounts[rechannelize(channelEvent->channel())][channelEvent->param1()];
                    noteOnEventRect->rect.size.width = length * _eventTickWidth;
                    noteOnEventRectIndices[rechannelize(channelEvent->channel())][channelEvent->param1()] = -1;
                }
                break;
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool TrackClipsView::handleEvent(const MDStudio::UIEvent *event)
{
    // Forward horizontal scroll events to main scroll view
    if (_mainScrollView && (event->type == MDStudio::SCROLL_UIEVENT) && isPointInRect(event->pt, resolvedClippedRect()))
        _mainScrollView->scroll(event->deltaX, 0.0f);
    
    if ((event->type == MDStudio::MOUSE_DOWN_UIEVENT  || event->type == MDStudio::MOUSE_MOVED_UIEVENT) && (_isCaptured || (isPointInRect(event->pt, resolvedClippedRect()) && event->type == MDStudio::MOUSE_DOWN_UIEVENT))) {
        
        MDStudio::Point off = resolvedOffset();
        
        float p = (event->pt.x - clippedRect().origin.x - off.x) / _eventTickWidth;
        if (p < 0.0f)
            p = 0.0f;
        if (!_isCaptured && (event->type == MDStudio::MOUSE_DOWN_UIEVENT)) {
            _isCaptured = responderChain()->captureResponder(this);
            
            _isMovingCursor = true;
            setCursorTickPos(p);
        } else if (event->type == MDStudio::MOUSE_MOVED_UIEVENT) {
            setCursorTickPos(p);
        }
        return true;
    } else if (_isCaptured && ((event->type == MDStudio::MOUSE_UP_UIEVENT))) {
        responderChain()->releaseResponder(this);
        _isCaptured = false;
        
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
void TrackClipsView::drawFilledRoundRectTopRight(MDStudio::Rect rect, float radius, MDStudio::Color fillColor)
{
    float lineWidth = 1.0f / 2.0f;
    
    // Top-right
    drawFilledArc(MDStudio::makePoint(rect.origin.x + rect.size.width - radius, rect.origin.y + rect.size.height - radius), radius - lineWidth, 0.0f , M_PI / 2.0f, fillColor);
    
    // Right
    drawFilledRect(MDStudio::makeRect(rect.origin.x + rect.size.width - radius, rect.origin.y, radius, rect.size.height - radius), fillColor);
    
    // Center
    drawFilledRect(MDStudio::makeRect(rect.origin.x, rect.origin.y, rect.size.width - radius, rect.size.height), fillColor);
    
    // Top-right
    drawArc(MDStudio::makePoint(rect.origin.x + rect.size.width - radius, rect.origin.y + rect.size.height - radius), radius - lineWidth, 0.0f , M_PI / 2.0f, MDStudio::whiteColor);
    
    // Left
    drawLine(MDStudio::makePoint(rect.origin.x + lineWidth, rect.origin.y), MDStudio::makePoint(rect.origin.x + lineWidth, rect.origin.y + rect.size.height), MDStudio::whiteColor, 1.0f);
    
    // Top
    drawLine(MDStudio::makePoint(rect.origin.x, rect.origin.y + rect.size.height - lineWidth), MDStudio::makePoint(rect.origin.x + rect.size.width - radius, rect.origin.y + rect.size.height - lineWidth), MDStudio::whiteColor, 1.0f);
    
    // Right
    drawLine(MDStudio::makePoint(rect.origin.x + rect.size.width - lineWidth, rect.origin.y), MDStudio::makePoint(rect.origin.x + rect.size.width - lineWidth, rect.origin.y + rect.size.height - radius), MDStudio::whiteColor, 1.0f);
    
    // Bottom
    drawLine(MDStudio::makePoint(rect.origin.x, rect.origin.y + lineWidth), MDStudio::makePoint(rect.origin.x + rect.size.width, rect.origin.y + lineWidth), MDStudio::whiteColor, 1.0f);
    
    
}

// ---------------------------------------------------------------------------------------------------------------------
void TrackClipsView::drawFilledRoundRectTopLeft(MDStudio::Rect rect, float radius, MDStudio::Color fillColor)
{
    float lineWidth = 1.0f / 2.0f;
    
    // Top-left
    drawFilledArc(MDStudio::makePoint(rect.origin.x + radius, rect.origin.y + rect.size.height - radius), radius - lineWidth, M_PI / 2.0f , M_PI / 2.0f, fillColor);
    
    // Left
    drawFilledRect(MDStudio::makeRect(rect.origin.x, rect.origin.y, radius, rect.size.height - radius), fillColor);
    
    // Center
    drawFilledRect(MDStudio::makeRect(rect.origin.x + radius, rect.origin.y, rect.size.width - radius, rect.size.height), fillColor);
    
    // Top-left
    drawArc(MDStudio::makePoint(rect.origin.x + radius, rect.origin.y + rect.size.height - radius), radius - lineWidth, M_PI / 2.0f , M_PI / 2.0f, MDStudio::whiteColor);
    
    // Left
    drawLine(MDStudio::makePoint(rect.origin.x + lineWidth, rect.origin.y), MDStudio::makePoint(rect.origin.x + lineWidth, rect.origin.y + rect.size.height - radius), MDStudio::whiteColor, 1.0f);
    
    // Top
    drawLine(MDStudio::makePoint(rect.origin.x + radius, rect.origin.y + rect.size.height - lineWidth), MDStudio::makePoint(rect.origin.x + rect.size.width, rect.origin.y + rect.size.height - lineWidth), MDStudio::whiteColor, 1.0f);
    
    // Right
    drawLine(MDStudio::makePoint(rect.origin.x + rect.size.width - lineWidth, rect.origin.y), MDStudio::makePoint(rect.origin.x + rect.size.width - lineWidth, rect.origin.y + rect.size.height), MDStudio::whiteColor, 1.0f);
    
    // Bottom
    drawLine(MDStudio::makePoint(rect.origin.x, rect.origin.y + lineWidth), MDStudio::makePoint(rect.origin.x + rect.size.width, rect.origin.y + lineWidth), MDStudio::whiteColor, 1.0f);
}

// ---------------------------------------------------------------------------------------------------------------------
void TrackClipsView::getMeasureTicks(int timeDivision, unsigned int totalNbTicks, bool areAbsTicks, std::vector<unsigned int> *measureTicks, std::vector<int> *numerators)
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
void TrackClipsView::drawAnnotations() {
    MDStudio::DrawContext* dc = drawContext();

    dc->pushStates();
    dc->setFillColor(MDStudio::redColor);
    auto nbAnnotations = _nbAnnotationsFn(this);
    for (unsigned int annotationIndex = 0; annotationIndex < nbAnnotations; annotationIndex++) {
        std::shared_ptr<MelobaseCore::SequenceAnnotation> annotation;
        _annotationAtIndexFn(this, annotationIndex, &annotation);
        auto r = MDStudio::makeRect(annotation->tickCount * _eventTickWidth, 0.0f, 1.0f, frame().size.height - _flagImage->size().height);
        dc->drawRect(r);
        dc->drawImage(MDStudio::makePoint(r.origin.x, r.origin.y + r.size.height), _flagImage);
    }
    dc->popStates();
}

// ---------------------------------------------------------------------------------------------------------------------
void TrackClipsView::draw()
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
    
    if (totalNbTicks[_trackIndex] <= TRACK_CLIPS_VIEW_MAX_NB_TICKS) {
        
        std::vector<unsigned int>measureTicks;
        std::vector<int>numerators;
        
        getMeasureTicks(timeDivision, totalNbTicks[_trackIndex], areAbsTicks, &measureTicks, &numerators);
        
        dc->pushStates();
        dc->setFillColor(MDStudio::makeColor(0.2f, 0.2f, 0.2f, 1.0f));
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
            dc->setFillColor(MDStudio::veryDimGrayColor);
            dc->drawRect(r);
            dc->popStates();
            
            if (measure) {
                dc->pushStates();
                dc->setFillColor(MDStudio::grayColor);
                dc->drawRect(MDStudio::makeRect(r.origin.x, 2, 1, bounds().size.height));
                dc->popStates();
            }
            
        } // for each measure
        
        // Draw the events
        updateEventRects(_trackIndex, false, false);
        if (_trackIndex > 0)
            updateEventRects(0, true, true);
        
        for (int channel = STUDIO_MAX_CHANNELS - 1; channel >= 0; --channel) {
            
            // Draw controller lines
            MDStudio::Color channelColor = channelColors[channel];
            channelColor.red *= 0.75f;
            channelColor.green *= 0.75f;
            channelColor.blue *= 0.75f;
            
            
            // Draw events
            for (auto eventRect : _eventRects[channel]) {
                
                bool isMetaEvent = getIsMetaEvent(eventRect.channelEvent);
                if (isMetaEvent || _visibleChannels[rechannelize(eventRect.channelEvent->channel())]) {
                    
                    if (isRectInRect(MDStudio::makeRect(eventRect.rect.origin.x + offset().x, eventRect.rect.origin.y + offset().y, eventRect.rect.size.width, eventRect.rect.size.height), clippedBounds())) {
                        MDStudio::Color color = isMetaEvent ? MDStudio::grayColor : channelColor;
                        
                        if (eventRect.channelEvent->type() == CHANNEL_EVENT_TYPE_NOTE) {
                            dc->pushStates();
                            dc->setFillColor(color);
                            dc->setStrokeColor(MDStudio::whiteColor);
                            dc->drawRect(eventRect.rect);
                            dc->popStates();
                        }
                    }
                } // If the channel is visible
            }
        }
    } // if we are not beyound the limit number of ticks
    
    drawAnnotations();

    // Draw the end of track grayed-out region
    MDStudio::Color color = MDStudio::makeColor(0.0f, 0.0f, 0.0f, 0.5f);
    float eotOrigin = eotTickCounts[_trackIndex] * _eventTickWidth;
    MDStudio::Rect r = MDStudio::makeRect(eotOrigin, 0.0f, rect().size.width - eotOrigin, rect().size.height);
    dc->pushStates();
    dc->setFillColor(color);
    dc->drawRect(r);
    dc->popStates();
    
    if (totalNbTicks[_trackIndex] > TRACK_CLIPS_VIEW_MAX_NB_TICKS) {
        dc->pushStates();
        dc->setFillColor(MDStudio::veryDimGrayColor);
        dc->drawRect(fullRect);
        dc->popStates();
        dc->pushStates();
        dc->setStrokeColor(MDStudio::whiteColor);
        dc->drawCenteredText(MDStudio::SystemFonts::sharedInstance()->semiboldFont(), MDStudio::makeRect(-offset().x, -offset().y, clippedRect().size.width, clippedRect().size.height), MDStudio::Platform::sharedInstance()->language() == "fr" ? "La séquence est trop longe pour être affichée" : "The sequence is too long to be displayed.");
        dc->popStates();
    }
    
    // We draw the cursor
    r = MDStudio::makeRect(_eventTickWidth * (float)_cursorTickPos, 0.0f, pianoRollCursorWidth, bounds().size.height);
    dc->pushStates();
    dc->setFillColor(MDStudio::makeColor(0.0f, 0.5f, 1.0f, 0.6f));
    dc->drawRect(r);
    dc->popStates();
}

// ---------------------------------------------------------------------------------------------------------------------
void TrackClipsView::setCursorTickPos(unsigned int cursorTickPos, bool isDelegateNotified)
{
    if (_trackIndex < 0)
        return;
    
    int timeDivision = 480;
    
    std::vector<unsigned int> totalNbTicks, eotTickCounts;
    bool areAbsTicks = true;
    
    // Get sequence infos
    _sequenceInfosFn(this, &timeDivision, &totalNbTicks, &eotTickCounts, &areAbsTicks);
    
    unsigned int totalNbTicksCombined = 0;
    if (totalNbTicks.size() > 0)
        totalNbTicksCombined = *std::max_element(totalNbTicks.begin(), totalNbTicks.end());
    
    _cursorTickPos = cursorTickPos < totalNbTicksCombined ? cursorTickPos : totalNbTicksCombined;
    setDirty();
    if (isDelegateNotified && _didSetCursorTickPosFn)
        _didSetCursorTickPosFn(this, _cursorTickPos);
}

// ---------------------------------------------------------------------------------------------------------------------
void TrackClipsView::setVisibleChannels(std::array<bool, STUDIO_MAX_CHANNELS> visibleChannels)
{
    _visibleChannels = visibleChannels;
    
    setDirty();
}

// ---------------------------------------------------------------------------------------------------------------------
void TrackClipsView::setTrackIndex(int trackIndex)
{
    if (trackIndex != _trackIndex) {
        _trackIndex = trackIndex;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TrackClipsView::setTrackChannel(UInt8 trackChannel)
{
    _trackChannel = trackChannel;
}
