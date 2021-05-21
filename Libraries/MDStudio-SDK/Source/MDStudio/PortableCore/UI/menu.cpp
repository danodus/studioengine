//
//  menu.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2016-09-10.
//  Copyright (c) 2016-2021 Daniel Cliche. All rights reserved.
//

#include "menu.h"

#include "draw.h"
#include "listitemview.h"

using namespace MDStudio;

std::unique_ptr<Window> Menu::_window;
int Menu::_highlightedItemIndex = -1;

// ---------------------------------------------------------------------------------------------------------------------
Menu::Menu(const std::string& name, void* owner, const std::string& title) : View(name, owner), _title(title) {
    _didSelectItemFn = nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------
Menu::~Menu() {}

// ---------------------------------------------------------------------------------------------------------------------
void Menu::addMenuItem(std::shared_ptr<MenuItem> menuItem) { _menuItems.push_back(menuItem); }

// ---------------------------------------------------------------------------------------------------------------------
Size Menu::contentSize() {
    float width = 0.0f;

    for (auto menuItem : _menuItems) {
        float w = getTextWidth(SystemFonts::sharedInstance()->semiboldFont(), menuItem->title());
        if (w > width) width = w;
    }

    return makeSize(width + 20.0f, static_cast<float>(_menuItems.size()) * 20.0f + 2.0f);
}

// ---------------------------------------------------------------------------------------------------------------------
void Menu::setFrame(Rect frame) { View::setFrame(frame); }

// ---------------------------------------------------------------------------------------------------------------------
void Menu::draw() {
    auto dc = drawContext();
    dc->pushStates();
    dc->setStrokeColor(whiteColor);
    dc->setStrokeWidth(1.0f);
    dc->setFillColor(blackColor);
    dc->drawRect(bounds());
    auto r = makeRect(0, bounds().size.height - 20.0f, bounds().size.width, 20.0f);
    r = makeInsetRect(r, 1.0f, 1.0f);
    int itemIndex = 0;
    for (auto menuItem : _menuItems) {
        if (itemIndex == _highlightedItemIndex) {
            dc->pushStates();
            dc->setStrokeColor(zeroColor);
            dc->setFillColor(blueColor);
            dc->drawRect(r);
            dc->popStates();
        }
        dc->drawLeftText(SystemFonts::sharedInstance()->semiboldFont(), makeInsetRect(r, 10.0f, 0.0f),
                         menuItem->title());
        r.origin.y -= 20.0f;
        ++itemIndex;
    }
    dc->popStates();
}

// ---------------------------------------------------------------------------------------------------------------------
bool Menu::handleEvent(const UIEvent* event) {
    if (isPointInRect(event->pt, clippedRect())) {
        if (event->type == MOUSE_MOVED_UIEVENT) {
            _highlightedItemIndex =
                static_cast<unsigned int>(_menuItems.size()) - (event->pt.y - rect().origin.y) / 20.0f;
            setDirty();
        } else if (event->type == MOUSE_DOWN_UIEVENT) {
            return true;
        } else if (event->type == MOUSE_UP_UIEVENT) {
            unsigned int itemIndex =
                static_cast<unsigned int>(_menuItems.size()) - (event->pt.y - rect().origin.y) / 20.0f;
            if (_didSelectItemFn) _didSelectItemFn(this, itemIndex);
            return true;
        }
    } else {
        if (event->type == MOUSE_MOVED_UIEVENT && _highlightedItemIndex >= 0) {
            _highlightedItemIndex = -1;
            setDirty();
        }
    }

    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
void Menu::popUpContextMenu(std::shared_ptr<Menu> menu, Point location, View* view) {
    _window = std::make_unique<Window>();
    Size size = menu->contentSize();

    _highlightedItemIndex = -1;
    _window->contentView()->addSubview(menu);
    _window->contentView()->setLayoutFn([menu](View* sender, Rect frame) { menu->setFrame(sender->bounds()); });
    menu->setDisposeFn([](View* sender) { Menu::closePopUp(); });
    _window->setDidResignKeyWindowFn([](Window* sender) { Menu::closePopUp(); });
    auto viewOrigin = view->rect().origin;
    _window->open(makeRect(location.x + viewOrigin.x, location.y + viewOrigin.y - size.height, size.width, size.height),
                  true);
}

// ---------------------------------------------------------------------------------------------------------------------
void Menu::closePopUp() {
    if (_window) {
        _window->close();
        _window = nullptr;
    }
}
