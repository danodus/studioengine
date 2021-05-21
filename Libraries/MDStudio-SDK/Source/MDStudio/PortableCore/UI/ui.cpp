//
//  ui.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-14.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#include "ui.h"

#include <assert.h>
#include <platform.h>

#include "menu.h"
#include "uiscriptmodule.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
UI::UI() {
    _script = nullptr;
    _topView = nullptr;
    _contentSize = makeZeroSize();
}

// ---------------------------------------------------------------------------------------------------------------------
UI::~UI() {
    Menu::closePopUp();

    if (_topView) {
        _topView->removeAllSubviews();
        _topView->setLayoutFn(nullptr);
        _topView->setDrawFn(nullptr);
        _topView->setHandleEventFn(nullptr);
    }

    if (_script) delete _script;
}

// ---------------------------------------------------------------------------------------------------------------------
bool UI::loadUI(MDStudio::View* view, std::string path, Script::LogFnType logFn,
                std::vector<ScriptModule*> additionalScriptModules, bool isDebugging) {
    if (_script) {
        Menu::closePopUp();

        // First, we process the remaining invokes before closing Lua
        Platform::sharedInstance()->process();

        if (_topView) {
            _topView->removeAllSubviews();
            _topView->setLayoutFn(nullptr);
            _topView->setDrawFn(nullptr);
            _topView->setHandleEventFn(nullptr);
        }

        delete (_script);
        _script = nullptr;
    }

    _script = new Script();
    if (logFn) _script->setLogFn(logFn);
    _script->setDidTerminateFn([=](Script* sender) {
        _topView->removeAllSubviews();
        _topView->setLayoutFn(nullptr);
        _topView->setDrawFn(nullptr);
        _topView->setHandleEventFn(nullptr);
        _topView->setDirty();
    });

    if (_topView) {
        _topView->removeAllSubviews();
        _topView->setLayoutFn(nullptr);
        _topView->setDrawFn(nullptr);
        _topView->setHandleEventFn(nullptr);
    }

    _topView = view;
    if (_topView) _topView->setOwner(nullptr);

    UIScriptModule uiScriptModule(_topView);

    std::vector<ScriptModule*> scriptModules = {&uiScriptModule};
    for (auto additionalScriptModule : additionalScriptModules) scriptModules.push_back(additionalScriptModule);

    if (!_script->execute(path, scriptModules, isDebugging)) {
        if (_topView) {
            _topView->removeAllSubviews();
            _topView->setLayoutFn(nullptr);
            _topView->setDrawFn(nullptr);
            _topView->setHandleEventFn(nullptr);
        }
        return false;
    }

    _contentSize = uiScriptModule.contentSize();

    if (_topView) _topView->setDirty();

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<View> UI::findView(const std::string& name) { return _script->findElement<View>(name); }

// ---------------------------------------------------------------------------------------------------------------------
std::string UI::findString(const std::string& name) { return _script->findString(name); }

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<Image> UI::findImage(const std::string& name) { return _script->findElement<Image>(name); }

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<Property> UI::findProperty(const std::string& name) { return _script->findElement<Property>(name); }
