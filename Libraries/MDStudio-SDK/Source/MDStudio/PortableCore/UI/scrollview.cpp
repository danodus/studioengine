//
//  scrollview.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-28.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#include "scrollview.h"

#include <math.h>

#include "draw.h"

using namespace MDStudio;

constexpr float kAutoscrollHeight = 20.0f;  // Height of the region for autoscroll
constexpr float kAutoscrollStep = 20.0f;    // Maximum step modulated with the distance

// ---------------------------------------------------------------------------------------------------------------------
static void horizPosChanged(ScrollBar* sender, float posMin, float posMax) {
    ScrollView* scrollView = (ScrollView*)sender->owner();
    scrollView->posChanged();
}

// ---------------------------------------------------------------------------------------------------------------------
static void vertPosChanged(ScrollBar* sender, float posMin, float posMax) {
    ScrollView* scrollView = (ScrollView*)sender->owner();
    scrollView->posChanged();
}

// ---------------------------------------------------------------------------------------------------------------------
ScrollView::ScrollView(const std::string& name, void* owner, std::shared_ptr<View> contentView, bool isContentToTop,
                       std::shared_ptr<ScrollBar> externalHorizScrollBar)
    : View(name, owner), _isContentToTop(isContentToTop) {
    _posChangedFn = nullptr;
    _isScrolling = false;
    _isResizingContent = false;
    _isExternalHorizScrollBar = false;

    _contentView = contentView;

    float horizScrollBarHeight = 0.0f;

    // Add horizontal scroll bar
    if (!externalHorizScrollBar) {
        _horizScrollBar = std::shared_ptr<ScrollBar>(new ScrollBar("horizScrollBar", this, 0.0f, 0.0f, 0.0f, 0.0f));
        addSubview(_horizScrollBar);
        horizScrollBarHeight = SCROLL_VIEW_SCROLL_BAR_THICKNESS;
    } else {
        _horizScrollBar = externalHorizScrollBar;
        _horizScrollBar->setOwner(this);
        _isExternalHorizScrollBar = true;
    }
    _horizScrollBar->setPosChangedFn(horizPosChanged);

    // Add vertical scroll bar
    _vertScrollBar = std::shared_ptr<ScrollBar>(new ScrollBar("vertScrollBar", this, 0.0f, 0.0f, 0.0f, 0.0f));
    _vertScrollBar->setPosChangedFn(vertPosChanged);
    addSubview(_vertScrollBar);

    Size contentSize = contentView->rect().size;

    // Add content view
    Rect contentRect = _contentView->rect();
    contentRect.origin.x = 0.0f;
    contentRect.origin.y = horizScrollBarHeight;
    _contentView->setFrame(contentRect);

    Rect r = makeRect(rect().origin.x, rect().origin.y + horizScrollBarHeight,
                      rect().size.width - SCROLL_VIEW_SCROLL_BAR_THICKNESS, rect().size.height - horizScrollBarHeight);
    _contentView->setClippedRect(makeIntersectRect(r, _contentView->rect()));
    addSubview(_contentView);

    // Set initial content size
    setContentSize(contentSize);
}

// ---------------------------------------------------------------------------------------------------------------------
ScrollView::~ScrollView() {
    if (!_isExternalHorizScrollBar) removeSubview(_horizScrollBar);
    removeSubview(_vertScrollBar);
    removeSubview(_contentView);
}

// ---------------------------------------------------------------------------------------------------------------------
void ScrollView::scroll(float deltaX, float deltaY) {
    Point p = pos();
    p.x -= deltaX;
    p.y += deltaY;
    setPos(p);
}

