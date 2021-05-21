//
//  textfield.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-08-05.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#include "textfield.h"

#include "draw.h"
#include "platform.h"
#include "responderchain.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
TextField::TextField(const std::string& name, void* owner) : Control(name, owner) {
    using namespace std::placeholders;

    _textChangingFn = nullptr;
    _textDidChangeFn = nullptr;
    _didEscapeFn = nullptr;

    _borderSize = 0.0f;

    _boxView = std::make_shared<BoxView>("boxView", this);
    _boxView->setBorderColor(zeroColor);
    _boxView->setFillColor(zeroColor);
    addSubview(_boxView);

    _textView = std::make_shared<TextView>("textView", this, true);
    _textView->setTextChangingFn(std::bind(&TextField::textChanging, this, _1, _2));
    _textView->setTextDidChangeFn(std::bind(&TextField::textDidChange, this, _1, _2));
    _textView->setCursorPositionDidChangeFn(std::bind(&TextField::cursorPositionDidChange, this, _1, _2, _3));
    _textView->setMouseSelectionDidBeginFn(std::bind(&TextField::mouseSelectionDidBegin, this, _1));
    _textView->setMouseSelectionDidEndFn(std::bind(&TextField::mouseSelectionDidEnd, this, _1));

    _scrollView = std::make_shared<ScrollView>("scrollView", this, _textView, true);
    _scrollView->setIsHorizScrollBarVisible(false);
    _scrollView->setIsVertScrollBarVisible(false);

    addSubview(_scrollView);
}

// ---------------------------------------------------------------------------------------------------------------------
TextField::~TextField() {
    removeSubview(_scrollView);
    removeSubview(_boxView);
}

// ---------------------------------------------------------------------------------------------------------------------
void TextField::setContentSize() {
    Size size = _textView->contentSize();
    if (size.width < _scrollView->rect().size.width) size.width = _scrollView->rect().size.width;
    _scrollView->setContentSize(size);
}

// ---------------------------------------------------------------------------------------------------------------------
void TextField::setText(std::string text, bool isDelegateNotified) {
    _textView->setText(text, isDelegateNotified);
    if (!isDelegateNotified) setContentSize();
}

// ---------------------------------------------------------------------------------------------------------------------
void TextField::selectAllText() { _textView->selectAllText(); }

// ---------------------------------------------------------------------------------------------------------------------
void TextField::setFrame(Rect frame) {
    View::setFrame(frame);
    _boxView->setFrame(bounds());
    auto r = makeInsetRect(bounds(), _borderSize, _borderSize);
    r = makeCenteredRectInRectLimited(r, r.size.width, fontHeight(_textView->font()));
    _scrollView->setFrame(r);
    setContentSize();
}

// ---------------------------------------------------------------------------------------------------------------------
void TextField::textChanging(MDStudio::TextView* sender, std::string text) {
    setContentSize();

    if (_textChangingFn) _textChangingFn(this, text);
}

// ---------------------------------------------------------------------------------------------------------------------
void TextField::textDidChange(MDStudio::TextView* sender, std::string text) {
    if (_textDidChangeFn) _textDidChangeFn(this, text);
}

// ---------------------------------------------------------------------------------------------------------------------
void TextField::cursorPositionDidChange(MDStudio::TextView* sender, unsigned int line, unsigned int col) {
    _scrollView->scrollToVisibleRect(sender->cursorRect());
}

// ---------------------------------------------------------------------------------------------------------------------
void TextField::repeatLastEvent() {
    MDStudio::UIEvent lastEvent = responderChain()->lastEvent();
    if (lastEvent.type == MDStudio::MOUSE_MOVED_UIEVENT) {
        _textView->handleEvent(&lastEvent);
        _scrollView->scrollToVisibleRect(_textView->selectionCursorRect());
    }
    Platform::sharedInstance()->invokeDelayed(
        this, [=] { repeatLastEvent(); }, 0.01);
}

// ---------------------------------------------------------------------------------------------------------------------
void TextField::mouseSelectionDidBegin(MDStudio::TextView* sender) { repeatLastEvent(); }

// ---------------------------------------------------------------------------------------------------------------------
void TextField::mouseSelectionDidEnd(MDStudio::TextView* sender) {
    Platform::sharedInstance()->cancelDelayedInvokes(this);
}
