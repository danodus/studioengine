//
//  sequence.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2016-08-11.
//  Copyright (c) 2016-2020 Daniel Cliche. All rights reserved.
//

#include "sequence.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
static std::vector<Event> mergeEvents(const std::vector<Event>& newEvents, const std::vector<Event>& events) {
    UInt32 tickCount;

    //
    // First, we convert parameters to absolute timings
    //

    std::vector<Event> absNewEvents;
    std::vector<Event> absEvents;

    tickCount = 0;
    for (auto event : newEvents) {
        tickCount += event.tickCount;

        Event absNewEvent;
        absNewEvent.type = event.type;
        absNewEvent.channel = event.channel;
        absNewEvent.tickCount = tickCount;
        absNewEvent.param1 = event.param1;
        absNewEvent.param2 = event.param2;
        absNewEvents.push_back(absNewEvent);
    }

    tickCount = 0;
    for (auto event : events) {
        tickCount += event.tickCount;

        Event absEvent;
        absEvent.type = event.type;
        absEvent.channel = event.channel;
        absEvent.tickCount = tickCount;
        absEvent.param1 = event.param1;
        absEvent.param2 = event.param2;
        absEvents.push_back(absEvent);
    }

    //
    // We now perform the merge
    //

    std::vector<Event> retEvents;

    std::size_t eventIndex = 0, newEventIndex = 0;

    Event eventToAdd;

    tickCount = 0;
    while (1) {
        Event* event = (eventIndex < events.size()) ? &absEvents[eventIndex] : nullptr;
        Event* newEvent = (newEventIndex < newEvents.size()) ? &absNewEvents[newEventIndex] : nullptr;

        // If we have no more events, we stop
        if ((event == nullptr) && (newEvent == nullptr)) break;

        UInt32 eventDeltaTickCount = event ? (event->tickCount - tickCount) : 0xFFFFFFFF;
        UInt32 newEventDeltaTickCount = newEvent ? (newEvent->tickCount - tickCount) : 0xFFFFFFFF;

        // We pick the earliest event
        if (eventDeltaTickCount < newEventDeltaTickCount) {
            eventToAdd.type = event->type;
            eventToAdd.channel = event->channel;
            eventToAdd.tickCount = eventDeltaTickCount;
            eventToAdd.param1 = event->param1;
            eventToAdd.param2 = event->param2;
            retEvents.push_back(eventToAdd);
            eventIndex++;
            tickCount += eventDeltaTickCount;
        } else {
            if (newEvent) {
                eventToAdd.type = newEvent->type;
                eventToAdd.channel = newEvent->channel;
                eventToAdd.tickCount = newEventDeltaTickCount;
                eventToAdd.param1 = newEvent->param1;
                eventToAdd.param2 = newEvent->param2;
                retEvents.push_back(eventToAdd);
                newEventIndex++;
                tickCount += newEventDeltaTickCount;
            }
        }
    }

    return retEvents;
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::convertSequenceToSingleTrack(std::shared_ptr<MDStudio::Sequence> sequence) {
    // If the sequence has no tracks, we do nothing
    if (sequence->data.tracks.empty()) return;

    Track track;

    size_t nbTracks = sequence->data.tracks.size();
    for (size_t trackIndex = 0; trackIndex < nbTracks; ++trackIndex) {
        track.events = mergeEvents(sequence->data.tracks[trackIndex].events, track.events);
    }

    //
    // Keep only the last end of track event
    //

    bool isEndOfTrackEventFound = false;

    for (auto rit = track.events.rbegin(); rit != track.events.rend(); ++rit) {
        if (rit->type == EVENT_TYPE_META_END_OF_TRACK) {
            if (!isEndOfTrackEventFound) {
                isEndOfTrackEventFound = true;
            } else {
                track.events.erase(std::next(rit).base());
            }
        }
    }

    sequence->data.tracks.clear();
    sequence->data.tracks.push_back(track);
}