// ---------------------------------------------------------------------------------------------------------------------
bool ScrollView::handleEvent(const UIEvent* event) {
    if ((event->type == SCROLL_UIEVENT) && isPointInRect(event->pt, resolvedClippedRect())) {
        if (event->phase == PHASE_BEGAN_UIEVENT) {
            _isScrolling = true;
        } else if (event->phase == PHASE_ENDED_UIEVENT) {
            _isScrolling = false;
        }

        scroll(event->deltaX, event->deltaY);

        return true;
    }

    if (_isScrollingWithDrag) {
        if (event->type == MOUSE_DOWN_UIEVENT) {
            if (!_isCaptured) {
                if (isPointInRect(event->pt, resolvedClippedRect())) {
                    _isCaptured = responderChain()->captureResponder(this);
                    _trackingPt = event->pt;
                    _hasMoved = false;
                    return true;
                }
            }
        } else if (event->type == MOUSE_MOVED_UIEVENT) {
            if (_isCaptured) {
                scroll(event->pt.x - _trackingPt.x, -(event->pt.y - _trackingPt.y));
                _trackingPt = event->pt;
                _hasMoved = true;
                return true;
            }
        } else if (event->type == MOUSE_UP_UIEVENT) {
            if (_isCaptured) {
                responderChain()->releaseResponder(this);
                _isCaptured = false;
                return _hasMoved;
            }
        }
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
void ScrollView::autoscroll(Point pt) {
    auto rcr = _contentView->resolvedClippedRect();

    if (rcr.size.height > 3.0f * kAutoscrollHeight) {
        if (isPointInRect(pt, makeRect(rcr.origin.x, rcr.origin.y + rcr.size.height - kAutoscrollHeight, rcr.size.width,
                                       kAutoscrollHeight))) {
            auto distanceFromTop = rcr.origin.y + rcr.size.height - pt.y;
            scroll(0, kAutoscrollStep * (kAutoscrollHeight - distanceFromTop) / kAutoscrollHeight);
        }

        if (isPointInRect(pt, makeRect(rcr.origin.x, rcr.origin.y, rcr.size.width, kAutoscrollHeight))) {
            auto distanceFromBottom = pt.y - rcr.origin.y;
            scroll(0, -kAutoscrollStep * (kAutoscrollHeight - distanceFromBottom) / kAutoscrollHeight);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ScrollView::setFrame(Rect aRect) {
    View::setFrame(aRect);

    Rect r;

    float horizScrollBarHeight = 0.0f;
    float vertScrollBarWidth = _vertScrollBar->isVisible() ? SCROLL_VIEW_SCROLL_BAR_THICKNESS : 0.0f;

    if (!_isExternalHorizScrollBar && _horizScrollBar->isVisible()) {
        r = makeRect(0.0f, 0.0f, frame().size.width - vertScrollBarWidth, SCROLL_VIEW_SCROLL_BAR_THICKNESS);
        _horizScrollBar->setFrame(r);
        horizScrollBarHeight = SCROLL_VIEW_SCROLL_BAR_THICKNESS;
    }

    if (_vertScrollBar->isVisible()) {
        r = makeRect(frame().size.width - SCROLL_VIEW_SCROLL_BAR_THICKNESS, horizScrollBarHeight,
                     SCROLL_VIEW_SCROLL_BAR_THICKNESS, frame().size.height - horizScrollBarHeight);
        _vertScrollBar->setFrame(r);
    }

    // Re-apply the content size in order to update the frame
    setContentSize(_contentView->frame().size);
}

// ---------------------------------------------------------------------------------------------------------------------
void ScrollView::posChanged() {
    Point offset = makePoint(roundf(-_horizScrollBar->posMin()), roundf(-_vertScrollBar->posMin()));
    _contentView->setOffset(offset);
    _contentView->setDirty();

    if (_posChangedFn) {
        _posChangedFn(this, offset);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ScrollView::setContentSize(Size size) {
    _isResizingContent = true;

    Point pt = posInvY();

    float horizScrollBarHeight =
        (_isExternalHorizScrollBar || !_horizScrollBar->isVisible()) ? 0.0f : SCROLL_VIEW_SCROLL_BAR_THICKNESS;
    float vertScrollBarWidth = !_vertScrollBar->isVisible() ? 0.0f : SCROLL_VIEW_SCROLL_BAR_THICKNESS;

    Rect r = makeRect(0.0f, horizScrollBarHeight, size.width, size.height);

    // If the content must be at the top, we perform the adjustment
    if (_isContentToTop && (size.height < (rect().size.height - horizScrollBarHeight))) {
        float toTopOffset = rect().size.height - size.height - horizScrollBarHeight;
        r.origin.y += toTopOffset;
    }

    _contentView->setFrame(r);
    r = makeRect(rect().origin.x, rect().origin.y + horizScrollBarHeight, rect().size.width - vertScrollBarWidth,
                 rect().size.height - horizScrollBarHeight);
    _contentView->setClippedRect(makeIntersectRect(r, _contentView->rect()));

    _horizScrollBar->setMin(0.0);
    _horizScrollBar->setMax(size.width);
    _vertScrollBar->setMin(0.0);
    _vertScrollBar->setMax(size.height);

    setPosInvY(pt);

    _isResizingContent = false;
}

// ---------------------------------------------------------------------------------------------------------------------
void ScrollView::setPos(MDStudio::Point pos) {
    _horizScrollBar->setPos(pos.x, pos.x + _contentView->clippedRect().size.width);
    _vertScrollBar->setPos(pos.y, pos.y + _contentView->clippedRect().size.height);
}

// ---------------------------------------------------------------------------------------------------------------------
Point ScrollView::pos() { return makePoint(_horizScrollBar->posMin(), _vertScrollBar->posMin()); }

// ---------------------------------------------------------------------------------------------------------------------
void ScrollView::setPosInvY(MDStudio::Point posInvY) {
    float posY = _contentView->rect().size.height - _contentView->clippedRect().size.height - posInvY.y;
    setPos(makePoint(posInvY.x, posY));
}

// ---------------------------------------------------------------------------------------------------------------------
Point ScrollView::posInvY() {
    Point p = pos();
    float posYInv = -(p.y - _contentView->rect().size.height + _contentView->clippedRect().size.height);
    return makePoint(p.x, posYInv);
}

// ---------------------------------------------------------------------------------------------------------------------
void ScrollView::scrollToVisibleRectH(Rect rect) {
    // Horizontal

    float p = rect.origin.x - _contentView->rect().origin.x;
    if (_horizScrollBar->posMin() > p) _horizScrollBar->setPos(p, p + _contentView->clippedRect().size.width);

    p = rect.origin.x + rect.size.width - _contentView->rect().origin.x;
    if (_horizScrollBar->posMax() < p) _horizScrollBar->setPos(p - _contentView->clippedRect().size.width, p);
}

// ---------------------------------------------------------------------------------------------------------------------
void ScrollView::scrollToVisibleRectV(Rect rect) {
    // Vertical

    float p = rect.origin.y - _contentView->rect().origin.y;
    if (_vertScrollBar->posMin() > p) _vertScrollBar->setPos(p, p + _contentView->clippedRect().size.height);

    p = rect.origin.y + rect.size.height - _contentView->rect().origin.y;
    if (_vertScrollBar->posMax() < p) _vertScrollBar->setPos(p - _contentView->clippedRect().size.height, p);
}

// ---------------------------------------------------------------------------------------------------------------------
void ScrollView::scrollToVisibleRect(Rect rect) {
    scrollToVisibleRectH(rect);
    scrollToVisibleRectV(rect);
}

// ---------------------------------------------------------------------------------------------------------------------
bool ScrollView::isScrollingH() { return (_isScrolling || _horizScrollBar->isCaptured()); }

// ---------------------------------------------------------------------------------------------------------------------
bool ScrollView::isScrollingV() { return (_isScrolling || _vertScrollBar->isCaptured()); }
