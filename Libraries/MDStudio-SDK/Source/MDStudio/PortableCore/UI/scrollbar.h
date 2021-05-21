//
//  scrollbar.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-28.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <functional>

#include "control.h"

namespace MDStudio {

class ScrollBar : public Control {
    float _min, _max;
    float _posMin, _posMax;
    float _trackingPosMin, _trackingPosMax;

    void draw() override;

    std::function<void(ScrollBar* sender, float posMin, float posMax)> _posChangedFn;

    bool handleEvent(const UIEvent* event) override;

    bool _isCaptured;
    UIEvent _lastEvent;

    Rect makeAdjustedKnobRect(Rect knobRect);

   public:
    ScrollBar(std::string name, void* owner, float min, float max, float posMin, float posMax);
    ~ScrollBar();

    float posMin() { return _posMin; }
    float posMax() { return _posMax; }

    void setMin(float min);
    void setMax(float max);

    void setPos(float posMin, float posMax);

    void setPosChangedFn(std::function<void(ScrollBar* sender, float posMin, float posMax)> posChangedFn) {
        _posChangedFn = posChangedFn;
    }

    bool isCaptured() { return _isCaptured; }
};

}  // namespace MDStudio

#endif  // SCROLLBAR_H
