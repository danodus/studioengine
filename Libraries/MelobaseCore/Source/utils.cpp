//
//  utils.cpp
//  MelobaseCore
//
//  Created by Daniel Cliche on 2021-01-20.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#include "utils.h"

#include <uricodec.h>
#include <utf8.h>

// ---------------------------------------------------------------------------------------------------------------------
std::vector<std::string> MelobaseCore::stringComponents(const std::string& string, const char divider,
                                                        bool isDividerIncluded) {
    std::vector<std::string> components;
    size_t start = 0;
    for (size_t i = 0; i < string.length(); ++i) {
        if ((string[i] == divider) || (i == string.length() - 1)) {
            bool isLast = (i == string.length() - 1);
            std::string token = string.substr(start, i - start + (isLast ? 1 : 0));
            if (!token.empty()) components.push_back(token);
            if ((!isLast) && isDividerIncluded) {
                char s[2] = {divider, 0};
                components.push_back(s);
            }
            start = i + 1;
        }
    }
    return components;
}

// ---------------------------------------------------------------------------------------------------------------------
std::string MelobaseCore::trim(const std::string& str, const std::string& whitespace) {
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos) return "";  // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<std::string> MelobaseCore::pathComponents(const std::string path) {
    return stringComponents(path, '/', true);
}

// ---------------------------------------------------------------------------------------------------------------------
std::map<std::string, std::string> MelobaseCore::queryMap(const std::string query) {
    std::map<std::string, std::string> map;

    std::vector<std::string> components = stringComponents(query, '&');

    for (std::string component : components) {
        size_t p = component.find_first_of('=');
        if (p != std::string::npos) {
            std::string key = component.substr(0, p);
            std::string value = MDStudio::uriDecode(component.substr(p + 1));
            map[key] = value;
        }
    }

    return map;
}

// ---------------------------------------------------------------------------------------------------------------------
// Ref.: http://stackoverflow.com/questions/1091945/what-characters-do-i-need-to-escape-in-xml-documents
std::string MelobaseCore::encodeXMLString(std::string s) {
    // Convert to utf-16
    std::vector<unsigned short> str16;
    std::vector<unsigned short> str16Ret;

    utf8::utf8to16(s.begin(), s.end(), back_inserter(str16));

    for (auto c : str16) {
        std::string replacementStr;

        if (c == '"') {
            replacementStr = "&quot;";
        } else if (c == '\'') {
            replacementStr = "&apos;";
        } else if (c == '<') {
            replacementStr = "&lt;";
        } else if (c == '>') {
            replacementStr = "&gt;";
        } else if (c == '&') {
            replacementStr = "&amp;";
        } else if (c >= 32) {
            str16Ret.push_back(c);
        }

        if (replacementStr.length() > 0) {
            std::vector<unsigned short> str16ToInsert;
            utf8::utf8to16(replacementStr.begin(), replacementStr.end(), back_inserter(str16ToInsert));
            str16Ret.insert(str16Ret.end(), str16ToInsert.begin(), str16ToInsert.end());
        }
    }

    // Convert back to utf-8
    std::string str8;
    utf8::utf16to8(str16Ret.begin(), str16Ret.end(), back_inserter(str8));

    return str8;
}