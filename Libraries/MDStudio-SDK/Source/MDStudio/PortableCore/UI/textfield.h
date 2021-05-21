//
//  textfield.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-08-05.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef TEXTFIELD_H
#define TEXTFIELD_H

#include <functional>
#include <string>

#include "boxview.h"
#include "control.h"
#include "font.h"
#include "scrollview.h"
#include "textview.h"

namespace MDStudio {

class TextField : public Control {
   public:
    typedef std::function<void(TextField* sender, std::string text)> textChangingFnType;
    typedef std::function<void(TextField* sender, std::string text)> textDidChangeFnType;
    typedef std::function<void(TextField* sender)> DidEscapeFnType;

   private:
    std::shared_ptr<TextView> _textView;
    std::shared_ptr<ScrollView> _scrollView;
    std::shared_ptr<BoxView> _boxView;

    textChangingFnType _textChangingFn;
    textDidChangeFnType _textDidChangeFn;
    DidEscapeFnType _didEscapeFn;

    float _borderSize;

    void textChanging(MDStudio::TextView* sender, std::string text);
    void textDidChange(MDStudio::TextView* sender, std::string text);
    void cursorPositionDidChange(MDStudio::TextView* sender, unsigned int line, unsigned int col);
    void mouseSelectionDidBegin(TextView* sender);
    void mouseSelectionDidEnd(TextView* sender);

    void setContentSize();

    void repeatLastEvent();

   public:
    TextField(const std::string& name, void* owner);
    ~TextField();

    void setFrame(Rect frame);

    std::shared_ptr<TextView> textView() { return _textView; }

    void setFont(MultiDPIFont* font) { _textView->setFont(font); }
    void setText(std::string text, bool isDelegateNotified = true);
    std::string text() { return _textView->text(); }
    void selectAllText();

    void setBorderColor(Color color) { _boxView->setBorderColor(color); }
    Color borderColor() { return _boxView->borderColor(); }

    void setBorderSize(float borderSize) { _borderSize = borderSize; }
    float borderSize() { return _borderSize; }

    void setIsEnabled(bool isEnabled) { _textView->setIsEnabled(isEnabled); }
    bool isEnabled(bool isEnabled) { return _textView->isEnabled(); }

    void startEdition() { _textView->startEdition(); }
    void stopEdition() { _textView->stopEdition(); }

    void setTextChangingFn(textChangingFnType textChangingFn) { _textChangingFn = textChangingFn; }
    void setTextDidChangeFn(textDidChangeFnType textDidChangeFn) { _textDidChangeFn = textDidChangeFn; }
    void setDidEscapeFn(DidEscapeFnType didEscapeFn) { _didEscapeFn = didEscapeFn; }
};

}  // namespace MDStudio

#endif  // TEXTFIELD_H
