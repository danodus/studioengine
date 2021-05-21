//
//  ustring.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2016-07-02.
//  Copyright © 2016-2018 Daniel Cliche. All rights reserved.
//

#include "ustring.h"

#include <algorithm>

#include "utf8unchecked.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
UString::UString() {}

// ---------------------------------------------------------------------------------------------------------------------
UString::UString(std::string s) {
    // Convert to utf-16
    utf8::unchecked::utf8to16(s.begin(), s.end(), back_inserter(_str16));
}

// ---------------------------------------------------------------------------------------------------------------------
UString::UString(const UString& s) {
    for (auto c : s._str16) {
        _str16.push_back(c);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
std::string UString::string() {
    // Convert to utf-8
    std::string str8;
    utf8::unchecked::utf16to8(_str16.begin(), _str16.end(), back_inserter(str8));
    return str8;
}

// ---------------------------------------------------------------------------------------------------------------------
std::string UString::operator[](size_t index) {
    // Convert to utf-8
    std::string str8;
    utf8::unchecked::utf16to8(_str16.begin() + index, _str16.begin() + index + 1, back_inserter(str8));
    return str8;
}

// ---------------------------------------------------------------------------------------------------------------------
size_t UString::length() { return _str16.size(); }

// ---------------------------------------------------------------------------------------------------------------------
UString UString::substr(size_t pos, size_t len) const {
    UString ret;

    for (size_t i = 0; i < len; ++pos, ++i) {
        ret.str16()->push_back(_str16[pos]);
    }

    return ret;
}

// ---------------------------------------------------------------------------------------------------------------------
UString UString::append(UString s) const {
    UString ret;

    for (auto c : _str16) ret.str16()->push_back(c);

    for (auto c : *(s.str16())) {
        ret.str16()->push_back(c);
    }

    return ret;
}

// ---------------------------------------------------------------------------------------------------------------------
UString UString::append(unsigned short c) const {
    UString ret;

    for (auto c2 : _str16) ret.str16()->push_back(c2);

    ret.str16()->push_back(c);

    return ret;
}

// ---------------------------------------------------------------------------------------------------------------------
void UString::erase(size_t pos, size_t len) { _str16.erase(_str16.begin() + pos, _str16.begin() + pos + len); }

// ---------------------------------------------------------------------------------------------------------------------
void UString::toAscii() {
    _str16.erase(std::remove_if(_str16.begin(), _str16.end(), [](unsigned short c) { return c > 127; }), _str16.end());
}
