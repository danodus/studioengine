//
//  test_plist.cpp
//  MelobaseStationTests
//
//  Created by Daniel Cliche on 2015-07-06.
//  Copyright (c) 2015-2021 Daniel Cliche. All rights reserved.
//

#include "test_plist.h"

#include <platform.h>
#include <plist.h>

#include <iostream>

using namespace Plist;

// ---------------------------------------------------------------------------------------------------------------------
bool testPlist() {
    auto obj = array_type();
    obj.push_back(integer_type(0));
    obj.push_back(integer_type(1));

    std::vector<char> plist;
    writePlistBinary(plist, obj);

    Any obj2;
    readPlist(&plist[0], plist.size(), obj2);

    array_type arr = obj2.as<array_type>();

    if (arr[0].as<integer_type>() != 0) return false;

    if (arr[1].as<integer_type>() != 1) return false;

    return true;
}
