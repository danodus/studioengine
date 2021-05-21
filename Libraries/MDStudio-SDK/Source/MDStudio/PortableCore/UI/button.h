//
//  Button.h
//  MDStudio
//
//  Created by Daniel Cliche on 2013-09-09.
//  Copyright (c) 2013-2020 Daniel Cliche. All rights reserved.
//

#ifndef BUTTON_H
#define BUTTON_H

#include <functional>
#include <string>

#include "control.h"
#include "font.h"
#include "image.h"

namespace MDStudio {

class Button : public Control {
   public:
    typedef enum {
        StandardButtonType,
        CheckBoxButtonType,
        CustomCheckBoxButtonType,
        RadioButtonType,
        OKButtonType,
        CancelButtonType,
        ComboBoxUpButtonType,
        ComboBoxDownButtonType,
        SegmentedControlButtonType,
        DisclosureButtonType,
        SortButtonType
    } ButtonEnumType;

    typedef std::function<void(Button* sender)> clickedFnType;
    typedef std::function<void(Button* sender, bool state)> stateDidChangeFnType;
    typedef std::function<void(Button* sender, bool state, unsigned int modifierFlags)>
        stateWithModifierDidChangeFnType;

   private:
    std::string _title;
    MultiDPIFont* _font;
    std::shared_ptr<Image> _image;
    ButtonEnumType _type;
    bool _state;
    bool _isCaptured;
    bool _isHighlighted;
    bool _isEnabled;
    bool _isSortEnabled;
    Color _borderColor, _highlightColor, _textColor;

    clickedFnType _clickedFn;
    stateDidChangeFnType _stateDidChangeFn;
    stateWithModifierDidChangeFnType _stateWithModifierDidChangeFn;

   protected:
    void draw() override;
    bool handleEvent(const UIEvent* event) override;

   public:
    Button(const std::string& name, void* owner, const std::string& title, std::shared_ptr<Image> image = nullptr);
    ~Button();

    void setFont(MultiDPIFont* font) { _font = font; }
    void setType(ButtonEnumType type) {
        _type = type;
        setDirty();
    }
    ButtonEnumType type() { return _type; }

    void setClickedFn(clickedFnType clickedFn) { _clickedFn = clickedFn; }
    void setStateDidChangeFn(stateDidChangeFnType stateDidChange) { _stateDidChangeFn = stateDidChange; }
    void setStateWithModifierDidChangeFn(stateWithModifierDidChangeFnType stateWithModifierDidChange) {
        _stateWithModifierDidChangeFn = stateWithModifierDidChange;
    }

    void setTitle(const std::string& title) {
        _title = title;
        setDirty();
    }
    std::string title() { return _title; }

    void setImage(std::shared_ptr<Image> image) {
        _image = image;
        setDirty();
    }

    bool state() { return _state; }
    void setState(bool state, bool isDelegateNotified = true, unsigned int modifierFlags = 0);

    bool isEnabled() { return _isEnabled; }
    void setIsEnabled(bool isEnabled) {
        _isEnabled = isEnabled;
        setDirty();
    }

    bool isSortEnabled() { return _isSortEnabled; }
    void setIsSortEnabled(bool isSortEnabled) {
        _isSortEnabled = isSortEnabled;
        setDirty();
    }

    Color borderColor() { return _borderColor; }
    void setBorderColor(Color borderColor) { _borderColor = borderColor; }

    Color highlightColor() { return _highlightColor; }
    void setHighlightColor(Color highlightColor) { _highlightColor = highlightColor; }

    Color textColor() { return _textColor; }
    void setTextColor(Color textColor) { _textColor = textColor; }
};

}  // namespace MDStudio

#endif  // BUTTON_H
