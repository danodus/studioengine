//
//  tests.cpp
//  MelobaseStationTests
//
//  Created by Daniel Cliche on 2015-04-12.
//  Copyright (c) 2015-2021 Daniel Cliche. All rights reserved.
//

#include "tests.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>

#include "test_importexport.h"
#include "test_pasteboard.h"
#include "test_plist.h"
#include "test_undomanager.h"

bool executeTest(const std::string& testName) {
    std::map<std::string, std::function<bool()>> tests = {{"Plist", testPlist},
                                                          {"UndoManager", testUndoManager},
                                                          {"PasteBoard", testPasteboard},
                                                          {"ImportExport", testImportExport}};

    if (tests.find(testName) == tests.end()) {
        std::cout << "Test not found\n";
        return false;
    }

    return tests[testName]();
}
