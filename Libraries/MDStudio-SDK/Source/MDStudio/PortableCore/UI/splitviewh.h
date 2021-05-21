//
//  splitview.h
//  MDStudio
//
//  Created by Daniel Cliche on 2013-09-09.
//  Copyright (c) 2013-2020 Daniel Cliche. All rights reserved.
//

#ifndef SPLITVIEWH_H
#define SPLITVIEWH_H

#include <functional>
#include <memory>

#include "view.h"

namespace MDStudio {

class SplitViewH : public View {
   public:
    typedef std::function<void(SplitViewH* sender, float pos)> PosChangedFnType;

   private:
    float _splitPos;
    bool _isPosRelativeToRight;

    void draw() override;
    bool handleEvent(const UIEvent* event) override;

    bool _isCaptured;
    bool _cursorWasSet;

    std::shared_ptr<View> _leftPane;
    std::shared_ptr<View> _rightPane;

    PosChangedFnType _posChangedFn;

    void updateFrames();

   public:
    SplitViewH(const std::string& name, void* owner, std::shared_ptr<View> leftPane, std::shared_ptr<View> rightPane,
               float splitPos, bool isPosRelativeToRight = false);
    ~SplitViewH();

    void setFrame(Rect rect) override;

    float splitPos() { return _splitPos; }
    void setSplitPos(float splitPos);

    void setLeftPaneVisibility(bool isVisible) {
        _leftPane->setIsVisible(isVisible);
        updateFrames();
    }
    void setRightPaneVisibility(bool isVisible) {
        _rightPane->setIsVisible(isVisible);
        updateFrames();
    }

    void setPosChangedFn(PosChangedFnType posChangedFn) { _posChangedFn = posChangedFn; }
};

}  // namespace MDStudio

#endif  // SPLITVIEWH_H
