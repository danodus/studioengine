//
//  labelview.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-07-15.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef LABELVIEW_H
#define LABELVIEW_H

#include <string>

#include "font.h"
#include "size.h"
#include "view.h"

namespace MDStudio {

class LabelView : public View {
   public:
    typedef enum { LeftTextAlign, CenterTextAlign, RightTextAlign } TextAlignEnumType;

   private:
    MultiDPIFont* _font;
    std::string _title;

    void draw() override;

    Color _borderColor, _fillColor, _textColor;

    std::vector<std::string> _lines;

    TextAlignEnumType _textAlign;

   public:
    LabelView(const std::string& name, void* owner, std::string title);

    void setFont(MultiDPIFont* font) { _font = font; }
    std::string title() { return _title; }
    void setTitle(std::string title);

    Color borderColor() { return _borderColor; }
    void setBorderColor(Color borderColor) { _borderColor = borderColor; }

    Color fillColor() { return _fillColor; }
    void setFillColor(Color fillColor) { _fillColor = fillColor; }

    Color textColor() { return _textColor; }
    void setTextColor(Color textColor) { _textColor = textColor; }

    Size contentSize();

    TextAlignEnumType textAlign() { return _textAlign; }
    void setTextAlign(TextAlignEnumType textAlign) { _textAlign = textAlign; }
};

}  // namespace MDStudio

#endif  // LABELVIEW_H
