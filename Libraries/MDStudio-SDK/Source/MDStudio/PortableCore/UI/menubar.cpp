//
//  menubar.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2020-12-07.
//  Copyright (c) 2020-2021 Daniel Cliche. All rights reserved.
//

#include "menubar.h"

#include "draw.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
MenuBar::MenuBar(const std::string& name, void* owner) : View(name, owner) {
    _boxView = std::make_shared<BoxView>("menuBarBoxView", this);
    _boxView->setBorderColor(zeroColor);

    addSubview(_boxView);
}

// ---------------------------------------------------------------------------------------------------------------------
void MenuBar::addMenu(std::shared_ptr<Menu> menu) {
    _menus.emplace_back(menu);

    auto menuButton = std::make_shared<Button>(menu->title() + "Button", this, menu->title());
    menuButton->setBorderColor(zeroColor);
    menuButton->setClickedFn([menu](Button* sender) {
        auto pt = sender->frame().origin;
        Menu::popUpContextMenu(menu, pt, sender->superview());
    });
    _menuButtons.emplace_back(menuButton);
    addSubview(menuButton);
}

// ---------------------------------------------------------------------------------------------------------------------
void MenuBar::removeAllMenus() {
    _menus.clear();
    for (auto menuButton : _menuButtons) removeSubview(menuButton);
    _menuButtons.clear();
}

// ---------------------------------------------------------------------------------------------------------------------
void MenuBar::setFrame(Rect aRect) {
    View::setFrame(aRect);
    auto r = bounds();
    _boxView->setFrame(r);
    for (auto menuButton : _menuButtons) {
        r.size.width = getTextWidth(SystemFonts::sharedInstance()->semiboldFont(), menuButton->title());
        menuButton->setFrame(r);
        r.origin.x += r.size.width + 10.0f;
    }
}