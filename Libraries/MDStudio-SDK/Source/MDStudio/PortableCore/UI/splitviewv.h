//
//  splitviewv.h
//  MDStudio
//
//  Created by Daniel Cliche on 2013-09-09.
//  Copyright (c) 2016-2020 Daniel Cliche. All rights reserved.
//

#ifndef SPLITVIEWV_H
#define SPLITVIEWV_H

#include <functional>
#include <memory>

#include "view.h"

namespace MDStudio {

class SplitViewV : public View {
   public:
    typedef std::function<void(SplitViewV* sender, float pos)> PosChangedFnType;

   private:
    float _splitPos;

    void draw() override;
    bool handleEvent(const UIEvent* event) override;

    bool _isCaptured;
    bool _cursorWasSet;

    std::shared_ptr<View> _bottomPane;
    std::shared_ptr<View> _topPane;

    PosChangedFnType _posChangedFn;

    void updateFrames();

   public:
    SplitViewV(const std::string& name, void* owner, std::shared_ptr<View> bottomPane, std::shared_ptr<View> topPane,
               float splitPos);
    ~SplitViewV();

    void setFrame(Rect rect) override;

    float splitPos() { return _splitPos; }
    void setSplitPos(float splitPos);

    void setBottomPaneVisibility(bool isVisible) {
        _bottomPane->setIsVisible(isVisible);
        updateFrames();
    }
    void setTopPaneVisibility(bool isVisible) {
        _topPane->setIsVisible(isVisible);
        updateFrames();
    }

    void setPosChangedFn(PosChangedFnType posChangedFn) { _posChangedFn = posChangedFn; }
};

}  // namespace MDStudio

#endif  // SPLITVIEWV_H
