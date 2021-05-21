//
//  boxview.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-07-26.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef BOXVIEW_H
#define BOXVIEW_H

#include "view.h"

namespace MDStudio {

class BoxView : public View {
    Color _bottomFillColor, _topFillColor, _borderColor;
    float _cornerRadius;

   protected:
    void draw() override;

   public:
    BoxView(const std::string& name, void* owner);

    void setFillColors(Color bottomFillColor, Color topFillColor);
    void setFillColor(Color fillColor) { setFillColors(fillColor, fillColor); }
    Color bottomFillColor() { return _bottomFillColor; }
    Color topFillColor() { return _topFillColor; }

    void setBorderColor(Color borderColor);
    Color borderColor() { return _borderColor; }

    float cornerRadius() { return _cornerRadius; }
    void setCornerRadius(float cornerRadius) {
        _cornerRadius = cornerRadius;
        setDirty();
    }
};

}  // namespace MDStudio

#endif  // BOXVIEW_H
