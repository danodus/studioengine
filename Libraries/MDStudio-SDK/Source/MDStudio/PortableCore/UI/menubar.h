//
//  menubar.h
//  MDStudio
//
//  Created by Daniel Cliche on 2020-12-07.
//  Copyright (c) 2020-2021 Daniel Cliche. All rights reserved.
//

#ifndef MENUBAR_H
#define MENUBAR_H

#include "boxview.h"
#include "button.h"
#include "menu.h"
#include "view.h"

namespace MDStudio {
class MenuBar : public View {
    std::vector<std::shared_ptr<Menu>> _menus;
    std::vector<std::shared_ptr<Button>> _menuButtons;

    std::shared_ptr<BoxView> _boxView;

   public:
    MenuBar(const std::string& name, void* owner);

    void addMenu(std::shared_ptr<Menu> menu);
    void removeAllMenus();

    void setFrame(Rect rect) override;
};

}  // namespace MDStudio

#endif  // MENUBAR_H