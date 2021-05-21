//
//  view.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2013-09-09.
//  Copyright (c) 2013-2021 Daniel Cliche. All rights reserved.
//

#include "view.h"

#include <assert.h>

#include <algorithm>
#include <deque>

#include "draw.h"
#include "responderchain.h"
#include "tooltipmanager.h"

using namespace MDStudio;

constexpr float kMinDistanceForDragging = 5.0f;  // Minimum mouse cursor distance before the dragging starts
constexpr double kAutoscrollPeriod = 0.05;       // Period in seconds between each autoscroll calls

// ---------------------------------------------------------------------------------------------------------------------
View::View(const std::string& name, void* owner) : Responder(name), _owner(owner) {
    _controller = owner;  // By default, the owner is the controller
    _rect = makeZeroRect();
    _clippedRect = makeZeroRect();
    _superview = nullptr;
    _toggleDirtyRect = false;
    _offset = makePoint(0.0f, 0.0f);
    _dirtySetFn = nullptr;
    _layoutFn = nullptr;
    _drawFn = nullptr;
    _handleEventFn = nullptr;
    _disposeFn = nullptr;
    _isVisible = true;
    _isDrawing = false;
    _resolvedClippedRect = makeZeroRect();
    _isDirty = false;
    _isResponderChainDirty = true;
    _areTooltipsDirty = true;
    _responderChain = nullptr;
    _tooltipManager = nullptr;
    _drawContext = nullptr;
    _mouseInsideView = nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------
View::~View() {
    if (_disposeFn) _disposeFn(this);

    // Check if any subview is still there
    // assert(_subviews.size() == 0);

    // Make sure we are no longer in responder chain
    assert(_responderChain || !isInChain());

    // Check if the super view is no longer having us in
    // if (_superview) {
    //    for (auto view : _superview->subviews()) {
    //        if (view.get() == this)
    //            assert(0);
    //    }
    //}

    if (_tooltipManager) delete _tooltipManager;

    if (_responderChain) delete _responderChain;
}

// ---------------------------------------------------------------------------------------------------------------------
void View::createResponderChain() {
    _responderChain = new ResponderChain();
    _responderChain->setWillSendEventFn([this](ResponderChain* sender, const UIEvent* event) {
        if (event->type == MOUSE_MOVED_UIEVENT) {
            trackMouse(event->pt, false);
        } else if (event->type == MOUSE_DOWN_UIEVENT || event->type == MOUSE_UP_UIEVENT) {
            trackMouse(event->pt, true);
        }
    });
}

// ---------------------------------------------------------------------------------------------------------------------
void View::createTooltipManager() { _tooltipManager = new TooltipManager(this); }

// ---------------------------------------------------------------------------------------------------------------------
void View::configureSubviews() {}

// ---------------------------------------------------------------------------------------------------------------------
void View::draw() {
    if (_drawFn) _drawFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void View::didResolveClippedRect() {}

// ---------------------------------------------------------------------------------------------------------------------
void View::willRemoveFromSuperview() {}

// ---------------------------------------------------------------------------------------------------------------------
void View::updateResponderChain() {
    if (_superview) {
        _superview->updateResponderChain();
    } else {
        assert(_responderChain);

        std::deque<Responder*> capturedResponders = _responderChain->capturedResponders();
        _responderChain->removeAllResponders(true);

        // Add the responder chain
        _responderChain->addResponder(this);
        addResponders();

        _responderChain->setCapturedResponders(capturedResponders);

        _isResponderChainDirty = false;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void View::updateTooltips() {
    if (_superview) {
        _superview->updateTooltips();
    } else {
        assert(_tooltipManager);

        _tooltipManager->removeAllTooltips();

        // Add the tooltips
        _tooltipManager->addTooltip(this);
        addTooltips();

        _areTooltipsDirty = false;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void View::drawSubviews(Rect dirtyRect, bool isOverlay) {
    DrawContext* dc = drawContext();
    assert(dc);

    _isDrawing = true;

    // Update the responder chain if we are the top view
    if (_superview == nullptr) {
        if (_isResponderChainDirty) updateResponderChain();

        if (_areTooltipsDirty) updateTooltips();

        resetResolvedClippedRects();

        // Clear the region to be updated
        if (!isOverlay) dc->setClearRegion(dirtyRect);
    }

    if (_isVisible) {
        configureSubviews();

        // Draw all the subviews
        std::vector<std::shared_ptr<View>>::iterator it;
        for (it = _subviews.begin(); it != _subviews.end(); it++) {
            (*it)->resetIsDirty();

            if (!(*it)->isVisible()) continue;

            Rect clippedRect = (*it)->clippedRect();
            Point totalOffset = getTotalOffset();
            Rect translatedClippedRect =
                makeRect(clippedRect.origin.x + totalOffset.x, clippedRect.origin.y + totalOffset.y,
                         clippedRect.size.width, clippedRect.size.height);
            Rect r = (*it)->rect();
            if ((r.origin.x == 0.0f) && (r.origin.y == 0.0f) && (r.size.width == 0.0f) && (r.size.height == 0.0f))
                std::cout << "Warning: drawing the view " << (*it)->name() << " with zero rect" << std::endl;
            Point offset = makePoint(r.origin.x + totalOffset.x + (*it)->offset().x,
                                     r.origin.y + totalOffset.y + (*it)->offset().y);
            Rect translatedRect =
                makeRect(r.origin.x + totalOffset.x, r.origin.y + totalOffset.y, r.size.width, r.size.height);
            Rect scissorRect = (dc->scissor().size.width >= 0 && dc->scissor().size.height >= 0)
                                   ? makeIntersectRect(translatedClippedRect, dc->scissor())
                                   : translatedClippedRect;

            if (isRectInRect(translatedRect, scissorRect)) {
                dc->pushStates();
                (*it)->setResolvedClippedRect(scissorRect);
                dc->setScissor(scissorRect);
                dc->setTranslation(offset);
                (*it)->drawSubviews(dirtyRect);
                if (isRectInRect(scissorRect, dirtyRect)) {
                    dc->pushStates();
                    dc->setScissor(makeIntersectRect(scissorRect, dirtyRect));
                    auto nbStates = dc->nbStates();
                    (*it)->draw();
                    assert(dc->nbStates() >= nbStates);

                    // Handle cases where the draw has misbehaved
                    while (dc->nbStates() > nbStates) dc->popStates();

                    dc->popStates();
                }
                // drawRect(scissorRect, redColor);
                dc->popStates();
            }
        }
    }

    // Debug
    /*
    if (_superview == nullptr) {
        _toggleDirtyRect = !_toggleDirtyRect;
        if (_toggleDirtyRect) {
            dc->pushStates();
            dc->setFillColor(makeColor(1.0f, 0.0f, 0.0f, 0.2f));
            dc->drawRect(dirtyRect);
            dc->popStates();
        }
    }
    */

    if (_draggingView && _isDraggingMotionStarted) {
        dc->pushStates();
        dc->setScissor(dirtyRect);
        dc->setFillColor(makeColor(0.0f, 0.0f, 1.0f, 0.5f));
        dc->drawRect(_draggingDestViewOutlineRect);
        dc->popStates();
        dc->pushStates();
        dc->setStrokeColor(whiteColor);
        dc->drawRect(_draggingViewOutlineRect);
        if (_nbDraggedViewsSendingDrop > 1)
            _drawContext->drawCenteredText(SystemFonts::sharedInstance()->semiboldFont(), _draggingViewOutlineRect,
                                           std::to_string(_nbDraggedViewsSendingDrop));
        dc->popStates();
    }

    // Send a tracking update if the view being tracked has been moved
    if (!_superview && _mouseInsideView) {
        auto mouseInsideResolvedClippedRect = _mouseInsideView->resolvedClippedRect();
        if (_lastMouseInsideResolvedClippedRect.origin.x != mouseInsideResolvedClippedRect.origin.x ||
            _lastMouseInsideResolvedClippedRect.origin.y != mouseInsideResolvedClippedRect.origin.y ||
            _lastMouseInsideResolvedClippedRect.size.width != mouseInsideResolvedClippedRect.size.width ||
            _lastMouseInsideResolvedClippedRect.size.height != mouseInsideResolvedClippedRect.size.height) {
            _lastMouseInsideResolvedClippedRect = mouseInsideResolvedClippedRect;

            trackMouse(responderChain()->lastMousePos(), false);
        }
    }

    _isDrawing = false;
    _isDirty = false;
}

// ---------------------------------------------------------------------------------------------------------------------
MDStudio::Point View::getTotalOffset() {
    Point ret = makeZeroPoint();
    View* view = this;
    while (view) {
        ret.x += view->offset().x;
        ret.y += view->offset().y;
        view = view->superview();
    }
    return ret;
}

// ---------------------------------------------------------------------------------------------------------------------
void View::addResponders() {
    // If the view is not visible, we do nothing
    if (!_isVisible) return;

    std::vector<std::shared_ptr<View>>::iterator it;
    for (it = _subviews.begin(); it != _subviews.end(); it++) {
        if ((*it)->isVisible()) {
            (*it)->addResponders();
            responderChain()->addResponder(it->get());
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void View::addTooltips() {
    // If the view is not visible, we do nothing
    if (!_isVisible) return;

    std::vector<std::shared_ptr<View>>::iterator it;
    for (it = _subviews.begin(); it != _subviews.end(); it++) {
        if ((*it)->isVisible()) {
            (*it)->addTooltips();
            tooltipManager()->addTooltip(it->get());
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void View::resetResolvedClippedRects() {
    std::vector<std::shared_ptr<View>>::iterator it;
    for (it = _subviews.begin(); it != _subviews.end(); it++) {
        (*it)->invalidateResolvedClippedRect();
        (*it)->resetResolvedClippedRects();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void View::addSubview(std::shared_ptr<View> view, bool isFront) {
    assert(!_isDrawing);

    view->setSuperview(this);
    view->setDrawContext(_drawContext);
    if (!isFront) {
        _subviews.push_back(view);
    } else {
        _subviews.insert(_subviews.begin(), view);
    }
    view->invalidateResolvedClippedRect();
    setResponderChainDirty();
    setTooltipsDirty();
}

// ---------------------------------------------------------------------------------------------------------------------
void View::removeSubview(std::shared_ptr<View> view) {
    assert(!_isDrawing);
    view->notifyWillRemoveFromSuperviewRecursively();
    view->removeAllResponders();
    if (isResponderChainAvailable()) responderChain()->removeResponder(view.get());
    view->removeAllTooltips();
    if (isTooltipManagerAvailable()) tooltipManager()->removeTooltip(view.get());
    topView()->unregisterForDragged(view);
    auto it = std::find(_subviews.begin(), _subviews.end(), view);
    assert(it != _subviews.end());
    (*it)->setSuperview(nullptr);
    (*it)->setDrawContext(nullptr);
    _subviews.erase(it);
    view->invalidateResolvedClippedRect();
    setResponderChainDirty();
    setTooltipsDirty();
}

// ---------------------------------------------------------------------------------------------------------------------
void View::removeAllResponders() {
    if (isResponderChainAvailable())
        for (auto view : _subviews) {
            view->removeAllResponders();
            responderChain()->removeResponder(view.get());
        }
}

// ---------------------------------------------------------------------------------------------------------------------
void View::removeAllTooltips() {
    if (isTooltipManagerAvailable())
        for (auto view : _subviews) {
            view->removeAllTooltips();
            tooltipManager()->removeTooltip(view.get());
        }
}

// ---------------------------------------------------------------------------------------------------------------------
void View::removeAllSubviews() {
    assert(!_isDrawing);
    for (auto view : _subviews) view->notifyWillRemoveFromSuperviewRecursively();

    removeAllResponders();
    removeAllTooltips();

    for (auto view : _subviews) {
        view->setSuperview(nullptr);
        view->setDrawContext(nullptr);
        topView()->unregisterForDragged(view);
    }
    _subviews.clear();
    setResponderChainDirty();
    setTooltipsDirty();
}

// ---------------------------------------------------------------------------------------------------------------------
void View::setDirty(Rect dirtyRect, bool isDelegateNotified) {
    if (!isRectInRect(dirtyRect, _resolvedClippedRect)) return;

    if (!_isDirty || (dirtyRect.origin.x < _dirtyRect.origin.x || dirtyRect.origin.y < _dirtyRect.origin.y ||
                      dirtyRect.origin.x + dirtyRect.size.width > _dirtyRect.origin.x + _dirtyRect.size.width ||
                      dirtyRect.origin.y + dirtyRect.size.height > _dirtyRect.origin.y + _dirtyRect.size.height)) {
        if (!_isDirty) {
            _isDirty = true;
            _dirtyRect = dirtyRect;
        } else {
            _dirtyRect = makeUnionRect(_dirtyRect, dirtyRect);
        }

        if (isDelegateNotified && (_dirtySetFn != nullptr)) _dirtySetFn(this, _dirtyRect);

        if (_superview != nullptr) {
            _superview->setDirty(_dirtyRect);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void View::setDirty(bool isDelegateNotified) { setDirty(_resolvedClippedRect, isDelegateNotified); }

// ---------------------------------------------------------------------------------------------------------------------
void View::setDirtyAll(bool isDelegateNotified) { setDirty(topView()->rect(), isDelegateNotified); }

// ---------------------------------------------------------------------------------------------------------------------
void View::setResponderChainDirty() {
    if (_superview) {
        _superview->setResponderChainDirty();
    } else {
        _isResponderChainDirty = true;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void View::setTooltipsDirty() {
    if (_superview) {
        _superview->setTooltipsDirty();
    } else {
        _areTooltipsDirty = true;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void View::setResolvedClippedRect(Rect resolvedClippedRect) {
    _resolvedClippedRect = resolvedClippedRect;
    didResolveClippedRect();
}

// ---------------------------------------------------------------------------------------------------------------------
void View::setClippedRect(Rect clippedRect) { _clippedRect = clippedRect; }

// ---------------------------------------------------------------------------------------------------------------------
void View::setOffset(Point offset) {
    _offset = offset;
    setDirty();
}

// ---------------------------------------------------------------------------------------------------------------------
void View::setRect(Rect rect) {
    // Set the previous position as dirty
    setDirty(_clippedRect);

    _rect = rect;
    _clippedRect = rect;

    if (!_superview) _resolvedClippedRect = rect;

    if (_layoutFn) {
        try {
            _layoutFn(this, frame());
        } catch (const std::runtime_error& e) {
            Platform::sharedInstance()->throwException(e);
        }
    }

    setDirty(_clippedRect);
}

// ---------------------------------------------------------------------------------------------------------------------
View* View::topView() {
    View* topView = this;

    while (topView->superview() != nullptr) topView = topView->superview();

    return topView;
}

// ---------------------------------------------------------------------------------------------------------------------
bool View::hasSubview(std::shared_ptr<View> view) {
    auto it = std::find(_subviews.begin(), _subviews.end(), view);
    return (it != _subviews.end());
}

// ---------------------------------------------------------------------------------------------------------------------
MDStudio::Rect View::frame() {
    if (_superview) {
        Rect r = _superview->rect();
        return makeRect(_rect.origin.x - r.origin.x, _rect.origin.y - r.origin.y, _rect.size.width, _rect.size.height);
    }
    return _rect;
}

// ---------------------------------------------------------------------------------------------------------------------
MDStudio::Rect View::clippedFrame() {
    if (_superview) {
        Rect r = _superview->rect();
        return makeRect(_clippedRect.origin.x - r.origin.x, _clippedRect.origin.y - r.origin.y, _clippedRect.size.width,
                        _clippedRect.size.height);
    }
    return _clippedRect;
}

// ---------------------------------------------------------------------------------------------------------------------
MDStudio::Rect View::clippedBounds() { return makeRect(0.0f, 0.0f, _clippedRect.size.width, _clippedRect.size.height); }

// ---------------------------------------------------------------------------------------------------------------------
MDStudio::Point View::resolvedOffset() {
    Point resolvedOffset = _offset;
    if (_superview) {
        Point offset = superview()->resolvedOffset();
        resolvedOffset.x += offset.x;
        resolvedOffset.y += offset.y;
    }
    return resolvedOffset;
}

// ---------------------------------------------------------------------------------------------------------------------
MDStudio::Rect View::resolvedRect() {
    auto offset = resolvedOffset();
    return makeRect(_rect.origin.x + offset.x, _rect.origin.y + offset.y, _rect.size.width, _rect.size.height);
}

// ---------------------------------------------------------------------------------------------------------------------
void View::setFrame(Rect frame) {
    Rect r;
    if (_superview) {
        Rect r2 = _superview->rect();
        r = makeRect(frame.origin.x + r2.origin.x, frame.origin.y + r2.origin.y, frame.size.width, frame.size.height);
    } else {
        r = frame;
    }
    setRect(r);
}

// ---------------------------------------------------------------------------------------------------------------------
void View::registerMouseTracking(View* view, mouseDidEnterFnType mouseDidEnterFn, mouseDidLeaveFnType mouseDidLeaveFn,
                                 mouseDidDetectButtonActivityFnType mouseDidDetectButtonActivityFn) {
    assert(!_superview);
    _mouseTrackingInfos.push_back(
        MouseTrackingInfoType{view, mouseDidEnterFn, mouseDidLeaveFn, mouseDidDetectButtonActivityFn});
}

// ---------------------------------------------------------------------------------------------------------------------
void View::unregisterMouseTracking(View* view) {
    assert(!_superview);

    if (view == _mouseInsideView) _mouseInsideView = nullptr;

    auto it =
        std::find_if(_mouseTrackingInfos.begin(), _mouseTrackingInfos.end(),
                     [view](const MouseTrackingInfoType& mouseTrackingInfo) { return mouseTrackingInfo.view == view; });

    if (it != _mouseTrackingInfos.end()) _mouseTrackingInfos.erase(it);
}

// ---------------------------------------------------------------------------------------------------------------------
void View::registerForDragged(std::shared_ptr<View> view) {
    assert(!_superview);
    _draggedViews.push_back(view);
}

// ---------------------------------------------------------------------------------------------------------------------
void View::unregisterForDragged(std::shared_ptr<View> view) {
    assert(!_superview);
    auto it = std::find(_draggedViews.begin(), _draggedViews.end(), view);
    if (it != _draggedViews.end()) _draggedViews.erase(it);
}

// ---------------------------------------------------------------------------------------------------------------------
void View::viewsAtPoint(Point pt, std::back_insert_iterator<std::vector<std::shared_ptr<View>>> views) {
    for (auto v : _subviews) {
        // We check if the point is in rect
        if (v->isVisible() && isPointInRect(pt, v->resolvedClippedRect())) {
            *views = v;
            views++;
            v->viewsAtPoint(pt, views);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void View::trackMouse(Point pt, bool isMouseButtonActivity) {
    std::vector<std::shared_ptr<View>> views;
    viewsAtPoint(pt, std::back_inserter(views));
    auto frontViewAtPoint = views.empty() ? nullptr : views.back();

    // Mouse button activity
    if (isMouseButtonActivity && _mouseInsideView) {
        for (auto& mouseTrackingInfo : _mouseTrackingInfos) {
            if (mouseTrackingInfo.view == _mouseInsideView) {
                if (mouseTrackingInfo.mouseDidDetectButtonActivityFn)
                    mouseTrackingInfo.mouseDidDetectButtonActivityFn(this, mouseTrackingInfo.view);
                break;
            }
        }
    }

    // Leaving
    if (_mouseInsideView) {
        if (!frontViewAtPoint || frontViewAtPoint.get() != _mouseInsideView ||
            !isPointInRect(pt, _mouseInsideView->resolvedClippedRect())) {
            for (auto& mouseTrackingInfo : _mouseTrackingInfos) {
                if (mouseTrackingInfo.view == _mouseInsideView) {
                    if (mouseTrackingInfo.mouseDidLeaveFn)
                        mouseTrackingInfo.mouseDidLeaveFn(this, mouseTrackingInfo.view);
                    break;
                }
            }
            _mouseInsideView = nullptr;
        }
    }

    // Entering
    if (!_mouseInsideView) {
        for (auto& mouseTrackingInfo : _mouseTrackingInfos) {
            if (frontViewAtPoint && frontViewAtPoint.get() == mouseTrackingInfo.view &&
                isPointInRect(pt, mouseTrackingInfo.view->resolvedClippedRect())) {
                // Entering new zone
                _mouseInsideView = mouseTrackingInfo.view;
                _lastMouseInsideResolvedClippedRect = _mouseInsideView->resolvedClippedRect();
                if (mouseTrackingInfo.mouseDidEnterFn) mouseTrackingInfo.mouseDidEnterFn(this, mouseTrackingInfo.view);
                break;
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void View::updateAutoscroll() {
    if (_isDraggingMotionStarted) {
        std::vector<std::shared_ptr<View>> views;
        viewsAtPoint(_draggingViewPt, std::back_inserter(views));
        for (auto v : views)
            if (v->isVisible()) v->autoscroll(_draggingViewPt);
        Platform::sharedInstance()->invokeDelayed(
            this, [this]() { updateAutoscroll(); }, kAutoscrollPeriod);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool View::handleEvent(const UIEvent* event) {
    if (_handleEventFn) {
        if (_handleEventFn(this, event)) return true;
    }

    if (_superview) return false;

    if (event->type == MOUSE_DOWN_UIEVENT) {
        if (!_draggingView) {
            _draggingViewStartPoint = event->pt;

            // Calculate the number of dragged views sending drop
            _nbDraggedViewsSendingDrop = 0;
            for (auto v : _draggedViews)
                if (v->isSendingDrop()) ++_nbDraggedViewsSendingDrop;

            for (auto v : _draggedViews) {
                if (v->isInChain() && isPointInRect(event->pt, v->resolvedClippedRect())) {
                    _draggingView = v;
                    _draggingViewRect = v->resolvedClippedRect();
                    _draggingViewPt = event->pt;
                    _draggingViewOutlineRect = _draggingViewRect;
                    _isDraggingMotionStarted = false;
                    responderChain()->captureResponder(this);
                    return true;
                }
            }
        }
    } else if (event->type == MOUSE_UP_UIEVENT) {
        if (_draggingView) {
            auto wasDraggingMotionStarted = _isDraggingMotionStarted;
            if (_isDraggingMotionStarted) {
                std::vector<std::shared_ptr<View>> views;
                viewsAtPoint(event->pt, std::back_inserter(views));
                if (!views.empty()) {
                    std::vector<std::shared_ptr<View>> droppedViews;
                    for (auto v : _draggedViews)
                        if (v->isSendingDrop()) droppedViews.push_back(v);

                    for (auto it = droppedViews.begin(); it != droppedViews.end(); ++it)
                        views.back()->didReceiveDrop(*it, it + 1 == droppedViews.end());
                }
                _isDraggingMotionStarted = false;
            }

            _draggingView = nullptr;
            responderChain()->releaseResponder(this);
            setDirty(makeInsetRect(_draggingViewOutlineRect, -2.0f, -2.0f));
            setDirty(makeInsetRect(_draggingDestViewOutlineRect, -2.0f, -2.0f));
            return wasDraggingMotionStarted;
        }
    } else if (event->type == MOUSE_MOVED_UIEVENT) {
        //
        // Drag & drop
        //

        if (_draggingView) {
            if (_isDraggingMotionStarted ||
                distanceBetweenPoints(_draggingViewStartPoint, event->pt) >= kMinDistanceForDragging) {
                setDirty(makeInsetRect(_draggingViewOutlineRect, -2.0f, -2.0f));
                if (_draggingDestViewOutlineRect.size.width > 0 || _draggingDestViewOutlineRect.size.height > 0)
                    setDirty(makeInsetRect(_draggingDestViewOutlineRect, -2.0f, -2.0f));

                Rect r = _draggingViewRect;
                r.origin.x += event->pt.x - _draggingViewStartPoint.x;
                r.origin.y += event->pt.y - _draggingViewStartPoint.y;
                _draggingViewPt = event->pt;
                _draggingViewOutlineRect = r;

                _draggingDestViewOutlineRect = makeZeroRect();
                std::vector<std::shared_ptr<View>> views;
                viewsAtPoint(event->pt, std::back_inserter(views));
                if (!views.empty()) {
                    if (views.back()->isAcceptingDrop(_draggingView)) {
                        _draggingDestViewOutlineRect = views.back()->resolvedClippedRect();
                    }
                }

                if (!_isDraggingMotionStarted) {
                    _isDraggingMotionStarted = true;
                    updateAutoscroll();
                }

                setDirty(makeInsetRect(_draggingViewOutlineRect, -2.0f, -2.0f));
                if (_draggingDestViewOutlineRect.size.width > 0 || _draggingDestViewOutlineRect.size.height > 0)
                    setDirty(makeInsetRect(_draggingDestViewOutlineRect, -2.0f, -2.0f));

                return true;
            }  // If the dragging motion has been started or if the minimum
               // cursor distance for dragging has been reached

            return false;
        }
    }

    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
bool View::isSendingDrop() { return false; }

// ---------------------------------------------------------------------------------------------------------------------
bool View::isAcceptingDrop(std::shared_ptr<View> draggingView) {
    if (_superview) return _superview->isAcceptingDrop(draggingView);

    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
bool View::didReceiveDrop(std::shared_ptr<View> draggingView, bool isLast) {
    if (_superview) return _superview->didReceiveDrop(draggingView, isLast);

    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
void View::autoscroll(Point pt) {}

// ---------------------------------------------------------------------------------------------------------------------
bool View::isResponderChainAvailable() {
    if (_superview) return _superview->isResponderChainAvailable();

    return _responderChain != nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------
bool View::isTooltipManagerAvailable() {
    if (_superview) return _superview->isTooltipManagerAvailable();

    return _tooltipManager != nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------
ResponderChain* View::responderChain() {
    if (_superview) return _superview->responderChain();

    assert(_responderChain);

    return _responderChain;
}

// ---------------------------------------------------------------------------------------------------------------------
TooltipManager* View::tooltipManager() {
    if (_superview) return _superview->tooltipManager();

    assert(_tooltipManager);

    return _tooltipManager;
}

// ---------------------------------------------------------------------------------------------------------------------
void View::notifyWillRemoveFromSuperviewRecursively() {
    for (auto subview : _subviews) subview->notifyWillRemoveFromSuperviewRecursively();
    willRemoveFromSuperview();
}