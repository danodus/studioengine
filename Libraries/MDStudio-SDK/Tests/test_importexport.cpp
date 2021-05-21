//
//  test_importexport.h
//  MDStudioTest
//
//  Created by Daniel Cliche on 2021-02-22.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#include "test_importexport.h"

#include <midifile.h>

#include <iostream>

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
static bool compareSequences(Sequence* s1, Sequence* s2) {
    if (s1->data.tracks.size() != s2->data.tracks.size()) return false;

    size_t trackIndex = 0;
    for (auto& track : s1->data.tracks) {
        auto nbEvents1 = track.events.size();
        auto nbEvents2 = s2->data.tracks[trackIndex].events.size();
        if (nbEvents1 != nbEvents2) return false;
        size_t eventIndex = 0;
        for (auto& e1 : track.events) {
            auto& e2 = s2->data.tracks[trackIndex].events[eventIndex];
            if (e1.type != e2.type || e1.channel != e2.channel || e1.param1 != e2.param1 || e1.param2 != e2.param2 ||
                e1.tickCount != e2.tickCount)
                return false;

            if (e1.data.size() != e2.data.size()) return false;

            size_t dataIndex = 0;
            for (auto& d : e1.data) {
                if (d != e2.data[dataIndex]) return false;
                dataIndex++;
            }

            eventIndex++;
        }

        trackIndex++;
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool testImportExport() {
    auto sequence1 = readMIDIFile("Resources/danc_qn.mid");

    if (!sequence1) {
        std::cout << "Unable to read MIDI file\n";
        return false;
    }

    if (!writeMIDIFile("/tmp/danc_qn.mid", sequence1)) {
        std::cout << "Unable to write MIDI file\n";
        return false;
    }

    auto sequence2 = readMIDIFile("/tmp/danc_qn.mid");

    if (!sequence2) {
        std::cout << "Unable to read back MIDI file\n";
        return false;
    }

    if (!compareSequences(sequence1.get(), sequence2.get())) {
        std::cout << "Sequence mismatch detected\n";
        return false;
    }

    return true;
}