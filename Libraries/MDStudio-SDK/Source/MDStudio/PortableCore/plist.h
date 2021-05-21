//
//  plist.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-08-23.
//  Copyright (c) 2014 Daniel Cliche. All rights reserved.
//

//
//   PlistCpp Property List (plist) serialization and parsing library.
//
//   https://github.com/animetrics/PlistCpp
//
//   Copyright (c) 2011 Animetrics Inc. (marc@animetrics.com)
//
//   Permission is hereby granted, free of charge, to any person obtaining a copy
//   of this software and associated documentation files (the "Software"), to deal
//   in the Software without restriction, including without limitation the rights
//   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//   copies of the Software, and to permit persons to whom the Software is
//   furnished to do so, subject to the following conditions:
//
//   The above copyright notice and this permission notice shall be included in
//   all copies or substantial portions of the Software.
//
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//   THE SOFTWARE.

#ifndef PLIST_H
#define PLIST_H

#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "any.h"
#include "plistdate.h"

namespace Plist {
// Plist value types and their corresponding c++ types

typedef std::string string_type;
typedef int64_t integer_type;
typedef double real_type;
typedef std::map<std::string, Any> dictionary_type;
typedef std::vector<Any> array_type;
typedef Date date_type;
typedef std::vector<char> data_type;
typedef bool boolean_type;

// Public read methods.  Plist type (binary only) automatically detected.

void readPlist(const char* byteArrayTemp, int64_t size, Any& message);
void readPlist(std::istream& stream, Any& message);
template <typename T>
void readPlist(const char* byteArray, int64_t size, T& message);
template <typename T>
void readPlist(std::istream& stream, T& message);
template <typename T>
void readPlist(const char* filename, T& message);
#if defined(_MSC_VER)
template <typename T>
void readPlist(const wchar_t* filename, T& message);
#endif

// Public binary write methods.

void writePlistBinary(std::ostream& stream, const Any& message);
void writePlistBinary(std::vector<char>& plist, const Any& message);
void writePlistBinary(const char* filename, const Any& message);

class Error : public std::runtime_error {
   public:
    //#if __cplusplus >= 201103L
    //				using std::runtime_error::runtime_error;
    //#else
    inline Error(const std::string& what) : runtime_error(what) {}
    //#endif
};
};  // namespace Plist

#if defined(_MSC_VER)
template <typename T>
void Plist::readPlist(const wchar_t* filename, T& message) {
    std::ifstream stream(filename, std::ios::binary);
    if (!stream) throw Error("Can't open file.");
    readPlist(stream, message);
}
#endif

template <typename T>
void Plist::readPlist(const char* filename, T& message) {
    std::ifstream stream(filename, std::ios::binary);
    if (!stream) throw Error("Can't open file.");
    readPlist(stream, message);
}

template <typename T>
void Plist::readPlist(const char* byteArrayTemp, int64_t size, T& message) {
    Any tmp_message;
    readPlist(byteArrayTemp, size, tmp_message);
    message = tmp_message.as<T>();
}

template <typename T>
void Plist::readPlist(std::istream& stream, T& message) {
    Any tmp_message;
    readPlist(stream, tmp_message);
    message = tmp_message.as<T>();
}

#endif
