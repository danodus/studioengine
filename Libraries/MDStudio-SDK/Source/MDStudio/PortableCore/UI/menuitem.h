//
//  menuitem.h
//  MDStudio
//
//  Created by Daniel Cliche on 2016-09-10.
//  Copyright (c) 2016-2020 Daniel Cliche. All rights reserved.
//

#ifndef MENUITEM_H
#define MENUITEM_H

#include <string>

namespace MDStudio {
class MenuItem {
    std::string _title;

   public:
    MenuItem(const std::string& title);

    std::string title() { return _title; }
};

}  // namespace MDStudio

#endif  // MENUITEM_H
