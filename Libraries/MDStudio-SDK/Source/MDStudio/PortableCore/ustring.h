//
//  ustring.h
//  MDStudio
//
//  Created by Daniel Cliche on 2016-07-02.
//  Copyright © 2016-2018 Daniel Cliche. All rights reserved.
//

#ifndef USTRING_H
#define USTRING_H

#include <string>
#include <vector>

namespace MDStudio {

class UString {
    std::vector<unsigned short> _str16;

   public:
    UString();
    UString(std::string s);
    UString(const UString& s);

    std::string string();
    size_t length();
    std::string operator[](size_t index);
    bool isEmpty() { return _str16.size() == 0; }

    UString substr(size_t pos, size_t len) const;
    UString append(UString s) const;
    UString append(unsigned short c) const;
    void erase(size_t pos, size_t len);
    void toAscii();

    std::vector<unsigned short>* str16() { return &_str16; }
};

}  // namespace MDStudio

#endif  // USTRING_H
