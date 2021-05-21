//
//  melobasecore_sequence.h
//  MelobaseCore
//
//  Created by Daniel Cliche on 2016-08-01.
//  Copyright © 2016-2021 Daniel Cliche. All rights reserved.
//

#ifndef MELOBASECORE_SEQUENCE_H
#define MELOBASECORE_SEQUENCE_H

#include <sequence.h>
#include <studio.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "melobasecore_event.h"

namespace MelobaseCore {

struct Clip {
    std::vector<std::shared_ptr<Event>> events;

    void addEvent(std::shared_ptr<Event> event) { events.push_back(event); }

    Clip() {}

    std::shared_ptr<Clip> copy() {
        auto newClip = std::make_shared<MelobaseCore::Clip>();
        for (auto e : events) {
            auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(e);
            auto newChannelEvent = std::make_shared<MelobaseCore::ChannelEvent>(
                channelEvent->type(), channelEvent->channel(), channelEvent->tickCount(), channelEvent->length(),
                channelEvent->param1(), channelEvent->param2(), channelEvent->param3(), channelEvent->data());
            newClip->events.push_back(newChannelEvent);
        }
        return newClip;
    }
};

struct Track {
    std::string name;
    std::vector<std::shared_ptr<Clip>> clips;
    UInt8 channel;

    Track() {
        std::shared_ptr<Clip> clip = std::make_shared<Clip>();
        clips.push_back(clip);
        channel = SEQUENCE_TRACK_MULTI_CHANNEL;
    }

    std::shared_ptr<Track> copy() {
        // Create a copy of the track
        auto newTrack = std::make_shared<MelobaseCore::Track>();
        newTrack->name = name;
        newTrack->channel = channel;
        newTrack->clips.clear();

        for (auto c : clips) {
            newTrack->clips.push_back(c->copy());
        }
        return newTrack;
    }
};

struct SequenceData {
    UInt64 id;
    UInt8 format;
    UInt32 currentPosition;
    Float64 tickPeriod;
    std::vector<std::shared_ptr<Track>> tracks;

    SequenceData() {
        id = 0;
        format = SEQUENCE_DATA_FORMAT_SINGLE_TRACK;
        currentPosition = 0;
        tickPeriod = 0;

        std::shared_ptr<Track> track = std::make_shared<Track>();
        tracks.push_back(track);
    }
};

struct SequenceAnnotation {
    UInt32 tickCount;

    SequenceAnnotation() { tickCount = 0; }
};

struct SequencesFolder;
struct Sequence {
    UInt64 id;
    Float64 date;
    SInt32 playCount;
    Float32 rating;
    Float64 version;
    Float64 dataVersion;
    std::string name;
    std::string desc;
    SequenceData data;
    std::vector<std::shared_ptr<SequenceAnnotation>> annotations;
    std::shared_ptr<SequencesFolder> folder;

    Sequence() {
        id = 0;
        date = 0;
        playCount = 0;
        rating = 0;
        version = 0;
        dataVersion = 0;
        folder = nullptr;
    }
};

struct SequencesFolder {
    UInt64 id;
    std::string name;
    Float64 date;
    Float32 rating;
    Float64 version;
    UInt64 parentID;

    std::shared_ptr<SequencesFolder> parent;
    std::vector<std::shared_ptr<SequencesFolder>> subfolders;

    SequencesFolder() {
        id = 0;
        date = 0;
        rating = 0;
        version = 0;
        parentID = 0;
    }
};

std::shared_ptr<MDStudio::Sequence> getStudioSequence(std::shared_ptr<Sequence> melobaseCoreSequence);
std::shared_ptr<Sequence> getMelobaseCoreSequence(std::shared_ptr<MDStudio::Sequence> studioSequence);
inline int getEventPriority(std::shared_ptr<ChannelEvent> channelEvent);
}  // namespace MelobaseCore

// ---------------------------------------------------------------------------------------------------------------------
inline int MelobaseCore::getEventPriority(std::shared_ptr<ChannelEvent> channelEvent) {
    if (channelEvent->type() == CHANNEL_EVENT_TYPE_NOTE_OFF) return -10;

    if (channelEvent->type() == CHANNEL_EVENT_TYPE_CONTROL_CHANGE) {
        // Make sure that the parameter number is first followed by the data entry
        if (channelEvent->param1() == STUDIO_CONTROL_REG_PARAM_NUM_MSB ||
            channelEvent->param1() == STUDIO_CONTROL_NON_REG_PARAM_NUM_MSB) {
            return -7;
        } else if (channelEvent->param1() == STUDIO_CONTROL_REG_PARAM_NUM_LSB ||
                   channelEvent->param1() == STUDIO_CONTROL_NON_REG_PARAM_NUM_LSB) {
            return -6;
        } else if (channelEvent->param1() == STUDIO_CONTROL_DATA_ENTRY_MSB) {
            return -5;
        } else if (channelEvent->param1() == STUDIO_CONTROL_DATA_ENTRY_LSB) {
            return -4;
        }
        return -3;
    }

    if (channelEvent->type() == CHANNEL_EVENT_TYPE_SUSTAIN) {
        if (channelEvent->param1() < 64) {
            return -2;
        } else {
            return -1;
        }
    }

    if (channelEvent->type() == CHANNEL_EVENT_TYPE_PROGRAM_CHANGE ||
        channelEvent->type() == CHANNEL_EVENT_TYPE_MIXER_LEVEL_CHANGE ||
        channelEvent->type() == CHANNEL_EVENT_TYPE_MIXER_BALANCE_CHANGE ||
        channelEvent->type() == CHANNEL_EVENT_TYPE_PITCH_BEND ||
        channelEvent->type() == CHANNEL_EVENT_TYPE_MODULATION) {
        return -1;
    }

    if (channelEvent->type() == CHANNEL_EVENT_TYPE_META_END_OF_TRACK) return 1;

    return 0;
}

#endif  // MELOBASECORE_SEQUENCE_H
