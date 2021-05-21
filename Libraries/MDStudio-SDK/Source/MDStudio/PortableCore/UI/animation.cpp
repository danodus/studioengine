//
//  animation.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-11-15.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#include "animation.h"

#include <platform.h>

#include <cassert>
#include <cmath>
#include <iostream>

using namespace MDStudio;

constexpr auto kAnimationUpdatePeriod = 0.05;

// ---------------------------------------------------------------------------------------------------------------------
AnimationPath::AnimationPath(float speed, std::function<void(Point pt)> animationFn)
    : _speed(speed), _animationFn(animationFn) {}

// ---------------------------------------------------------------------------------------------------------------------
LinearAnimationPath::LinearAnimationPath(Point startPt, Point endPt, float speed, bool isRepeating,
                                         std::function<void(Point pt)> animationFn)
    : _startPt(startPt), _endPt(endPt), _isRepeating(isRepeating), AnimationPath(speed, animationFn) {
    _pt = startPt;
    auto dx = _endPt.x - _startPt.x;
    auto dy = _endPt.y - _startPt.y;
    auto distance = sqrtf(dx * dx + dy * dy);
    _nbSteps = static_cast<unsigned int>(distance / (speed * static_cast<float>(kAnimationUpdatePeriod)));
    _incX = dx / _nbSteps;
    _incY = dy / _nbSteps;

    _step = 0;
}

// ---------------------------------------------------------------------------------------------------------------------
bool LinearAnimationPath::update() {
    _animationFn(_pt);
    _pt.x += _incX;
    _pt.y += _incY;
    _step++;
    if (_step > _nbSteps) {
        if (_isRepeating) {
            _step = 0;
            _pt = _startPt;
        } else {
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
Animation::~Animation() { Platform::sharedInstance()->cancelDelayedInvokes(this); }

// ---------------------------------------------------------------------------------------------------------------------
void Animation::update() {
    bool isDone = true;
    for (const auto& path : _paths) {
        if (!path->update()) isDone = false;
    }

    if (!isDone) {
        Platform::sharedInstance()->invokeDelayed(
            this, [=] { update(); }, kAnimationUpdatePeriod);
    } else {
        if (_animationDidFinishFn) _animationDidFinishFn(this);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Animation::addPath(std::shared_ptr<AnimationPath> path) { _paths.push_back(path); }

// ---------------------------------------------------------------------------------------------------------------------
void Animation::start() { update(); }

// ---------------------------------------------------------------------------------------------------------------------
void Animation::stop() { Platform::sharedInstance()->cancelDelayedInvokes(this); }
