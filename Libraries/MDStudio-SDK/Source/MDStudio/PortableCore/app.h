//
//  app.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-09-08.
//  Copyright (c) 2014-2018 Daniel Cliche. All rights reserved.
//

#ifndef APP_H
#define APP_H

#include <view.h>

#include <memory>
#include <string>

namespace MDStudio {

class App {
    std::string _title;

   public:
    App(std::string title);
    virtual ~App() = default;
    virtual std::shared_ptr<MDStudio::View> topView() = 0;
    std::string title() { return _title; }
};

void initPlatform();
int run(App* app);

}  // namespace MDStudio

#endif  // APP_H
