//
//  ui.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-14.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef UI_H
#define UI_H

#include <script.h>

#include <memory>
#include <string>

#include "any.h"
#include "property.h"
#include "view.h"

namespace MDStudio {

void bindUITables(Script* script);

class UI {
    Script* _script;

    MDStudio::View* _topView;
    MDStudio::Size _contentSize;

   public:
    UI();
    ~UI();

    View* topView() { return _topView; }

    std::shared_ptr<View> findView(const std::string& name);
    std::string findString(const std::string& name);
    std::shared_ptr<Image> findImage(const std::string& name);
    std::shared_ptr<Property> findProperty(const std::string& name);

    bool loadUI(MDStudio::View* view, std::string path, Script::LogFnType logFn = nullptr,
                std::vector<ScriptModule*> additionalScriptModules = {}, bool isDebugging = false);

    MDStudio::Size contentSize() { return _contentSize; }

    std::string lastError() { return _script->lastError(); }
};

}  // namespace MDStudio

#endif  // UI_H
