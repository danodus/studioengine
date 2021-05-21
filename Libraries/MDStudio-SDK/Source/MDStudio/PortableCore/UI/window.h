//
//  window.h
//  MDStudio
//
//  Created by Daniel Cliche on 2017-05-29.
//  Copyright (c) 2017-2020 Daniel Cliche. All rights reserved.
//

#ifndef WINDOW_H
#define WINDOW_H

#include "view.h"

namespace MDStudio {

class Window {
   public:
    typedef std::function<void(Window* sender)> DidResignKeyWindowFnType;

   private:
    View* _contentView;

    bool _isOpened;

    DidResignKeyWindowFnType _didResignKeyWindowFn;

   public:
    Window();
    ~Window();

    void open(Rect rect, bool isKeyWindow);
    void makeKeyWindow();
    void close();

    View* contentView() { return _contentView; }

    void setDidResignKeyWindowFn(DidResignKeyWindowFnType didResignKeyWindowFn) {
        _didResignKeyWindowFn = didResignKeyWindowFn;
    }

    // Platform access only
    void sendDidResignKeyWindow();
};

}  // namespace MDStudio

#endif  // WINDOW_H
