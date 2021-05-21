//
//  melobasecorescriptmodule.cpp
//  MelobaseCore
//
//  Created by Daniel Cliche on 2019-09-16.
//  Copyright (c) 2019-2021 Daniel Cliche. All rights reserved.
//

#include "melobasecorescriptmodule.h"

#include <platform.h>
#include <sequencer.h>

#include <iostream>

#include "melobasecore_sequence.h"
#include "sequenceeditor.h"
#include "sequencesdb.h"
#include "server.h"
#include "studiocontroller.h"

using namespace MelobaseCore;

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCoreScriptModule::init(MDStudio::Script* script) {
    script->setGlobal("melobaseScriptModule", this);

    // SequencesFolder
    std::vector<struct luaL_Reg> sequencesFolderTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             auto sequencesFolder = std::make_shared<SequencesFolder>();
             sequencesFolder->name = "New Folder";
             MDStudio::registerElement<SequencesFolder>(L, sequencesFolder);
             return 1;
         }},
        {"__gc", MDStudio::destroyElement<SequencesFolder>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = MDStudio::getElement<SequencesFolder>(L, 1);
             auto e2 = MDStudio::getElement<SequencesFolder>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"setParentID", [](lua_State* L) -> int {
             auto sequencesFolder = MDStudio::getElement<SequencesFolder>(L);
             sequencesFolder->parentID = luaL_checkinteger(L, 2);
             return 0;
         }}};
    script->bindTable<MelobaseCore::SequencesFolder>("SequencesFolder", {sequencesFolderTableDefinition});

    // SequenceAnnotation
    std::vector<struct luaL_Reg> sequenceAnnotationTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             auto sequenceAnnotation = std::make_shared<SequenceAnnotation>();
             MDStudio::registerElement<SequenceAnnotation>(L, sequenceAnnotation);
             return 1;
         }},
        {"__gc", MDStudio::destroyElement<SequenceAnnotation>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = MDStudio::getElement<SequenceAnnotation>(L, 1);
             auto e2 = MDStudio::getElement<SequenceAnnotation>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"setTickCount", [](lua_State* L) -> int {
             auto sequenceAnnotation = MDStudio::getElement<SequenceAnnotation>(L);
             sequenceAnnotation->tickCount = static_cast<UInt32>(luaL_checkinteger(L, 2));
             return 0;
         }}};
    script->bindTable<MelobaseCore::SequenceAnnotation>("SequenceAnnotation", {sequenceAnnotationTableDefinition});

    // Sequence
    std::vector<struct luaL_Reg> sequenceTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             auto sequence = std::make_shared<Sequence>();
             MDStudio::registerElement<Sequence>(L, sequence);
             sequence->date = sequence->version = sequence->dataVersion = MDStudio::getTimestamp();
             sequence->data.tickPeriod = 0.001;
             sequence->data.currentPosition = 0;
             sequence->playCount = 0;
             sequence->rating = 0;

             return 1;
         }},
        {"__gc", MDStudio::destroyElement<Sequence>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = MDStudio::getElement<Sequence>(L, 1);
             auto e2 = MDStudio::getElement<Sequence>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"setName",
         [](lua_State* L) -> int {
             auto sequence = MDStudio::getElement<Sequence>(L);
             auto name = luaL_checkstring(L, 2);
             sequence->name = name;
             return 0;
         }},
        {"setFolder",
         [](lua_State* L) -> int {
             auto sequence = MDStudio::getElement<Sequence>(L);
             auto folder = MDStudio::getElement<SequencesFolder>(L, 2);
             sequence->folder = folder;
             return 0;
         }},
        {"setAnnotations",
         [](lua_State* L) -> int {
             auto sequence = MDStudio::getElement<Sequence>(L);

             sequence->annotations.clear();

             luaL_checktype(L, 2, LUA_TTABLE);
             lua_pushnil(L);
             while (lua_next(L, 2)) {
                 auto annotation = MDStudio::getElement<SequenceAnnotation>(L, -1);
                 sequence->annotations.emplace_back(annotation);
                 lua_pop(L, 1);
             }

             return 0;
         }},
        {"tracks", [](lua_State* L) -> int {
             auto sequence = MDStudio::getElement<Sequence>(L);
             auto tracks = sequence->data.tracks;

             lua_newtable(L);

             lua_Integer i = 1;
             for (auto track : tracks) {
                 lua_pushinteger(L, i);
                 MDStudio::registerElement<Track>(L, track);
                 lua_settable(L, -3);
                 ++i;
             }

             return 1;
         }}};
    script->bindTable<Sequence>("MelobaseSequence", {sequenceTableDefinition});

    // ChannelEvent
    std::vector<struct luaL_Reg> channelEventTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             auto channelEvent = std::make_shared<ChannelEvent>();
             MDStudio::registerElement<ChannelEvent>(L, channelEvent);
             return 1;
         }},
        {"__gc", MDStudio::destroyElement<ChannelEvent>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = MDStudio::getElement<ChannelEvent>(L, 1);
             auto e2 = MDStudio::getElement<ChannelEvent>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"setType",
         [](lua_State* L) -> int {
             auto channelEvent = MDStudio::getElement<ChannelEvent>(L);
             channelEvent->setType(luaL_checkinteger(L, 2));
             return 0;
         }},
        {"type",
         [](lua_State* L) -> int {
             auto channelEvent = MDStudio::getElement<ChannelEvent>(L);
             lua_pushinteger(L, channelEvent->type());
             return 1;
         }},
        {"setChannel",
         [](lua_State* L) -> int {
             auto channelEvent = MDStudio::getElement<ChannelEvent>(L);
             channelEvent->setChannel(luaL_checkinteger(L, 2));
             return 0;
         }},
        {"channel",
         [](lua_State* L) -> int {
             auto channelEvent = MDStudio::getElement<ChannelEvent>(L);
             lua_pushinteger(L, channelEvent->channel());
             return 1;
         }},
        {"setTickCount",
         [](lua_State* L) -> int {
             auto channelEvent = MDStudio::getElement<ChannelEvent>(L);
             channelEvent->setTickCount(static_cast<UInt32>(luaL_checkinteger(L, 2)));
             return 0;
         }},
        {"tickCount",
         [](lua_State* L) -> int {
             auto channelEvent = MDStudio::getElement<ChannelEvent>(L);
             lua_pushinteger(L, channelEvent->tickCount());
             return 1;
         }},
        {"setLength",
         [](lua_State* L) -> int {
             auto channelEvent = MDStudio::getElement<ChannelEvent>(L);
             channelEvent->setLength(static_cast<UInt32>(luaL_checkinteger(L, 2)));
             return 0;
         }},
        {"length",
         [](lua_State* L) -> int {
             auto channelEvent = MDStudio::getElement<ChannelEvent>(L);
             lua_pushinteger(L, channelEvent->length());
             return 1;
         }},
        {"setParam1",
         [](lua_State* L) -> int {
             auto channelEvent = MDStudio::getElement<ChannelEvent>(L);
             channelEvent->setParam1(static_cast<SInt32>(luaL_checkinteger(L, 2)));
             return 0;
         }},
        {"param1",
         [](lua_State* L) -> int {
             auto channelEvent = MDStudio::getElement<ChannelEvent>(L);
             lua_pushinteger(L, channelEvent->param1());
             return 1;
         }},
        {"setParam2",
         [](lua_State* L) -> int {
             auto channelEvent = MDStudio::getElement<ChannelEvent>(L);
             channelEvent->setParam2(static_cast<SInt32>(luaL_checkinteger(L, 2)));
             return 0;
         }},
        {"param2",
         [](lua_State* L) -> int {
             auto channelEvent = MDStudio::getElement<ChannelEvent>(L);
             lua_pushinteger(L, channelEvent->param2());
             return 1;
         }},
        {"setParam3",
         [](lua_State* L) -> int {
             auto channelEvent = MDStudio::getElement<ChannelEvent>(L);
             channelEvent->setParam3(static_cast<SInt32>(luaL_checkinteger(L, 2)));
             return 0;
         }},
        {"param3", [](lua_State* L) -> int {
             auto channelEvent = MDStudio::getElement<ChannelEvent>(L);
             lua_pushinteger(L, channelEvent->param3());
             return 1;
         }}};

    script->bindTable<ChannelEvent>("ChannelEvent", {channelEventTableDefinition});

    // Clip
    std::vector<struct luaL_Reg> clipTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             auto clip = std::make_shared<Clip>();
             MDStudio::registerElement<Clip>(L, clip);
             return 1;
         }},
        {"__gc", MDStudio::destroyElement<Clip>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = MDStudio::getElement<Clip>(L, 1);
             auto e2 = MDStudio::getElement<Clip>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"addEvent",
         [](lua_State* L) -> int {
             auto clip = MDStudio::getElement<Clip>(L);
             auto channelEvent = MDStudio::getElement<ChannelEvent>(L, 2);
             clip->addEvent(channelEvent);
             return 0;
         }},
        {"events", [](lua_State* L) -> int {
             auto clip = MDStudio::getElement<Clip>(L);
             auto events = clip->events;

             lua_newtable(L);

             lua_Integer i = 1;
             for (auto event : events) {
                 auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);
                 lua_pushinteger(L, i);
                 MDStudio::registerElement<ChannelEvent>(L, channelEvent);
                 lua_settable(L, -3);
                 ++i;
             }

             return 1;
         }}};
    script->bindTable<Clip>("Clip", {clipTableDefinition});

    // Track
    std::vector<struct luaL_Reg> trackTableDefinition = {{"new",
                                                          [](lua_State* L) -> int {
                                                              auto track = std::make_shared<Track>();
                                                              MDStudio::registerElement<Track>(L, track);
                                                              return 1;
                                                          }},
                                                         {"__gc", MDStudio::destroyElement<Track>},
                                                         {"__eq",
                                                          [](lua_State* L) -> int {
                                                              auto e1 = MDStudio::getElement<Track>(L, 1);
                                                              auto e2 = MDStudio::getElement<Track>(L, 2);
                                                              lua_pushboolean(L, e1 == e2);
                                                              return 1;
                                                          }},
                                                         {"channel",
                                                          [](lua_State* L) -> int {
                                                              auto track = MDStudio::getElement<Track>(L);
                                                              lua_pushinteger(L, track->channel);
                                                              return 1;
                                                          }},
                                                         {"clips", [](lua_State* L) -> int {
                                                              auto track = MDStudio::getElement<Track>(L);
                                                              auto clips = track->clips;

                                                              lua_newtable(L);

                                                              lua_Integer i = 1;
                                                              for (auto clip : clips) {
                                                                  lua_pushinteger(L, i);
                                                                  MDStudio::registerElement<Clip>(L, clip);
                                                                  lua_settable(L, -3);
                                                                  ++i;
                                                              }

                                                              return 1;
                                                          }}};
    script->bindTable<Track>("Track", {trackTableDefinition});

    // SequencesDB
    std::vector<struct luaL_Reg> sequencesDBTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             auto path = luaL_checkstring(L, 1);
             auto undoManager = MDStudio::getElement<MDStudio::UndoManager>(L, 2);
             auto sequencesDB = std::make_shared<SequencesDB>(path, undoManager.get());
             MDStudio::registerElement<SequencesDB>(L, sequencesDB);
             return 1;
         }},
        {"__gc", MDStudio::destroyElement<SequencesDB>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = MDStudio::getElement<SequencesDB>(L, 1);
             auto e2 = MDStudio::getElement<SequencesDB>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"open",
         [](lua_State* L) -> int {
             auto sequencesDB = MDStudio::getElement<SequencesDB>(L);
             if (!lua_isboolean(L, 2)) luaL_error(L, "Boolean expected");
             auto isInitiallyEmpty = lua_toboolean(L, 2) != 0;
             sequencesDB->open(isInitiallyEmpty);
             return 0;
         }},
        {"getFolderWithID",
         [](lua_State* L) -> int {
             auto sequencesDB = MDStudio::getElement<SequencesDB>(L);
             auto id = luaL_checkinteger(L, 2);
             auto folder = sequencesDB->getFolderWithID(id);
             MDStudio::registerElement<SequencesFolder>(L, folder);
             return 1;
         }},

        {"addFolder",
         [](lua_State* L) -> int {
             auto sequencesDB = MDStudio::getElement<SequencesDB>(L);
             auto folder = MDStudio::getElement<SequencesFolder>(L, 2);
             sequencesDB->addFolder(folder);
             return 0;
         }},

        {"addSequence", [](lua_State* L) -> int {
             auto sequencesDB = MDStudio::getElement<SequencesDB>(L);
             auto sequence = MDStudio::getElement<Sequence>(L, 2);
             sequencesDB->addSequence(sequence);
             return 0;
         }}};
    script->bindTable<SequencesDB>("SequencesDB", {sequencesDBTableDefinition});

    // SequenceEditor
    std::vector<struct luaL_Reg> sequenceEditorTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             auto sequenceEditor = std::make_shared<SequenceEditor>(nullptr, nullptr);
             MDStudio::registerElement<SequenceEditor>(L, sequenceEditor);
             return 1;
         }},
        {"__gc", MDStudio::destroyElement<SequenceEditor>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = MDStudio::getElement<SequenceEditor>(L, 1);
             auto e2 = MDStudio::getElement<SequenceEditor>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"setSequence",
         [](lua_State* L) -> int {
             auto sequenceEditor = MDStudio::getElement<SequenceEditor>(L);
             auto sequence = MDStudio::getElement<Sequence>(L, 2);
             sequenceEditor->setSequence(sequence);
             return 0;
         }},
        {"sequence",
         [](lua_State* L) -> int {
             auto sequenceEditor = MDStudio::getElement<SequenceEditor>(L);
             auto sequence = sequenceEditor->sequence();
             MDStudio::registerElement<Sequence>(L, sequence);
             return 1;
         }},
        {"convertToMultiTrack",
         [](lua_State* L) -> int {
             auto sequenceEditor = MDStudio::getElement<SequenceEditor>(L);
             sequenceEditor->convertToMultiTrack();
             return 0;
         }},
        {"convertToSingleTrack",
         [](lua_State* L) -> int {
             auto sequenceEditor = MDStudio::getElement<SequenceEditor>(L);
             sequenceEditor->convertToSingleTrack();
             return 0;
         }},
        {"addTrack",
         [](lua_State* L) -> int {
             auto sequenceEditor = MDStudio::getElement<SequenceEditor>(L);
             auto track = MDStudio::getElement<Track>(L, 2);
             sequenceEditor->addTrack(track);
             return 0;
         }},
        {"setTrackChannel", [](lua_State* L) -> int {
             auto sequenceEditor = MDStudio::getElement<SequenceEditor>(L);
             auto track = MDStudio::getElement<Track>(L, 2);
             auto channel = luaL_checkinteger(L, 3);
             sequenceEditor->setTrackChannel(track, channel);
             return 0;
         }}};
    script->bindTable<SequenceEditor>("SequenceEditor", {sequenceEditorTableDefinition});

    // StudioController
    std::vector<struct luaL_Reg> studioControllerTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             auto studio = MDStudio::getElement<MDStudio::Studio>(L, 1);
             auto sequencer = MDStudio::getElement<MDStudio::Sequencer>(L, 2);
             auto studioController = std::make_shared<StudioController>(nullptr, studio.get(), sequencer.get());
             MDStudio::registerElement<StudioController>(L, studioController);
             return 1;
         }},
        {"__gc", MDStudio::destroyElement<StudioController>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = MDStudio::getElement<StudioController>(L, 1);
             auto e2 = MDStudio::getElement<StudioController>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"setSequence", [](lua_State* L) -> int {
             auto studioController = MDStudio::getElement<StudioController>(L);
             auto sequence = MDStudio::getElement<Sequence>(L, 2);
             studioController->setSequence(sequence);
             return 0;
         }}};
    script->bindTable<StudioController>("StudioController", {studioControllerTableDefinition});

    // Server
    std::vector<struct luaL_Reg> serverTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             auto sequencesDB = MDStudio::getElement<SequencesDB>(L, 1);
             auto server = std::make_shared<Server>(sequencesDB.get());
             MDStudio::registerElement<Server>(L, server);
             return 1;
         }},
        {"__gc", MDStudio::destroyElement<Server>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = MDStudio::getElement<Server>(L, 1);
             auto e2 = MDStudio::getElement<Server>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"start",
         [](lua_State* L) -> int {
             auto server = MDStudio::getElement<Server>(L);
             auto port = static_cast<int>(luaL_checkinteger(L, 2));
             server->start(port);
             return 0;
         }},
        {"stop", [](lua_State* L) -> int {
             auto server = MDStudio::getElement<Server>(L);
             server->stop();
             return 0;
         }}};
    script->bindTable<Server>("Server", {serverTableDefinition});
}
