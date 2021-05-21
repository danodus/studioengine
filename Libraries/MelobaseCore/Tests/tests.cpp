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

#include "test_sequenceedition.h"
#include "test_sequencesdb.h"
#include "test_sync.h"

bool executeTest(const std::string& testName) {
    std::map<std::string, std::function<bool()>> tests = {

        {"MoveEvents", testMoveEvents},   {"QuantizeEvents", testQuantizeEvents},
        {"Tracks", testTracks},           {"StudioSequenceConversion", testStudioSequenceConversion},
        {"SequencesDB", testSequencesDB}, {"Sync", testSync}};

    if (tests.find(testName) == tests.end()) {
        std::cout << "Test not found\n";
        return false;
    }

    return tests[testName]();
}
