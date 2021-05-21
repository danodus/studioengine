//
//  script.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2018-04-07.
//  Copyright (c) 2018-2021 Daniel Cliche. All rights reserved.
//

#include "script.h"

#include <iostream>
#include <lrdb/server.hpp>
#include <sstream>
#include <typeinfo>

#include "any.h"
#include "platform.h"
#include "property.h"
#include "undomanager.h"

#ifndef _WIN32
#include <unistd.h>
#else
#include <direct.h>
#endif

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
void printValue(lua_State* L, int i) {
    int t = lua_type(L, i);
    switch (t) {
        case LUA_TSTRING:
            std::cout << "'" << lua_tostring(L, i) << "'";
            break;
        case LUA_TBOOLEAN:
            std::cout << (lua_toboolean(L, i) ? "true" : "false");
            break;
        case LUA_TNUMBER:
            std::cout << lua_tonumber(L, i);
            break;
        case LUA_TLIGHTUSERDATA:
            std::cout << "<Light User Data>";
            break;
        default:
            std::cout << lua_typename(L, i);
            break;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void dumpStack(lua_State* L) {
    int i;
    int top = lua_gettop(L);
    for (i = 1; i <= top; ++i) {
        int t = lua_type(L, i);
        switch (t) {
            case LUA_TTABLE: {
                lua_pushnil(L);
                std::cout << "{";
                while (lua_next(L, i)) {
                    // Uses 'key' (index -2) and 'value' (index -1)
                    printValue(L, -2);
                    std::cout << "=";
                    printValue(L, -1);
                    std::cout << " ";
                    // Removes 'value'; keep 'key' for the next iteration
                    lua_pop(L, 1);
                }
                std::cout << "}";
                break;
            }

            default:
                printValue(L, i);
                break;
        }
        std::cout << "  ";
    }
    std::cout << "\n";
}

// =====================================================================================================================
// Core functions
// =====================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------
static int invoke(lua_State* L) {
    // Store the reference to the Lua function at the top of the stack in a variable to be used later
    int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_getglobal(L, "script");
    MDStudio::Script* script = (MDStudio::Script*)lua_touserdata(L, -1);

    MDStudio::Platform::sharedInstance()->invoke([=](void) {
        script->callCallback(L, callbackRef);
        luaL_unref(L, LUA_REGISTRYINDEX, callbackRef);
    });

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
static int invokeDelayed(lua_State* L) {
    // Stack: callback, delay

    lua_Number delay = luaL_checknumber(L, 2);

    // Move the callback to the top of the stack
    lua_pushvalue(L, 1);
    lua_remove(L, 1);

    // Stack: delay, callback

    // Store the reference to the Lua function at the top of the stack in a variable to be used later
    int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_getglobal(L, "script");
    MDStudio::Script* script = (MDStudio::Script*)lua_touserdata(L, -1);

    MDStudio::Platform::sharedInstance()->invokeDelayed(
        script,
        [=](void) {
            script->callCallback(L, callbackRef);
            luaL_unref(L, LUA_REGISTRYINDEX, callbackRef);
        },
        delay);

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<lua_Number> MDStudio::getNumbers(lua_State* L, int index) {
    std::vector<lua_Number> v;

    lua_pushnil(L);
    while (lua_next(L, index)) {
        // Key: -2, Value: -1
        v.push_back(luaL_checknumber(L, -1));
        lua_pop(L, 1);
    }
    return v;
}

// ---------------------------------------------------------------------------------------------------------------------
static int operatingSystem(lua_State* L) {
    lua_pushstring(L, MDStudio::Platform::sharedInstance()->operatingSystem().c_str());
    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int language(lua_State* L) {
    lua_pushstring(L, MDStudio::Platform::sharedInstance()->language().c_str());
    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int appVersion(lua_State* L) {
    lua_pushstring(L, MDStudio::Platform::sharedInstance()->appVersion().c_str());
    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int appCoreVersion(lua_State* L) {
    lua_pushstring(L, MDStudio::Platform::sharedInstance()->appCoreVersion().c_str());
    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int licenseName(lua_State* L) {
    lua_pushstring(L, MDStudio::Platform::sharedInstance()->licenseName().c_str());
    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int studioVersion(lua_State* L) {
    lua_pushstring(L, MDStudio::Platform::sharedInstance()->studioVersion().c_str());
    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int dataPath(lua_State* L) {
    lua_pushstring(L, MDStudio::Platform::sharedInstance()->dataPath().c_str());
    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
Script::Script() {
    _logFn = nullptr;
    _didTerminateFn = nullptr;
    _debugServer = nullptr;
    _L = nullptr;
    _isExecutionCritical = false;
}

// ---------------------------------------------------------------------------------------------------------------------
Script::~Script() {
    if (_L) {
        auto debugServer = reinterpret_cast<lrdb::server*>(_debugServer);
        if (debugServer) debugServer->reset();  // unassign debug server (Required before lua_close )
        lua_close(_L);
    }

    if (_debugServer) {
        auto debugServer = reinterpret_cast<lrdb::server*>(_debugServer);
        delete debugServer;
    }

    MDStudio::Platform::sharedInstance()->cancelDelayedInvokes(this);
}

// ---------------------------------------------------------------------------------------------------------------------
// Calls our Lua callback function
void Script::callCallback(lua_State* L, int callbackRef) {
    // Push the callback onto the stack using the Lua reference we stored in the registry
    lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);

    // Call the callback
    // NOTE: This is using the one we duplicated with lua_pushvalue

    if (0 != lua_pcall(L, 0, 0, 0)) {
        error(lua_tostring(L, -1));
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Script::setGlobal(const char* name, void* p) {
    lua_pushlightuserdata(_L, p);
    lua_setglobal(_L, name);
}

// ---------------------------------------------------------------------------------------------------------------------
void Script::bindFunction(const char* name, lua_CFunction fn) {
    lua_pushcfunction(_L, fn);
    lua_setglobal(_L, name);
}

// ---------------------------------------------------------------------------------------------------------------------
bool Script::execute(std::string path, std::vector<ScriptModule*> scriptModules, bool isDebugging,
                     int debugServerPort) {
    if (_L) {
        auto debugServer = reinterpret_cast<lrdb::server*>(_debugServer);
        if (debugServer) debugServer->reset();  // unassign debug server (Required before lua_close )
        lua_close(_L);
    }

    MDStudio::Platform::sharedInstance()->cancelDelayedInvokes(this);

    _L = luaL_newstate();

    if (isDebugging) {
        auto debugServer = new lrdb::server(debugServerPort);
        debugServer->reset(_L);  // assign debug server to lua state(Required before script load)
        _debugServer = debugServer;
    }

    lua_gc(_L, LUA_GCSTOP, 0); /* stop collector during initialization */
    luaL_openlibs(_L);         /* open libraries */
#if TARGET_OS_MAC
    luaopen_lfs(_L);  // Open Lua File System
#endif
    lua_gc(_L, LUA_GCRESTART, 0);

    // Bind core functions
    setGlobal("script", this);
    bindFunction("log", [](lua_State* L) -> int {
        lua_getglobal(L, "script");
        MDStudio::Script* script = (MDStudio::Script*)lua_touserdata(L, -1);
        const char* s = luaL_checkstring(L, 1);
        script->log(s);
        return 0;
    });
    bindFunction("invoke", &invoke);
    bindFunction("invokeDelayed", &invokeDelayed);
    bindFunction("operatingSystem", &operatingSystem);
    bindFunction("language", &language);
    bindFunction("appVersion", &appVersion);
    bindFunction("appCoreVersion", &appCoreVersion);
    bindFunction("licenseName", &licenseName);
    bindFunction("studioVersion", &studioVersion);
    bindFunction("dataPath", &dataPath);
    bindFunction("getTimestamp", [](lua_State* L) -> int {
        lua_pushnumber(L, getTimestamp());
        return 1;
    });

    // UndoManager
    struct luaL_Reg undoManagerTableDefinition[] = {
        {"new",
         [](lua_State* L) -> int {
             auto undoManager = std::make_shared<UndoManager>();
             registerElement<UndoManager>(L, undoManager);

             return 1;
         }},
        {"__gc", destroyElement<UndoManager>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<UndoManager>(L, 1);
             auto e2 = getElement<UndoManager>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},

        {NULL, NULL}  // Sentinel value
    };
    bindTable<UndoManager>("UndoManager", undoManagerTableDefinition);

    // Property
    struct luaL_Reg propertyTableDefinition[] = {
        {"new",
         [](lua_State* L) -> int {
             const char* name = luaL_checkstring(L, 1);

             std::shared_ptr<Property> property(new Property(name));
             registerElement<Property>(L, property);

             return 1;
         }},
        {"__gc", destroyElement<Property>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<Property>(L, 1);
             auto e2 = getElement<Property>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"setOwner",
         [](lua_State* L) -> int {
             auto property = getElement<Property>(L);
             if (lua_isnil(L, -1)) {
                 if (property->owner()) {
                     int* ownerRef = (int*)property->owner();
                     delete ownerRef;
                 }
             } else {
                 int* ownerRef = new int;
                 *ownerRef = luaL_ref(L, LUA_REGISTRYINDEX);
                 property->setOwner(ownerRef);
             }
             return 0;
         }},
        {"owner",
         [](lua_State* L) -> int {
             auto property = getElement<Property>(L);
             if (property->owner()) {
                 int* ownerRef = (int*)property->owner();
                 lua_rawgeti(L, LUA_REGISTRYINDEX, *ownerRef);
             } else {
                 lua_pushnil(L);
             }
             return 1;
         }},
        {"setValueWillChangeFn",
         [](lua_State* L) -> int {
             // Stack: property, callback

             // store the reference to the Lua function in a variable to be used later
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);

             // Stack: property

             auto property = getElement<Property>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);

             property->setValueWillChangeFn([=](Property* sender) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<Property>(L, std::shared_ptr<Property>(sender, BypassDeleter<Property>()));
                 if (lua_pcall(L, 1, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });

             return 0;
         }},
        {"setValueDidChangeFn",
         [](lua_State* L) -> int {
             // Stack: property, callback

             // store the reference to the Lua function in a variable to be used later
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);

             // Stack: property

             auto property = getElement<Property>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);

             property->setValueDidChangeFn([=](Property* sender) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<Property>(L, std::shared_ptr<Property>(sender, BypassDeleter<Property>()));
                 if (lua_pcall(L, 1, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });

             return 0;
         }},
        {"value",
         [](lua_State* L) -> int {
             auto property = getElement<Property>(L);
             auto value = property->value();
             if (value.is<int>()) {
                 lua_pushinteger(L, value.as<int>());
             } else if (value.is<float>()) {
                 lua_pushnumber(L, value.as<float>());
             } else {
                 lua_pushnil(L);
             }
             return 1;
         }},
        {"setValue",
         [](lua_State* L) -> int {
             auto property = getElement<Property>(L);
             if (lua_isinteger(L, 2)) {
                 auto value = static_cast<int>(luaL_checkinteger(L, 2));
                 property->setValue(value);
             } else if (lua_isnumber(L, 2)) {
                 auto value = static_cast<float>(luaL_checknumber(L, 2));
                 property->setValue(value);
             }
             return 0;
         }},

        {NULL, NULL}  // Sentinel value
    };
    bindTable<Property>("Property", propertyTableDefinition);

    // Init additional modules
    for (auto module : scriptModules) module->init(this);

    char cwd[FILENAME_MAX];
    getcwd(cwd, sizeof(cwd));

    // Find the directory of the provided file
    size_t found;
    found = path.find_last_of("/\\");
    auto directory = path.substr(0, found);
    auto filename = path.substr(found + 1);

    _lastError.clear();

    chdir(directory.c_str());
    if (luaL_loadfile(_L, filename.c_str())) {
        _lastError = "Unable to load Lua script \"" + path + "\": " + lua_tostring(_L, -1);
        std::stringstream ss;
        ss << "Lua error: " << _lastError << std::endl;
        log(ss.str());
        chdir(cwd);
        if (_debugServer) {
            auto debugServer = reinterpret_cast<lrdb::server*>(_debugServer);
            debugServer->reset();  // unassign debug server (Required before lua_close )
        }
        lua_close(_L);
        _L = nullptr;
        MDStudio::Platform::sharedInstance()->cancelDelayedInvokes(this);
        return false;
    }
    _isExecutionCritical = true;
    if (lua_pcall(_L, 0, 0, 0) != 0) {
        _lastError = lua_tostring(_L, -1);
        std::stringstream ss;
        ss << "Lua error: " << _lastError << std::endl;
        log(ss.str());
        chdir(cwd);
        if (_debugServer) {
            auto debugServer = reinterpret_cast<lrdb::server*>(_debugServer);
            debugServer->reset();  // unassign debug server (Required before lua_close )
        }
        lua_close(_L);
        _L = nullptr;
        MDStudio::Platform::sharedInstance()->cancelDelayedInvokes(this);
        _isExecutionCritical = false;
        return false;
    }
    _isExecutionCritical = false;
    chdir(cwd);
    // Note: we keep the Lua states available at this point

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
std::string Script::findString(std::string name) {
    std::string ret = std::string("[") + name + std::string("]");

    if (!_L) return ret;

    lua_getglobal(_L, name.c_str());
    if (lua_isstring(_L, -1)) ret = std::string(lua_tostring(_L, -1));

    lua_pop(_L, 1);

    return ret;
}

// ---------------------------------------------------------------------------------------------------------------------
void Script::log(const std::string& s) {
    std::cout << s + "\n";
    if (_logFn) _logFn(this, s + "\n");
}

// ---------------------------------------------------------------------------------------------------------------------
void Script::error(const std::string& s) {
    if (_isExecutionCritical) {
        luaL_error(_L, s.c_str());
    } else {
        MDStudio::Platform::sharedInstance()->invoke([=] {
            if (_L) {
                log(s);
                if (_debugServer) {
                    auto debugServer = reinterpret_cast<lrdb::server*>(_debugServer);
                    debugServer->reset();  // unassign debug server (Required before lua_close )
                }
                lua_close(_L);
                _L = nullptr;
                MDStudio::Platform::sharedInstance()->cancelDelayedInvokes(this);
                log("Script terminated.");
                if (_didTerminateFn) _didTerminateFn(this);
            }
        });
    }
}
