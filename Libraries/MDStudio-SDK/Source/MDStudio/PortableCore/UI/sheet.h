//
//  sheet.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-07-26.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef SHEET_H
#define SHEET_H

#include "boxview.h"
#include "view.h"

namespace MDStudio {

class Sheet : public View {
    std::shared_ptr<BoxView> _boxView;
    bool _modal;

   protected:
    bool handleEvent(const UIEvent* event) override;

   public:
    Sheet(const std::string& name, void* owner, bool modal = true);
    ~Sheet();

    void setFrame(Rect rect) override;
};

}  // namespace MDStudio

#endif  // SHEET_H
