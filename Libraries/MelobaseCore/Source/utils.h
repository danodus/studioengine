//
//  utils.h
//  MelobaseCore
//
//  Created by Daniel Cliche on 2021-01-20.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#ifndef MELOBASECORE_UTILS_H
#define MELOBASECORE_UTILS_H

#include <map>
#include <string>
#include <vector>

namespace MelobaseCore {

std::vector<std::string> stringComponents(const std::string& string, const char divider,
                                          bool isDividerIncluded = false);

std::string trim(const std::string& str, const std::string& whitespace = " \t");

std::vector<std::string> pathComponents(const std::string path);

std::map<std::string, std::string> queryMap(const std::string query);

std::string encodeXMLString(std::string s);

}  // namespace MelobaseCore

#endif  // MELOBASECORE_UTILS_H
