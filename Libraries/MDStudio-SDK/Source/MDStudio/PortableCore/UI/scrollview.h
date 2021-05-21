//
//  scrollview.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-28.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#ifndef SCROLLVIEW_H
#define SCROLLVIEW_H

#include <functional>
#include <memory>

#include "scrollbar.h"
#include "view.h"

#define SCROLL_VIEW_SCROLL_BAR_THICKNESS 16.0f

namespace MDStudio {

class ScrollView : public View {
   public:
    typedef std::function<void(ScrollView* sender, Point pos)> posChangedFnType;

   private:
    std::shared_ptr<View> _contentView;
    std::shared_ptr<ScrollBar> _horizScrollBar;
    std::shared_ptr<ScrollBar> _vertScrollBar;

    bool _isContentToTop;
    bool _isExternalHorizScrollBar;

    posChangedFnType _posChangedFn;

    bool handleEvent(const UIEvent* event) override;
    void autoscroll(Point pt) override;

    bool _isScrolling;
    bool _isResizingContent;
    bool _isScrollingWithDrag = false;
    bool _isCaptured = false;
    bool _hasMoved;
    Point _trackingPt;

   public:
    ScrollView(const std::string& name, void* owner, std::shared_ptr<View> contentView, bool isContentToTop = false,
               std::shared_ptr<ScrollBar> externalHorizScrollBar = nullptr);
    ~ScrollView();
    void posChanged();
    void setContentSize(Size size);
    void setPos(Point pos);
    Point pos();
    void setPosInvY(Point posInvY);
    Point posInvY();
    float posMinH() { return _horizScrollBar->posMin(); }
    float posMaxH() { return _horizScrollBar->posMax(); }
    float posMinV() { return _vertScrollBar->posMin(); }
    float posMaxV() { return _vertScrollBar->posMax(); }

    void setFrame(Rect rect) override;

    void scrollToVisibleRect(Rect rect);
    void scrollToVisibleRectH(Rect rect);
    void scrollToVisibleRectV(Rect rect);

    void setPosChangedFn(posChangedFnType posChangedFn) { _posChangedFn = posChangedFn; }

    bool isScrollingH();
    bool isScrollingV();
    bool isResizingContent() { return _isResizingContent; }

    void scroll(float deltaX, float deltaY);

    void setIsHorizScrollBarVisible(bool isHorizScrollBarVisible) {
        _horizScrollBar->setIsVisible(isHorizScrollBarVisible);
    }
    void setIsVertScrollBarVisible(bool isVertScrollBarVisible) {
        _vertScrollBar->setIsVisible(isVertScrollBarVisible);
    }

    bool isHorizScrollBarVisible() { return _horizScrollBar->isVisible(); }
    bool isVertScrollBarVisible() { return _horizScrollBar->isVisible(); }

    std::shared_ptr<View> contentView() { return _contentView; }

    void setIsScrollingWithDrag(bool isScrollingWithDrag) { _isScrollingWithDrag = isScrollingWithDrag; }
};

}  // namespace MDStudio

#endif  // SCROLLVIEW_H
