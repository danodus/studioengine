//
//  listitemview.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-07-12.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef LISTITEMVIEW_H
#define LISTITEMVIEW_H

#include <string>

#include "font.h"
#include "view.h"

namespace MDStudio {

class ListItemView : public View {
    MultiDPIFont* _font;
    Color _textColor;
    std::string _title;
    bool _isHovering;
    bool _isHighlighted;
    bool _hasFocus;

    void draw() override;

   public:
    ListItemView(const std::string& name, void* owner, const std::string& title);

    std::string title() { return _title; }

    void setFont(MultiDPIFont* font) { _font = font; }
    void setTextColor(Color color) { _textColor = color; }
    void setIsHovering(bool isHovering);
    void setIsHighlighted(bool isHighlighted);
    void setFocusState(bool focusState);
    bool isHovering() { return _isHovering; }
    bool isHighlighted() { return _isHighlighted; }
};

}  // namespace MDStudio

#endif  // LISTITEMVIEW_H
