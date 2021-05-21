//
//  slider.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-14.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef SLIDER_H
#define SLIDER_H

#include "control.h"

namespace MDStudio {

class Slider : public Control {
   public:
    typedef std::function<void(Slider* sender, float value)> PosChangedFnType;
    typedef std::function<void(Slider* sender, float value)> PosSetFnType;

    typedef enum { LinearSliderType, RadialSliderType } SliderEnumType;

   private:
    bool _isCaptured;
    float _pos, _trackingPos;
    float _min, _max;
    UIEvent _lastEvent;
    SliderEnumType _type;

    PosChangedFnType _posChangedFn;
    PosSetFnType _posSetFn;

    std::shared_ptr<Image> _thumbImage;
    std::shared_ptr<Image> _minRailImage, _middleRailImage, _maxRailImage;

    void drawHSlider(DrawContext* dc, Rect r);

   protected:
    bool handleEvent(const UIEvent* event) override;

   public:
    Slider(std::string name, void* owner, float min, float max, float pos);
    ~Slider();

    void setType(SliderEnumType type) {
        _type = type;
        setDirty();
    }
    SliderEnumType type() { return _type; }

    void draw() override;

    void setPosChangedFn(PosChangedFnType posChangedFn) { _posChangedFn = posChangedFn; }
    void setPosSetFn(PosSetFnType posSetFn) { _posSetFn = posSetFn; }

    void setMin(float min, bool isDelegateNotified = true);
    void setMax(float max, bool isDelegateNotified = true);
    void setPos(float pos, bool isDelegateNotified = true);

    float pos() { return _pos; }
    float min() { return _min; }
    float max() { return _max; }

    void setThumbImage(std::shared_ptr<Image> thumbImage) {
        _thumbImage = thumbImage;
        setDirty();
    }
    void setMinRailImage(std::shared_ptr<Image> minRailImage) {
        _minRailImage = minRailImage;
        setDirty();
    }
    void setMiddleRailImage(std::shared_ptr<Image> middleRailImage) {
        _middleRailImage = middleRailImage;
        setDirty();
    }
    void setMaxRailImage(std::shared_ptr<Image> maxRailImage) {
        _maxRailImage = maxRailImage;
        setDirty();
    }
};
}  // namespace MDStudio

#endif  // SLIDER_H
