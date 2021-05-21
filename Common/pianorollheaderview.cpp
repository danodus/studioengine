//
//  pianorollheaderview.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2014-09-29.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#include "pianorollheaderview.h"

#include <draw.h>
#include <platform.h>
#include <responderchain.h>

#include <algorithm>
#include <cassert>

#define PIANO_ROLL_HEADER_VIEW_MAX_NB_MEASURES 20000

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
PianoRollHeaderView::PianoRollHeaderView(std::string name, void* owner, int trackIndex, double eventTickWidth,
                                         MDStudio::ScrollView* mainScrollView)
    : View(name, owner), _trackIndex(trackIndex), _eventTickWidth(eventTickWidth), _mainScrollView(mainScrollView) {
    _isCaptured = false;
    _hasFocus = false;

    std::string path = Platform::sharedInstance()->resourcesPath() + "/OpenSans-Semibold.ttf";
    _font = new MultiDPIFont(20, path);
    _flagImage = std::make_shared<MDStudio::Image>("RedFlag@2x.png");
}

// ---------------------------------------------------------------------------------------------------------------------
PianoRollHeaderView::~PianoRollHeaderView() { delete _font; }

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollHeaderView::setCursorTickPos(unsigned int cursorTickPos) {
    int timeDivision = 480;
    std::vector<unsigned int> totalNbTicks, eotTickCounts;
    bool areAbsTicks = true;

    // Get sequence infos
    _sequenceInfosFn(this, &timeDivision, &totalNbTicks, &eotTickCounts, &areAbsTicks);

    unsigned int totalNbTicksCombined = 0;
    if (totalNbTicks.size() > 0) totalNbTicksCombined = *std::max_element(totalNbTicks.begin(), totalNbTicks.end());

    unsigned int tickPos = cursorTickPos < totalNbTicksCombined ? cursorTickPos : totalNbTicksCombined;
    if (_didSetCursorTickPosFn) _didSetCursorTickPosFn(this, tickPos);
}

// ---------------------------------------------------------------------------------------------------------------------
bool PianoRollHeaderView::handleEvent(const UIEvent* event) {
    // Forward horizontal scroll events to main scroll view
    if (_mainScrollView && (event->type == SCROLL_UIEVENT) && isPointInRect(event->pt, clippedRect())) {
        _mainScrollView->scroll(event->deltaX, 0.0f);
        return true;
    }

    if ((event->type == MOUSE_DOWN_UIEVENT || event->type == MOUSE_MOVED_UIEVENT) &&
        (_isCaptured || (isPointInRect(event->pt, clippedRect()) && event->type == MOUSE_DOWN_UIEVENT))) {
        float p = (event->pt.x - clippedRect().origin.x - offset().x) / _eventTickWidth;
        if (p < 0.0f) p = 0.0f;
        if (!_isCaptured && (event->type == MOUSE_DOWN_UIEVENT)) _isCaptured = responderChain()->captureResponder(this);
        setCursorTickPos(p);
        return true;
    } else if (_isCaptured && (event->type == MOUSE_UP_UIEVENT)) {
        responderChain()->releaseResponder(this);
        _isCaptured = false;
        return true;
    }

    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollHeaderView::drawMeasures() {
    DrawContext* dc = drawContext();

    int timeDivision = 480;
    int numerator = 0;
    std::vector<unsigned int> totalNbTicks, eotTickCounts;
    bool areAbsTicks = true;

    // Get sequence infos
    if (_sequenceInfosFn) _sequenceInfosFn(this, &timeDivision, &totalNbTicks, &eotTickCounts, &areAbsTicks);

    if ((totalNbTicks.size() == 0) || (timeDivision == 0)) return;

    std::vector<unsigned int> measureTicks;

    unsigned int nbEvents = _nbEventsFn(this, 0);

    unsigned int tick = 0L, refTick = 0L;

    bool isNoteOn[16][256];
    for (int i = 0; i < 16; ++i)
        for (int j = 0; j < 256; ++j) isNoteOn[i][j] = false;

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
                for (unsigned int t = refTick; t < tick; t += timeDivision * numerator) measureTicks.push_back(t);
            }
            numerator = channelEvent->param1();
            refTick = tick;
        }
    }

    if (numerator) {
        for (unsigned int t = refTick; t < totalNbTicks[_trackIndex]; t += timeDivision * numerator)
            measureTicks.push_back(t);
    }

    unsigned long nbMeasures = measureTicks.size();

    if (nbMeasures > PIANO_ROLL_HEADER_VIEW_MAX_NB_MEASURES) return;

    dc->pushStates();
    dc->setFillColor(_hasFocus ? MDStudio::makeColor(0.4f, 0.4f, 0.4f, 1.0f)
                               : MDStudio::makeColor(0.3f, 0.3f, 0.3f, 1.0f));
    dc->drawRect(MDStudio::makeRect(0, 0, totalNbTicks[_trackIndex] * _eventTickWidth, bounds().size.height));
    dc->popStates();

    // Draw the measures
    for (unsigned int measure = 0; measure < nbMeasures; measure++) {
        unsigned int nbTicksInMeasure =
            (measure == (nbMeasures - 1) ? totalNbTicks[_trackIndex] : measureTicks[measure + 1]) -
            measureTicks[measure];

        Rect r = makeRect((float)(measureTicks[measure]) * _eventTickWidth, 0.0f, nbTicksInMeasure * _eventTickWidth,
                          bounds().size.height);

        if (!isRectInRect(makeRect(r.origin.x + offset().x, r.origin.y + offset().y, r.size.width, r.size.height),
                          clippedBounds()))
            continue;

        dc->pushStates();
        dc->setFillColor(_hasFocus ? grayColor : dimGrayColor);
        dc->drawRect(r);
        dc->popStates();

        r = makeRect((float)(measureTicks[measure]) * _eventTickWidth, 0.0f, 2, bounds().size.height);

        if (measure) {
            dc->pushStates();
            dc->setFillColor(_hasFocus ? lightGrayColor : grayColor);
            dc->drawRect(r);
            dc->popStates();
        }

        if (measure < nbMeasures) {
            r.origin.x += 10.0f;
            dc->pushStates();
            dc->setStrokeColor(makeColor(0.15f, 0.15f, 0.15f, 1.0f));
            dc->drawLeftText(_font, r, std::to_string(measure + 1));
            dc->popStates();
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollHeaderView::drawAnnotations() {
    DrawContext* dc = drawContext();

    dc->pushStates();
    auto nbAnnotations = _nbAnnotationsFn(this);
    for (unsigned int annotationIndex = 0; annotationIndex < nbAnnotations; annotationIndex++) {
        std::shared_ptr<MelobaseCore::SequenceAnnotation> annotation;
        _annotationAtIndexFn(this, annotationIndex, &annotation);

        dc->drawImage(MDStudio::makePoint(annotation->tickCount * _eventTickWidth, 0.0f), _flagImage);
    }
    dc->popStates();
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollHeaderView::draw() {
    if (_trackIndex < 0) return;

    drawMeasures();
    drawAnnotations();
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollHeaderView::setTrackIndex(int trackIndex) {
    _trackIndex = trackIndex;
    setDirty();
}
