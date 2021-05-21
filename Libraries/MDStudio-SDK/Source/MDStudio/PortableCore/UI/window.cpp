//
//  window.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2017-05-29.
//  Copyright (c) 2017-2020 Daniel Cliche. All rights reserved.
//

#include "window.h"

#include <platform.h>

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
Window::Window() {
    _isOpened = false;
    _didResignKeyWindowFn = nullptr;
    _contentView = new View("contentView", this);
    _contentView->createResponderChain();
    _contentView->createTooltipManager();
}

// ---------------------------------------------------------------------------------------------------------------------
void Window::open(Rect rect, bool isKeyWindow) {
    Platform::sharedInstance()->createWindow(this, rect.origin.x, rect.origin.y, rect.size.width, rect.size.height,
                                             _contentView, isKeyWindow);
    _contentView->setFrame(makeRect(0.0f, 0.0f, rect.size.width, rect.size.height));
    _contentView->updateResponderChain();
    _contentView->updateTooltips();
    _contentView->setDirtyAll();
    _isOpened = true;
}

// ---------------------------------------------------------------------------------------------------------------------
void Window::makeKeyWindow() { Platform::sharedInstance()->makeKeyWindow(this); }

// ---------------------------------------------------------------------------------------------------------------------
void Window::close() {
    Platform::sharedInstance()->destroyWindow(this);
    _isOpened = false;
}

// ---------------------------------------------------------------------------------------------------------------------
Window::~Window() {
    if (_isOpened) close();

    _contentView->removeAllSubviews();
    delete _contentView;
}

// ---------------------------------------------------------------------------------------------------------------------
void Window::sendDidResignKeyWindow() {
    if (_didResignKeyWindowFn) _didResignKeyWindowFn(this);
}
