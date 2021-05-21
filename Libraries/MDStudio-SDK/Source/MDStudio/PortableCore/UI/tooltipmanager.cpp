//
//  tooltipmanager.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2018-09-07.
//  Copyright (c) 2018-2021 Daniel Cliche. All rights reserved.
//

#include "tooltipmanager.h"

#include <algorithm>

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
TooltipManager::TooltipManager(View* topView) : _topView(topView) { _tooltip = nullptr; }

// ---------------------------------------------------------------------------------------------------------------------
TooltipManager::~TooltipManager() { hideTooltip(); }

// ---------------------------------------------------------------------------------------------------------------------
void TooltipManager::hideTooltip() {
    Platform::sharedInstance()->cancelDelayedInvokes(this);
    if (_tooltip) {
        delete _tooltip;
        _tooltip = nullptr;
    }
    _currentTooltipView = nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------
void TooltipManager::addTooltip(View* view) {
    if (view->tooltipText().empty()) return;

    using namespace std::placeholders;

    _views.push_back(view);
    _topView->registerMouseTracking(view, std::bind(&TooltipManager::viewMouseDidEnter, this, _1, _2),
                                    std::bind(&TooltipManager::viewMouseDidLeave, this, _1, _2),
                                    std::bind(&TooltipManager::viewMouseDidDetectButtonActivity, this, _1, _2));
}

// ---------------------------------------------------------------------------------------------------------------------
void TooltipManager::removeTooltip(View* view) {
    if (_currentTooltipView == view) hideTooltip();

    _topView->unregisterMouseTracking(view);
    auto it = std::find(_views.begin(), _views.end(), view);
    if (it != _views.end()) _views.erase(it);
}

// ---------------------------------------------------------------------------------------------------------------------
void TooltipManager::removeAllTooltips() {
    hideTooltip();
    for (auto view : _views) {
        _topView->unregisterMouseTracking(view);
    }
    _views.clear();
}

// ---------------------------------------------------------------------------------------------------------------------
void TooltipManager::viewMouseDidEnter(View* sender, View* view) {
    if (_currentTooltipView != view) {
        bool wasTooltipShown = _tooltip;
        hideTooltip();

        _currentTooltipView = view;
        Platform::sharedInstance()->cancelDelayedInvokes(this);
        Platform::sharedInstance()->invokeDelayed(
            this,
            [=] {
                auto mousePos = _topView->responderChain()->lastMousePos();
                _tooltip = new Tooltip(view->tooltipText(), makePoint(mousePos.x, mousePos.y - 40.0f));
            },
            !wasTooltipShown ? 1.0f : 0.0f);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TooltipManager::viewMouseDidLeave(View* sender, View* view) {
    if (_currentTooltipView == view) {
        Platform::sharedInstance()->cancelDelayedInvokes(this);
        Platform::sharedInstance()->invokeDelayed(
            this, [=] { hideTooltip(); }, 0.5f);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TooltipManager::viewMouseDidDetectButtonActivity(View* sender, View* view) {
    if (_currentTooltipView == view) hideTooltip();
}
