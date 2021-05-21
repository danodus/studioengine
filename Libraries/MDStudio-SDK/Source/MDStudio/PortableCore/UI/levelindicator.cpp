//
//  levelindicator.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-09-13.
//  Copyright (c) 2014-2019 Daniel Cliche. All rights reserved.
//

#include "levelindicator.h"

#include <math.h>

#include "draw.h"
#include "responderchain.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
LevelIndicator::LevelIndicator(std::string name, void* owner) : Control(name, owner) {
    _isCaptured = false;
    _level = 0.0f;
    _levelDidChangeFn = nullptr;

    for (int i = 0; i < 5; ++i) {
        _starImageViews[i] = std::shared_ptr<ImageView>(new ImageView(
            "starImageView" + std::to_string(i), owner, SystemImages::sharedInstance()->starEmptyImage()));
        addSubview(_starImageViews[i]);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
LevelIndicator::~LevelIndicator() {
    for (int i = 0; i < 5; ++i) {
        removeSubview(_starImageViews[i]);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool LevelIndicator::handleEvent(const UIEvent* event) {
    if ((event->type == MOUSE_DOWN_UIEVENT || event->type == MOUSE_MOVED_UIEVENT) &&
        (_isCaptured || isPointInRect(event->pt, resolvedClippedRect()))) {
        if (!_isCaptured && event->type == MOUSE_DOWN_UIEVENT) _isCaptured = responderChain()->captureResponder(this);
        if (_isCaptured) {
            float f = event->pt.x - resolvedClippedRect().origin.x + rect().size.width / 10.0f;
            setLevel(roundf(f / rect().size.width * 5.0f) / 5.0f, false);
            return true;
        } else {
            return false;
        }
    } else if (event->type == MOUSE_UP_UIEVENT && _isCaptured) {
        responderChain()->releaseResponder(this);
        _isCaptured = false;
        if (_levelDidChangeFn) _levelDidChangeFn(this, _level);
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
void LevelIndicator::setFrame(Rect aRect) {
    Control::setFrame(aRect);
    float width = bounds().size.width / 5.0f;
    for (int i = 0; i < 5; ++i) {
        _starImageViews[i]->setFrame(makeRect((float)i * width, 0.0f, width, bounds().size.height));
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void LevelIndicator::setLevel(float level, bool notifyDelegate) {
    if (level == _level) return;

    if (level < 0.0f) {
        level = 0.0f;
    } else if (level > 1.0f) {
        level = 1.0f;
    }

    _level = level;
    for (int i = 0; i < 5; i++) {
        _starImageViews[i]->setImage(_level >= (float)((i + 1) / 5.0f)
                                         ? SystemImages::sharedInstance()->starFilledImage()
                                         : SystemImages::sharedInstance()->starEmptyImage());
    }
    if (notifyDelegate && _levelDidChangeFn) _levelDidChangeFn(this, _level);
}
