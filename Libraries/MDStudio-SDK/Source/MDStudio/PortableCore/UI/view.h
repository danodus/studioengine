//
//  view.h
//  MDStudio
//
//  Created by Daniel Cliche on 2013-09-09.
//  Copyright (c) 2013-2021 Daniel Cliche. All rights reserved.
//

#ifndef VIEW_H
#define VIEW_H

#include <functional>
#include <memory>
#include <vector>

#include "color.h"
#include "drawcontext.h"
#include "rect.h"
#include "responder.h"
#include "responderchain.h"

namespace MDStudio {

class TooltipManager;

class View : public Responder {
   public:
    typedef std::function<void(View* sender, Rect dirtyRect)> dirtySetFnType;
    typedef std::function<void(View* sender, Rect frame)> layoutFnType;
    typedef std::function<void(View* sender)> drawFnType;
    typedef std::function<bool(View* sender, const UIEvent* event)> handleEventFnType;
    typedef std::function<void(View* sender)> disposeFnType;
    typedef std::function<void(View* sender, View* view)> mouseDidEnterFnType;
    typedef std::function<void(View* sender, View* view)> mouseDidLeaveFnType;
    typedef std::function<void(View* sender, View* view)> mouseDidDetectButtonActivityFnType;

   private:
    typedef struct {
        View* view;
        mouseDidEnterFnType mouseDidEnterFn;
        mouseDidLeaveFnType mouseDidLeaveFn;
        mouseDidDetectButtonActivityFnType mouseDidDetectButtonActivityFn;
    } MouseTrackingInfoType;

    std::vector<std::shared_ptr<View>> _subviews;
    std::vector<MouseTrackingInfoType> _mouseTrackingInfos;
    View* _mouseInsideView;
    Rect _lastMouseInsideResolvedClippedRect;
    std::vector<std::shared_ptr<View>> _draggedViews;
    View* _superview;
    bool _toggleDirtyRect;
    bool _isDirty;
    bool _isResponderChainDirty;
    bool _areTooltipsDirty;
    Rect _dirtyRect;

    void* _owner;
    void* _controller;

    DrawContext* _drawContext;

    dirtySetFnType _dirtySetFn;
    layoutFnType _layoutFn;
    drawFnType _drawFn;
    handleEventFnType _handleEventFn;
    disposeFnType _disposeFn;

    bool _isVisible;

    bool _isDrawing;

    Point _draggingViewStartPoint;
    std::shared_ptr<View> _draggingView;
    Rect _draggingViewRect, _draggingViewOutlineRect, _draggingDestViewOutlineRect;
    Point _draggingViewPt;
    bool _isDraggingMotionStarted;
    unsigned int _nbDraggedViewsSendingDrop;

    Rect _resolvedClippedRect;

    std::string _tooltipText;

    void setSuperview(View* view) { _superview = view; }
    void setRect(Rect rect);

    Point getTotalOffset();

    ResponderChain* _responderChain;
    TooltipManager* _tooltipManager;

    void removeAllResponders();
    void removeAllTooltips();

    void trackMouse(Point pt, bool isMouseButtonActivity);

    void addResponders();
    void addTooltips();

    void setResolvedClippedRect(Rect resolvedClippedRect);

    void setResponderChainDirty();
    void setTooltipsDirty();

    void resetResolvedClippedRects();

    void invalidateResolvedClippedRect() { _resolvedClippedRect = makeZeroRect(); }

    void updateAutoscroll();

    void notifyWillRemoveFromSuperviewRecursively();

    void viewsAtPoint(Point pt, std::back_insert_iterator<std::vector<std::shared_ptr<View>>> views);

   protected:
    Rect _rect;
    Rect _clippedRect;
    Point _offset;

   public:
    View(const std::string& name, void* owner);
    virtual ~View();

    void setDrawContext(DrawContext* drawContext) { _drawContext = drawContext; }

    DrawContext* drawContext() {
        if (_superview) return _superview->drawContext();
        return _drawContext;
    }

    virtual void configureSubviews();
    virtual void draw();
    virtual void didResolveClippedRect();
    virtual void willRemoveFromSuperview();
    void drawSubviews(Rect dirtyRect, bool isOverlay = false);
    void addSubview(std::shared_ptr<View> view, bool isFront = false);
    void removeSubview(std::shared_ptr<View> view);
    void removeAllSubviews();
    const std::vector<std::shared_ptr<View>>& subviews() { return _subviews; }

    View* superview() { return _superview; }

    View* topView();

    Rect rect() { return _rect; }
    void setClippedRect(Rect clippedRect);

    Rect clippedFrame();

    Rect frame();
    virtual void setFrame(Rect frame);

    Rect bounds() { return makeRect(0.0f, 0.0f, _rect.size.width, _rect.size.height); }

    Rect clippedRect() { return _clippedRect; }
    Rect clippedBounds();

    Point offset() { return _offset; }
    void setOffset(Point offset);

    Point resolvedOffset();
    Rect resolvedRect();

    void setDirty(Rect dirtyRect, bool isDelegateNotified = true);
    void setDirty(bool isDelegateNotified = true);
    void setDirtyAll(bool isDelegateNotified = true);
    void resetIsDirty() { _isDirty = false; }

    void updateResponderChain();
    void updateTooltips();

    void* owner() { return _owner; }
    void setOwner(void* owner) { _owner = owner; }

    void* controller() { return _controller; }
    void setController(void* controller) { _controller = controller; }

    void setIsVisible(bool isVisible) {
        if (_isVisible != isVisible) {
            _isVisible = isVisible;
            setResponderChainDirty();
            setTooltipsDirty();
            setDirty();
        }
    }
    bool isVisible() { return _isVisible; }

    Rect resolvedClippedRect() const { return _resolvedClippedRect; }

    bool hasSubview(std::shared_ptr<View> view);

    void setDirtySetFn(dirtySetFnType dirtySetFn) { _dirtySetFn = dirtySetFn; }
    void setLayoutFn(layoutFnType layoutFn) { _layoutFn = layoutFn; }
    void setDrawFn(drawFnType drawFn) { _drawFn = drawFn; }
    void setHandleEventFn(handleEventFnType handleEventFn) { _handleEventFn = handleEventFn; }
    void setDisposeFn(disposeFnType disposeFn) { _disposeFn = disposeFn; }

    bool handleEvent(const UIEvent* event) override;

    // Must be called from top view
    void registerMouseTracking(View* view, mouseDidEnterFnType mouseDidEnterFn, mouseDidLeaveFnType mouseDidLeaveFn,
                               mouseDidDetectButtonActivityFnType mouseDidDetectButtonActivityFn = nullptr);
    // Must be called from top view
    void unregisterMouseTracking(View* view);

    // Must be called from top view
    void registerForDragged(std::shared_ptr<View> view);
    // Must be called from top view
    void unregisterForDragged(std::shared_ptr<View> view);

    // Must be called from top view
    void createResponderChain();

    // Must be called from top view
    void createTooltipManager();

    bool isResponderChainAvailable();
    ResponderChain* responderChain();

    bool isTooltipManagerAvailable();
    TooltipManager* tooltipManager();

    virtual bool isSendingDrop();
    virtual bool isAcceptingDrop(std::shared_ptr<View> draggingView);
    virtual bool didReceiveDrop(std::shared_ptr<View> draggingView, bool isLast);
    virtual void autoscroll(Point pt);

    void setTooltipText(const std::string& tooltipText) { _tooltipText = tooltipText; }
    std::string tooltipText() const { return _tooltipText; }
};

}  // namespace MDStudio

#endif  // VIEW_H
