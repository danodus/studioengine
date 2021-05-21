//
//  searchfield.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2015-03-16.
//  Copyright (c) 2015-2020 Daniel Cliche. All rights reserved.
//

#include "searchfield.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
SearchField::SearchField(const std::string& name, void* owner) : Control(name, owner) {
    using namespace std::placeholders;

    _textChangingFn = nullptr;
    _textDidChangeFn = nullptr;

    _boxView = std::shared_ptr<BoxView>(new BoxView("searchFieldBoxView", owner));
    _boxView->setCornerRadius(10.0f);
    addSubview(_boxView);

    _textField = std::shared_ptr<TextField>(new TextField("searchFieldTextField", owner));
    _textField->setTextChangingFn(std::bind(&SearchField::textFieldTextChanging, this, _1, _2));
    _textField->setTextDidChangeFn(std::bind(&SearchField::textFieldTextDidChange, this, _1, _2));
    _textField->setDidEscapeFn(std::bind(&SearchField::textFieldDidEscape, this, _1));
    addSubview(_textField);

    _resetButton = std::shared_ptr<Button>(
        new Button("searchFieldResetButton", owner, "", SystemImages::sharedInstance()->crossCircleImage()));
    _resetButton->setBorderColor(zeroColor);
    _resetButton->setClickedFn(std::bind(&SearchField::resetButtonClicked, this, _1));
    addSubview(_resetButton);
}

// ---------------------------------------------------------------------------------------------------------------------
SearchField::~SearchField() {
    removeSubview(_boxView);
    removeSubview(_textField);
    removeSubview(_resetButton);
}

// ---------------------------------------------------------------------------------------------------------------------
void SearchField::setFrame(Rect frame) {
    Control::setFrame(frame);

    Rect r = bounds();
    _boxView->setFrame(r);

    float cornerRadius = _boxView->cornerRadius();
    _textField->setFrame(makeRect(cornerRadius > 0 ? cornerRadius : 1.0f, 1.0f, r.size.width - (25.0f + cornerRadius),
                                  r.size.height - 2.0f));
    _resetButton->setFrame(makeRect(r.size.width - 21.0f, 0.0f, 20.0f, r.size.height));
}

// ---------------------------------------------------------------------------------------------------------------------
void SearchField::textFieldTextChanging(TextField* sender, std::string text) {
    if (_textChangingFn) _textChangingFn(this, text);
}

// ---------------------------------------------------------------------------------------------------------------------
void SearchField::textFieldTextDidChange(TextField* sender, std::string text) {
    if (_textDidChangeFn) _textDidChangeFn(this, text);
}

// ---------------------------------------------------------------------------------------------------------------------
void SearchField::textFieldDidEscape(TextField* sender) { _textField->setText(""); }

// ---------------------------------------------------------------------------------------------------------------------
void SearchField::resetButtonClicked(Button* sender) { _textField->setText(""); }
