//
//  melobasecore_sequence.cpp
//  MelobaseCore
//
//  Created by Daniel Cliche on 2016-08-01.
//  Copyright © 2016-2021 Daniel Cliche. All rights reserved.
//

#include "melobasecore_sequence.h"

#include <studio.h>

#include <algorithm>
#include <iostream>
#include <stack>

using namespace MelobaseCore;

// ---------------------------------------------------------------------------------------------------------------------
static void setSequenceEventsToRelativeTicks(std::vector<std::shared_ptr<Event>>* events) {
    // Sort the events based on their absolute ticks
    std::sort(events->begin(), events->end(), [](std::shared_ptr<Event> a, std::shared_ptr<Event> b) {
        auto channelEventA = std::dynamic_pointer_cast<ChannelEvent>(a);
        auto channelEventB = std::dynamic_pointer_cast<ChannelEvent>(b);

        if (channelEventA->tickCount() == channelEventB->tickCount()) {
            int priA = getEventPriority(channelEventA);
            int priB = getEventPriority(channelEventB);

            return priA < priB;
        } else
            return channelEventA->tickCount() < channelEventB->tickCount();
    });

    // Create a new list using relative ticks
    UInt32 tickCount = 0;
    for (auto event : *events) {
        auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);
        UInt32 relTickCount = channelEvent->tickCount() - tickCount;
        channelEvent->setTickCount(relTickCount);
        tickCount += relTickCount;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MDStudio::Sequence> MelobaseCore::getStudioSequence(std::shared_ptr<Sequence> melobaseCoreSequence) {
    if (!melobaseCoreSequence) return nullptr;

    std::shared_ptr<MDStudio::Sequence> studioSequence = std::make_shared<MDStudio::Sequence>();

    studioSequence->data.tracks.clear();

    std::shared_ptr<ChannelEvent> lastNoteOffs[128];
    std::shared_ptr<ChannelEvent> lastNoteOns[128];

    for (auto track : melobaseCoreSequence->data.tracks) {
        std::vector<std::shared_ptr<Event>> trackEvents;
        for (auto event : track->clips[0]->events) {
            auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);
            trackEvents.push_back(std::make_shared<ChannelEvent>(
                channelEvent->type(), channelEvent->channel(), channelEvent->tickCount(), channelEvent->length(),
                channelEvent->param1(), channelEvent->param2(), channelEvent->param3(), channelEvent->data()));
        }

        // Add missing note off events
        std::vector<std::shared_ptr<Event>> eventsToAdd;
        std::vector<std::shared_ptr<Event>> eventsToRemove;
        for (auto event : trackEvents) {
            auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);
            if (channelEvent->type() == CHANNEL_EVENT_TYPE_NOTE) {
                auto lastNoteOff = lastNoteOffs[channelEvent->param1()];

                // If the last note off is beyond the note, an overlap is detected
                if (lastNoteOff && (lastNoteOff->channel() == channelEvent->channel()) &&
                    (lastNoteOff->tickCount() > channelEvent->tickCount()) &&
                    (lastNoteOff->param1() == channelEvent->param1())) {
                    // Re-adjust the note off in order to not overlap
                    lastNoteOff->setTickCount(channelEvent->tickCount());

                    auto lastNoteOn = lastNoteOns[channelEvent->param1()];

                    // Handle the case where the notes are overlapping at exactly the same tick
                    if (lastNoteOn->tickCount() == lastNoteOff->tickCount()) {
                        eventsToRemove.push_back(lastNoteOn);
                        eventsToRemove.push_back(lastNoteOff);
                    }
                }

                lastNoteOns[channelEvent->param1()] = channelEvent;

                lastNoteOff = std::make_shared<ChannelEvent>(CHANNEL_EVENT_TYPE_NOTE_OFF, channelEvent->channel(),
                                                             channelEvent->tickCount() + channelEvent->length(), 0,
                                                             channelEvent->param1(), 64);
                eventsToAdd.push_back(lastNoteOff);
                lastNoteOffs[channelEvent->param1()] = lastNoteOff;
            }
        }

        trackEvents.insert(trackEvents.end(), eventsToAdd.begin(), eventsToAdd.end());

        for (auto event : eventsToRemove)
            trackEvents.erase(std::remove(trackEvents.begin(), trackEvents.end(), event), trackEvents.end());

        // Convert to relative ticks
        setSequenceEventsToRelativeTicks(&trackEvents);

        MDStudio::Track studioTrack;
        for (auto event : trackEvents) {
            auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);
            studioTrack.events.push_back(MDStudio::makeEvent(channelEvent->type(), channelEvent->channel(),
                                                             channelEvent->tickCount(), channelEvent->param1(),
                                                             channelEvent->param2(), channelEvent->data()));
        }

        studioTrack.name = track->name;
        studioTrack.channel = track->channel;
        studioSequence->data.format = melobaseCoreSequence->data.format;
        studioSequence->data.tracks.push_back(studioTrack);
        studioSequence->data.tickPeriod = melobaseCoreSequence->data.tickPeriod;
    }

    return studioSequence;
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<Sequence> MelobaseCore::getMelobaseCoreSequence(std::shared_ptr<MDStudio::Sequence> studioSequence) {
    if (!studioSequence) return nullptr;

    std::shared_ptr<Sequence> melobaseCoreSequence = std::make_shared<Sequence>();

    melobaseCoreSequence->data.tracks.clear();

    for (auto studioTrack : studioSequence->data.tracks) {
        std::stack<UInt32> noteOnTickCounts[STUDIO_MAX_CHANNELS][128];
        std::stack<std::shared_ptr<ChannelEvent>> noteOnChannelEvents[STUDIO_MAX_CHANNELS][128];

        UInt32 currentTickCount = 0;

        std::shared_ptr<MelobaseCore::Track> melobaseCoreTrack = std::make_shared<MelobaseCore::Track>();
        for (auto event : studioTrack.events) {
            currentTickCount += event.tickCount;

            switch (event.type) {
                case EVENT_TYPE_NOTE_ON: {
                    noteOnTickCounts[event.channel][event.param1].push(currentTickCount);

                    auto channelEvent = std::make_shared<MelobaseCore::ChannelEvent>(
                        CHANNEL_EVENT_TYPE_NOTE, event.channel, currentTickCount, 0, event.param1, event.param2);
                    noteOnChannelEvents[event.channel][event.param1].push(channelEvent);
                    melobaseCoreTrack->clips[0]->events.push_back(channelEvent);
                } break;
                case EVENT_TYPE_NOTE_OFF: {
                    if (!noteOnChannelEvents[event.channel][event.param1].empty()) {
                        // Adjust the length and the note off velocity of the note event
                        auto noteOnChannelEvent = noteOnChannelEvents[event.channel][event.param1].top();
                        noteOnChannelEvents[event.channel][event.param1].pop();
                        auto noteOnTickCount = noteOnTickCounts[event.channel][event.param1].top();
                        noteOnTickCounts[event.channel][event.param1].pop();
                        noteOnChannelEvent->setLength(currentTickCount - noteOnTickCount);
                        noteOnChannelEvent->setParam3(event.param2);
                    }
                } break;

                default: {
                    auto channelEvent = std::make_shared<MelobaseCore::ChannelEvent>(
                        event.type, event.channel, currentTickCount, 0, event.param1, event.param2, 0, event.data);
                    melobaseCoreTrack->clips[0]->events.push_back(channelEvent);
                }
            }
        }

        melobaseCoreTrack->name = studioTrack.name;
        melobaseCoreTrack->channel = studioTrack.channel;

        melobaseCoreSequence->data.tracks.push_back(melobaseCoreTrack);

    }  // for each track

    melobaseCoreSequence->data.format = studioSequence->data.format;
    melobaseCoreSequence->data.tickPeriod = studioSequence->data.tickPeriod;

    return melobaseCoreSequence;
}
