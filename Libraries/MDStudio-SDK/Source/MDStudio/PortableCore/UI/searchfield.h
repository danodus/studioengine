//
//  searchfield.h
//  MDStudio
//
//  Created by Daniel Cliche on 2015-03-16.
//  Copyright (c) 2015-2020 Daniel Cliche. All rights reserved.
//

#ifndef SEARCHFIELD_H
#define SEARCHFIELD_H

#include <functional>
#include <memory>

#include "boxview.h"
#include "button.h"
#include "control.h"
#include "textfield.h"

namespace MDStudio {

class SearchField : public Control {
   public:
    typedef std::function<void(SearchField* sender, std::string text)> TextChangingFnType;
    typedef std::function<void(SearchField* sender, std::string text)> TextDidChangeFnType;

   private:
    std::shared_ptr<TextField> _textField;
    std::shared_ptr<BoxView> _boxView;
    std::shared_ptr<Button> _resetButton;

    TextChangingFnType _textChangingFn;
    TextDidChangeFnType _textDidChangeFn;

    void textFieldTextChanging(TextField* sender, std::string text);
    void textFieldTextDidChange(TextField* sender, std::string text);
    void textFieldDidEscape(TextField* sender);
    void resetButtonClicked(Button* sender);

   public:
    SearchField(const std::string& name, void* owner);
    ~SearchField();

    void setFrame(Rect frame);

    std::shared_ptr<TextField> textField() { return _textField; }

    void setText(std::string text, bool isDelegateNotified = true) { _textField->setText(text, isDelegateNotified); }
    std::string text() { return _textField->text(); }

    void setFont(MultiDPIFont* font) { _textField->setFont(font); }
    void setCornerRadius(float cornerRadius) { _boxView->setCornerRadius(cornerRadius); }

    void startEdition() { _textField->startEdition(); }
    void stopEdition() { _textField->stopEdition(); }

    void setTextChangingFn(TextChangingFnType textChangingFn) { _textChangingFn = textChangingFn; }
    void setTextDidChangeFn(TextDidChangeFnType textDidChangeFn) { _textDidChangeFn = textDidChangeFn; }
};
}  // namespace MDStudio

#endif  // SEARCHFIELD_H
