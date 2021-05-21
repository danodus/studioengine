//
//  menu.h
//  MDStudio
//
//  Created by Daniel Cliche on 2016-09-10.
//  Copyright (c) 2016-2021 Daniel Cliche. All rights reserved.
//

#ifndef MENU_H
#define MENU_H

#include <vector>

#include "boxview.h"
#include "listview.h"
#include "menuitem.h"
#include "view.h"
#include "window.h"

namespace MDStudio {

class Menu : public View {
   public:
    typedef std::function<void(Menu* sender, int itemIndex)> DidSelectItemFn;

   private:
    std::vector<std::shared_ptr<MenuItem>> _menuItems;

    DidSelectItemFn _didSelectItemFn;

    static std::unique_ptr<Window> _window;

    std::string _title;

    static int _highlightedItemIndex;

   public:
    Menu(const std::string& name, void* owner, const std::string& title);
    ~Menu();

    void setFrame(Rect frame) override;
    void draw() override;
    bool handleEvent(const UIEvent* event) override;

    void addMenuItem(std::shared_ptr<MenuItem> menuItem);

    Size contentSize();

    static void popUpContextMenu(std::shared_ptr<Menu> menu, Point location, View* view);
    static void closePopUp();

    std::string title() { return _title; }

    void setDidSelectItemFn(DidSelectItemFn didSelectItemFn) { _didSelectItemFn = didSelectItemFn; }
};

}  // namespace MDStudio

#endif  // MENU_H
