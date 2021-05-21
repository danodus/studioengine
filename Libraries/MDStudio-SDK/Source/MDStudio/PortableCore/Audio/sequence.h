//
//  sequence.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-15.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "event.h"

#define SEQUENCE_TRACK_MULTI_CHANNEL 255

#define SEQUENCE_DATA_FORMAT_SINGLE_TRACK 0
#define SEQUENCE_DATA_FORMAT_MULTI_TRACK 1

namespace MDStudio {

struct Track {
    std::string name;
    UInt8 channel;

    typedef std::function<void(Track* sender, Event* event)> DidAddEventFnType;
    DidAddEventFnType _didAddEventFn;

    std::vector<struct Event> events;

    void addEvent(Event event) {
        events.push_back(event);
        if (_didAddEventFn) _didAddEventFn(this, &event);
    }

    Track() {
        _didAddEventFn = nullptr;
        channel = SEQUENCE_TRACK_MULTI_CHANNEL;
    }
};

struct SequenceData {
    UInt8 format;
    Float64 tickPeriod;
    std::vector<struct Track> tracks;

    SequenceData() {
        format = SEQUENCE_DATA_FORMAT_SINGLE_TRACK;
        tickPeriod = 0;

        Track track;
        tracks.push_back(track);
    }
};

struct Sequence {
    SequenceData data;

    Sequence() {}
};

void convertSequenceToSingleTrack(std::shared_ptr<Sequence> sequence);
}  // namespace MDStudio

#endif  // SEQUENCE_H
