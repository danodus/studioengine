//
//  test_sequenceedition.cpp
//  MelobaseStationTests
//
//  Created by Daniel Cliche on 2015-04-12.
//  Copyright (c) 2015-2021 Daniel Cliche. All rights reserved.
//

#include "test_sequenceedition.h"

#include <melobasecore_sequence.h>
#include <platform.h>
#include <sequence.h>
#include <sequenceeditor.h>

#include <iostream>

// ---------------------------------------------------------------------------------------------------------------------
bool testMoveEvents() {
    std::shared_ptr<MelobaseCore::Sequence> sequence =
        std::shared_ptr<MelobaseCore::Sequence>(new MelobaseCore::Sequence());

    auto ev1 = std::make_shared<MelobaseCore::ChannelEvent>(CHANNEL_EVENT_TYPE_NOTE, 0, 1000, 500, 60, 0);
    auto ev2 = std::make_shared<MelobaseCore::ChannelEvent>(CHANNEL_EVENT_TYPE_NOTE, 0, 1500, 500, 60, 0);

    sequence->data.tracks[0]->clips[0]->events.push_back(ev1);
    sequence->data.tracks[0]->clips[0]->events.push_back(ev2);

    MDStudio::UndoManager undoManager;

    MelobaseCore::SequenceEditor sequenceEditor(&undoManager, nullptr);
    sequenceEditor.setSequence(sequence);

    std::vector<std::shared_ptr<MelobaseCore::Event>> eventsToMove;
    eventsToMove.push_back(ev1);
    eventsToMove.push_back(ev2);

    sequenceEditor.moveEvents(sequence->data.tracks[0], eventsToMove, -500, 10, 0, true, true);

    auto event = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(sequence->data.tracks[0]->clips[0]->events[0]);
    if (event->tickCount() != 500) {
        std::cout << "Move failed 1" << std::endl;
        return false;
    }

    if (event->param1() != 70) {
        std::cout << "Move failed 2" << std::endl;
        return false;
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool testQuantizeEvents() {
    std::shared_ptr<MelobaseCore::Sequence> sequence =
        std::shared_ptr<MelobaseCore::Sequence>(new MelobaseCore::Sequence());

    auto ev = std::make_shared<MelobaseCore::ChannelEvent>(CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE, 0, 0, 0, 4, 4);
    sequence->data.tracks[0]->clips[0]->events.push_back(ev);

    ev = std::make_shared<MelobaseCore::ChannelEvent>(CHANNEL_EVENT_TYPE_NOTE, 0, 1030, 500, 60, 0);
    sequence->data.tracks[0]->clips[0]->events.push_back(ev);

    MDStudio::UndoManager undoManager;

    MelobaseCore::SequenceEditor sequenceEditor(&undoManager, nullptr);
    sequenceEditor.setSequence(sequence);

    std::vector<std::shared_ptr<MelobaseCore::Event>> eventsToQuantize;
    eventsToQuantize.push_back(ev);

    sequenceEditor.quantizeEvents(sequence->data.tracks[0], eventsToQuantize, 100);

    auto event = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(sequence->data.tracks[0]->clips[0]->events[1]);
    if (event->tickCount() != 1000) {
        std::cout << "Quantize events failed" << std::endl;
        return false;
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool testTracks() {
    std::shared_ptr<MelobaseCore::Sequence> sequence =
        std::shared_ptr<MelobaseCore::Sequence>(new MelobaseCore::Sequence());

    auto ev1 = std::make_shared<MelobaseCore::ChannelEvent>(CHANNEL_EVENT_TYPE_NOTE, 0, 1000, 500, 60, 0);
    auto ev2 = std::make_shared<MelobaseCore::ChannelEvent>(CHANNEL_EVENT_TYPE_NOTE, 1, 2000, 500, 60, 0);

    sequence->data.tracks[0]->clips[0]->events.push_back(ev1);
    sequence->data.tracks[0]->clips[0]->events.push_back(ev2);

    MDStudio::UndoManager undoManager;

    MelobaseCore::SequenceEditor sequenceEditor(&undoManager, nullptr);
    sequenceEditor.setSequence(sequence);

    if (sequence->data.format != SEQUENCE_DATA_FORMAT_SINGLE_TRACK) {
        std::cout << "Invalid format" << std::endl;
        return false;
    }

    sequenceEditor.convertToMultiTrack();

    if ((sequence->data.format != SEQUENCE_DATA_FORMAT_MULTI_TRACK) || (sequence->data.tracks.size() != 3)) {
        std::cout << "Invalid format" << std::endl;
        return false;
    }

    auto event2 = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(sequence->data.tracks[2]->clips[0]->events[0]);
    if (event2->channel() != 1) {
        std::cout << "Invalid channel" << std::endl;
        return false;
    }

    auto tr1 = std::make_shared<MelobaseCore::Track>();
    sequenceEditor.addTrack(tr1, -1);

    if (sequence->data.tracks.size() != 4) {
        std::cout << "Invalid number of tracks" << std::endl;
        return false;
    }

    sequenceEditor.convertToSingleTrack();

    if ((sequence->data.format != SEQUENCE_DATA_FORMAT_SINGLE_TRACK) || (sequence->data.tracks.size() != 1)) {
        std::cout << "Invalid format" << std::endl;
        return false;
    }

    event2 = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(sequence->data.tracks[0]->clips[0]->events[1]);
    if (event2->channel() != 1) {
        std::cout << "Invalid channel" << std::endl;
        return false;
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool testStudioSequenceConversion() {
    std::shared_ptr<MelobaseCore::Sequence> sequence =
        std::shared_ptr<MelobaseCore::Sequence>(new MelobaseCore::Sequence());

    auto ev1 = std::make_shared<MelobaseCore::ChannelEvent>(CHANNEL_EVENT_TYPE_MIXER_BALANCE_CHANGE, 0, 0, 0, 0, 0);
    auto ev2 = std::make_shared<MelobaseCore::ChannelEvent>(CHANNEL_EVENT_TYPE_MIXER_LEVEL_CHANGE, 0, 0, 0, 0, 0);
    auto ev3 = std::make_shared<MelobaseCore::ChannelEvent>(CHANNEL_EVENT_TYPE_NOTE, 0, 1920, 480, 60, 0);
    auto ev4 = std::make_shared<MelobaseCore::ChannelEvent>(CHANNEL_EVENT_TYPE_NOTE, 0, 2400, 480, 60, 0);
    auto ev5 = std::make_shared<MelobaseCore::ChannelEvent>(CHANNEL_EVENT_TYPE_PITCH_BEND, 0, 2880, 0, 8000, 0);
    auto ev6 = std::make_shared<MelobaseCore::ChannelEvent>(CHANNEL_EVENT_TYPE_NOTE, 0, 2880, 480, 60, 0);

    sequence->data.tracks[0]->clips[0]->events.push_back(ev1);
    sequence->data.tracks[0]->clips[0]->events.push_back(ev2);
    sequence->data.tracks[0]->clips[0]->events.push_back(ev3);
    sequence->data.tracks[0]->clips[0]->events.push_back(ev4);
    sequence->data.tracks[0]->clips[0]->events.push_back(ev5);
    sequence->data.tracks[0]->clips[0]->events.push_back(ev6);

    auto studioSequence = MelobaseCore::getStudioSequence(sequence);

    // Check for consecutive note on or note off events
    bool isNoteOn = false;
    for (auto event : studioSequence->data.tracks[0].events) {
        if (event.type == EVENT_TYPE_NOTE_ON) {
            if (isNoteOn) return false;
            isNoteOn = true;
        } else if (event.type == EVENT_TYPE_NOTE_OFF) {
            if (!isNoteOn) return false;
            isNoteOn = false;
        }
    }

    return true;
}
