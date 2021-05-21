//
//  sequenceeditor.h
//  MelobaseCore
//
//  Created by Daniel Cliche on 2014-10-11.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#ifndef SEQUENCEEDITOR_H
#define SEQUENCEEDITOR_H

#include <studio.h>
#include <undomanager.h>

#include <functional>
#include <memory>
#include <vector>

#include "melobasecore_event.h"
#include "melobasecore_sequence.h"
#include "sequencesdb.h"

namespace MelobaseCore {

class SequenceEditor {
   public:
    typedef std::function<void(SequenceEditor* sender)> WillModifySequenceFnType;
    typedef std::function<void(SequenceEditor* sender)> DidModifySequenceFnType;

    typedef std::function<void(SequenceEditor* sender, std::shared_ptr<Track> track)> WillRemoveTrackFnType;

   private:
    WillModifySequenceFnType _willModifySequenceFn;
    DidModifySequenceFnType _didModifySequenceFn;
    WillRemoveTrackFnType _willRemoveTrackFn;

    MDStudio::UndoManager* _undoManager;
    std::shared_ptr<Sequence> _sequence;

    std::vector<MDStudio::Preset> _presets;

    bool _areEventsSortedAutomatically;

    void setDataFormat(UInt8 dataFormat);
    void internalMergeTracks(std::shared_ptr<Track> destinationTrack, std::vector<std::shared_ptr<Track>> tracks);

   public:
    SequenceEditor(MDStudio::UndoManager* undoManager, MDStudio::Studio* studio,
                   bool areEventsSortedAutomatically = true);

    MDStudio::UndoManager* undoManager() { return _undoManager; }

    void setSequence(std::shared_ptr<Sequence> sequence);
    std::shared_ptr<Sequence> sequence() { return _sequence; }

    bool canAddEvents(std::shared_ptr<Track> track, std::vector<std::shared_ptr<Event>> events);
    void addEvent(std::shared_ptr<Track> track, std::shared_ptr<Event> eventToAdd, bool isFirst, bool isLast);
    void removeEvent(std::shared_ptr<Track> track, std::shared_ptr<Event> eventToRemove, bool isFirst, bool isLast);
    void updateEvent(std::shared_ptr<Track> track, std::shared_ptr<Event> eventToUpdate, UInt32 tickCount,
                     UInt32 length, UInt8 channel, SInt32 param1, SInt32 param2, SInt32 param3,
                     std::vector<UInt8> data);

    bool canMoveEvents(std::shared_ptr<Track> track, std::vector<std::shared_ptr<Event>> events, SInt32 relativeTick,
                       SInt32 relativePitch, SInt32 relativeValue);
    SInt32 moveEvents(std::shared_ptr<Track> track, std::vector<std::shared_ptr<Event>> events, SInt32 relativeTick,
                      SInt32 relativePitch, SInt32 relativeValue, bool isFirst, bool isLast);

    bool canResizeEvents(std::shared_ptr<Track> track, std::vector<std::shared_ptr<Event>> events,
                         SInt32 relativeLength);
    void resizeEvents(std::shared_ptr<Track> track, std::vector<std::shared_ptr<Event>> events, SInt32 relativeLength);

    void quantizeEvents(std::shared_ptr<Track> track, std::vector<std::shared_ptr<Event>> events, UInt32 resTickCount);

    void setChannelOfEvents(std::shared_ptr<Track> track, std::vector<std::shared_ptr<Event>> events, UInt8 channel);
    void setVelocityOfEvents(std::shared_ptr<Track> track, std::vector<std::shared_ptr<Event>> events, SInt32 velocity);
    void setProgramOfEvents(std::shared_ptr<Track> track, std::vector<std::shared_ptr<Event>> events, SInt32 program);
    void setTimeSignatureOfEvents(std::shared_ptr<Track> track, std::vector<std::shared_ptr<Event>> events,
                                  SInt32 numerator, SInt32 denominator);
    void setSysexDataOfEvents(std::shared_ptr<Track> track, std::vector<std::shared_ptr<Event>> events,
                              std::vector<UInt8> data);
    void setMetaDataOfEvents(std::shared_ptr<Track> track, std::vector<std::shared_ptr<Event>> events,
                             std::vector<UInt8> data);
    void setPitchOfEvents(std::shared_ptr<Track> track, std::vector<std::shared_ptr<Event>> events, SInt32 pitch);

    void addTrack(std::shared_ptr<Track> trackToAdd, int trackIndex = -1);
    void removeTrack(std::shared_ptr<Track> trackToRemove);
    void setTrackName(std::shared_ptr<Track> track, std::string name);
    void setTrackChannel(std::shared_ptr<Track> track, UInt8 channel);

    void setTrackEvents(std::shared_ptr<Track> track, std::vector<std::shared_ptr<Event>> events);

    void convertToMultiTrack();
    void convertToSingleTrack();

    void mergeTracks(std::shared_ptr<Track> destinationTrack, std::vector<std::shared_ptr<Track>> tracks);

    std::vector<std::shared_ptr<Event>> getNoteEventsAtTick(UInt32 tick);

    void sortEvents(std::shared_ptr<Track> track);

    void setWillModifySequenceFn(WillModifySequenceFnType willModifySequenceFn) {
        _willModifySequenceFn = willModifySequenceFn;
    }
    void setDidModifySequenceFn(DidModifySequenceFnType didModifySequenceFn) {
        _didModifySequenceFn = didModifySequenceFn;
    }
    void setWillRemoveTrack(WillRemoveTrackFnType willRemoveTrackFn) { _willRemoveTrackFn = willRemoveTrackFn; }
};

}  // namespace MelobaseCore

#endif  // SEQUENCEEDITOR_H
