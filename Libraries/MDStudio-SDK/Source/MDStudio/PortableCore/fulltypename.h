//
//  fulltypename.h
//  MDStudio
//
//  Created by Daniel Cliche on 2020-04-18.
//  Copyright © 2020 Daniel Cliche. All rights reserved.
//

#ifndef FULLTYPENAME_H
#define FULLTYPENAME_H

namespace MDStudio {

template <typename...>
struct FullTypeName;

template <typename T>
struct FullTypeName<T> {
    static std::string get(char sep) { return std::string(typeid(T).name()); }
};

template <typename T, typename... R>
struct FullTypeName<T, R...> {
    static std::string get(char sep) { return std::string(typeid(T).name()) + sep + FullTypeName<R...>::get(sep); }
};

template <>
struct FullTypeName<> {
    static std::string get(char sep) { return ""; }
};

template <typename... C>
std::string fullTypeName(char sep) {
    return FullTypeName<C...>::get(sep);
}
}  // namespace MDStudio

#endif  // FULLTYPENAME_H
