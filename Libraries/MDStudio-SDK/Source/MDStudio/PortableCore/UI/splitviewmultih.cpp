//
//  splitviewmultih.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2017-11-18.
//  Copyright (c) 2017-2020 Daniel Cliche. All rights reserved.
//

#include "splitviewmultih.h"

#include "../platform.h"
#include "draw.h"
#include "math.h"
#include "responderchain.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
SplitViewMultiH::SplitViewMultiH(const std::string& name, void* owner, const std::vector<std::shared_ptr<View>>& panes,
                                 const std::vector<std::pair<float, bool>>& splitPos)
    : View(name, owner) {
    _panes = panes;

    for (auto p : panes) addSubview(p);

    _splitPos = splitPos;
    // setSplitPos(splitPos);

    _isCaptured = false;
    _capturedPosIndex = 0;

    _posChangedFn = nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------
SplitViewMultiH::~SplitViewMultiH() {
    for (auto p : _panes) removeSubview(p);
}

// ---------------------------------------------------------------------------------------------------------------------
void SplitViewMultiH::setSplitPos(const std::vector<std::pair<float, bool>>& splitPos) {
    _splitPos = splitPos;

    updateFrames();

    setDirty();

    if (_posChangedFn) _posChangedFn(this, _splitPos);
}

// ---------------------------------------------------------------------------------------------------------------------
void SplitViewMultiH::draw() {
    DrawContext* dc = drawContext();

    dc->pushStates();
    dc->setFillColor(lightGrayColor);
    dc->setStrokeColor(veryDimGrayColor);

    float p = 0.0f;
    for (auto pos : _splitPos) {
        p += pos.first;
        auto r = makeRect(p - (SPLIT_VIEW_MULTI_H_SPLITTER_WIDTH / 2), -1.0f, SPLIT_VIEW_MULTI_H_SPLITTER_WIDTH,
                          rect().size.height + 2.0f);
        dc->drawRect(r);
    }

    dc->popStates();
}

// ---------------------------------------------------------------------------------------------------------------------
bool SplitViewMultiH::isPointInAnyRect(Point pt, const std::vector<Rect>& rects, size_t* index) {
    size_t i = 0;
    for (auto rect : rects) {
        if (isPointInRect(pt, rect)) {
            *index = i;
            return true;
        }
        ++i;
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
bool SplitViewMultiH::handleEvent(const UIEvent* event) {
    std::vector<Rect> splitterRects;
    float p = 0.0f;
    for (auto pos : _splitPos) {
        p += pos.first;
        auto r = makeRect(p - (SPLIT_VIEW_MULTI_H_SPLITTER_WIDTH / 2), 0.0f, SPLIT_VIEW_MULTI_H_SPLITTER_WIDTH,
                          rect().size.height);
        r.origin.x += rect().origin.x + resolvedOffset().x;
        r.origin.y += rect().origin.y + resolvedOffset().y;
        splitterRects.push_back(r);
    }

    // Update the mouse cursor
    if (isMouseEvent(event)) {
        int i = 0;
        for (auto splitterRect : splitterRects) {
            if (_splitPos.at(i).second && isPointInRect(event->pt, splitterRect)) {
                responderChain()->setCursorInRect(this, Platform::ResizeLeftRightCursor, splitterRect);
                break;
            }
            ++i;
        }
    }

    size_t posIndex = 0;
    auto splitPos = _splitPos;
    if (((event->type == MOUSE_DOWN_UIEVENT) || (event->type == MOUSE_MOVED_UIEVENT)) &&
        (_isCaptured || isPointInAnyRect(event->pt, splitterRects, &posIndex))) {
        // If the column is resizable
        if (_isCaptured || _splitPos.at(posIndex).second) {
            if (!_isCaptured && event->type == MOUSE_DOWN_UIEVENT) {
                _isCaptured = responderChain()->captureResponder(this);
                _capturedPosIndex = posIndex;
                return true;
            } else if (_isCaptured) {
                posIndex = _capturedPosIndex;
                float newSplitPos = floor(event->pt.x) - rect().origin.x - resolvedOffset().x;
                if (newSplitPos < 0.0f) newSplitPos = 0.0f;
                // Adjust the split position in order to be relative to the previous one(s)
                if (posIndex > 0)
                    for (size_t i = 0; i < posIndex; ++i) newSplitPos -= splitPos[i].first;

                if (newSplitPos < 0.0f) newSplitPos = 0.0f;

                splitPos[posIndex].first = newSplitPos;
                setSplitPos(splitPos);
                return true;
            }
        }
        return false;
    } else if (_isCaptured && event->type == MOUSE_UP_UIEVENT) {
        responderChain()->releaseResponder(this);
        _isCaptured = false;
        return true;
    }

    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
void SplitViewMultiH::updateFrames() {
    size_t i = 0;

    float pos = 0.0f;

    for (auto pane : _panes) {
        if (i < _panes.size() - 1) {
            float paneWidth = _splitPos.at(i).first;

            auto r = makeRect(pos + (i > 0 ? (SPLIT_VIEW_MULTI_H_SPLITTER_WIDTH / 2) : 0.0f), 0.0f,
                              paneWidth - (SPLIT_VIEW_MULTI_H_SPLITTER_WIDTH / 2) -
                                  (i > 0 ? (SPLIT_VIEW_MULTI_H_SPLITTER_WIDTH / 2) : 0.0f),
                              rect().size.height);
            pane->setFrame(r);

            pos += paneWidth;
        } else {
            // Last pane

            auto r = makeRect(pos + (i > 0 ? (SPLIT_VIEW_MULTI_H_SPLITTER_WIDTH / 2) : 0.0f), 0.0f,
                              rect().size.width - pos - (SPLIT_VIEW_MULTI_H_SPLITTER_WIDTH / 2), rect().size.height);
            pane->setFrame(r);
        }

        ++i;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void SplitViewMultiH::setFrame(Rect aRect) {
    View::setFrame(aRect);

    updateFrames();
}
