//
//  script.h
//  MDStudio
//
//  Created by Daniel Cliche on 2018-04-07.
//  Copyright © 2018-2020 Daniel Cliche. All rights reserved.
//

#ifndef SCRIPT_H
#define SCRIPT_H

#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include "fulltypename.h"

void dumpStack(lua_State* L);

namespace MDStudio {

template <typename... C>
void bindTable(lua_State* L, const char* name, const struct luaL_Reg* tableDefinition) {
    lua_newtable(L);
    luaL_newmetatable(L, fullTypeName<C...>('.').c_str());
    luaL_setfuncs(L, tableDefinition, 0);
    lua_pushliteral(L, "__index");
    lua_pushvalue(L, -2);
    lua_rawset(L, -3);
    lua_setglobal(L, name);
}

template <typename T>
inline std::shared_ptr<T> getElement(lua_State* L, int index = 1) {
    if (lua_isnil(L, index)) {
        return nullptr;
    } else if (lua_getmetatable(L, index)) {
        lua_pushstring(L, "__name");
        lua_rawget(L, -2);
        auto s = std::string(lua_tostring(L, -1));
        lua_pop(L, 2);

        size_t pos = 0;
        std::string token = s;

        while ((pos = s.find('.')) != std::string::npos) {
            token = s.substr(0, pos);
            if (token == typeid(T).name()) return *((std::shared_ptr<T>*)lua_touserdata(L, index));

            s.erase(0, pos + 1);
        }

        if (s == typeid(T).name()) return *((std::shared_ptr<T>*)lua_touserdata(L, index));
    }

    luaL_error(L, "Invalid element type");
    return nullptr;
}

template <typename T, typename... R>
void registerElement(lua_State* L, std::shared_ptr<T> element) {
    if (!element) {
        lua_pushnil(L);
    } else {
        auto p = (std::shared_ptr<T>*)lua_newuserdata(L, sizeof(element));
        std::memcpy(p, &element, sizeof(element));
        new (p) std::shared_ptr<T>(element);
        // Now just set the metatable on this new object
        std::string typeName = fullTypeName<T, R...>('.');
        luaL_getmetatable(L, typeName.c_str());
        lua_setmetatable(L, -2);
    }
}

template <typename T, typename... R>
static int destroyElement(lua_State* L) {
    std::string typeName = fullTypeName<T, R...>('.');
    void* p = luaL_checkudata(L, 1, typeName.c_str());

    if (p) {
        auto e = static_cast<std::shared_ptr<T>*>(p);
        e->reset();
    }

    return 0;
}

template <typename T>
struct BypassDeleter {
    void operator()(T* p) const {
        // Do nothing
    }
};

std::vector<lua_Number> getNumbers(lua_State* L, int index = 2);

class Script;
class ScriptModule {
   public:
    virtual void init(Script* script) = 0;
    virtual ~ScriptModule() = default;
};

class Script {
   public:
    typedef std::function<void(Script* sender, const std::string& message)> LogFnType;
    typedef std::function<void(Script* sender)> DidTerminateFnType;

   private:
    void* _debugServer;
    lua_State* _L;

    std::string _lastError;

    LogFnType _logFn;
    DidTerminateFnType _didTerminateFn;

    bool _isExecutionCritical;

    std::map<std::string, std::vector<std::vector<struct luaL_Reg>>> _tables;

    template <typename... C>
    void bindTable(const char* name, const struct luaL_Reg* tableDefinition);

   public:
    Script();
    ~Script();

    void setGlobal(const char* name, void* p);

    template <typename... C>
    void bindTable(const char* name, const std::vector<std::vector<struct luaL_Reg>>& tableDefinitions);

    void bindFunction(const char* name, lua_CFunction fn);

    bool execute(std::string path, std::vector<ScriptModule*> scriptModules, bool isDebugging,
                 int debugServerPort = 21110);

    template <typename T>
    std::shared_ptr<T> findElement(std::string name);
    std::string findString(std::string name);

    std::string lastError() { return _lastError; }

    bool isRunning() { return _L != nullptr; }

    void callCallback(lua_State* L, int callbackRef);
    void log(const std::string& s);
    void error(const std::string& s);

    std::map<std::string, std::vector<std::vector<struct luaL_Reg>>>& tables() { return _tables; };

    void setLogFn(LogFnType logFn) { _logFn = logFn; }
    void setDidTerminateFn(DidTerminateFnType didTerminateFn) { _didTerminateFn = didTerminateFn; }
};

// ---------------------------------------------------------------------------------------------------------------------
template <typename... C>
void Script::bindTable(const char* name, const struct luaL_Reg* tableDefinition) {
    MDStudio::bindTable<C...>(_L, name, tableDefinition);
}

// ---------------------------------------------------------------------------------------------------------------------
template <typename... C>
void Script::bindTable(const char* name, const std::vector<std::vector<struct luaL_Reg>>& tableDefinitions) {
    _tables[name] = tableDefinitions;

    std::vector<struct luaL_Reg> combinedTable;
    for (auto& table : tableDefinitions) {
        combinedTable.insert(combinedTable.end(), table.begin(), table.end());
    }
    combinedTable.push_back({NULL, NULL});

    MDStudio::bindTable<C...>(_L, name, combinedTable.data());
}

// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::shared_ptr<T> Script::findElement(std::string name) {
    std::shared_ptr<T> element = nullptr;

    lua_getglobal(_L, name.c_str());

    if (lua_isuserdata(_L, -1)) element = *((std::shared_ptr<T>*)lua_touserdata(_L, -1));

    lua_pop(_L, 1);

    return element;
}

}  // namespace MDStudio

#endif  // SCRIPT_H
