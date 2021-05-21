//
//  animation.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-11-15.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#ifndef ANIMATION_H
#define ANIMATION_H

#include <functional>
#include <memory>
#include <vector>

#include "point.h"

namespace MDStudio {

class AnimationPath {
   protected:
    std::function<void(Point pt)> _animationFn;
    float _speed;
    unsigned int _nbSteps;
    unsigned int _step;

   public:
    AnimationPath(float speed, std::function<void(Point pt)> animationFn);
    virtual ~AnimationPath() = default;

    virtual bool update() = 0;
};

class LinearAnimationPath : public AnimationPath {
    Point _startPt;
    Point _endPt;
    bool _isRepeating;
    Point _pt;
    float _incX, _incY;

    bool update() override;

   public:
    LinearAnimationPath(Point startPt, Point endPt, float speed, bool isRepeating,
                        std::function<void(Point pt)> animationFn);
};

class Animation {
   public:
    typedef std::function<void(Animation* sender)> AnimationDidFinishFnType;

   private:
    std::vector<std::shared_ptr<AnimationPath>> _paths;

    void update();

    AnimationDidFinishFnType _animationDidFinishFn = nullptr;

   public:
    ~Animation();

    void addPath(std::shared_ptr<AnimationPath> path);

    void start();
    void stop();

    void setAnimationDidFinishFn(AnimationDidFinishFnType animationDidFinish) {
        _animationDidFinishFn = animationDidFinish;
    }
};

}  // namespace MDStudio

#endif  // ANIMATION_H
