//
//  melobasescriptmodule.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2019-09-16.
//  Copyright (c) 2019-2021 Daniel Cliche. All rights reserved.
//

#include "melobasescriptmodule.h"

#include "midihub.h"
#include "topview.h"
#include "topviewcontroller.h"

// ---------------------------------------------------------------------------------------------------------------------
MelobaseScriptModule::MelobaseScriptModule(MelobaseCore::SequencesDB* sequencesDB,
                                           MelobaseCore::SequenceEditor* sequenceEditor)
    : _sequencesDB(sequencesDB), _sequenceEditor(sequenceEditor) {}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseScriptModule::init(MDStudio::Script* script) {
    script->setGlobal("melobaseScriptModule", this);

    script->bindFunction("getSequencesDB", [](lua_State* L) -> int {
        lua_getglobal(L, "melobaseScriptModule");
        auto melobaseScriptModule = static_cast<MelobaseScriptModule*>(lua_touserdata(L, -1));
        auto sequencesDB = std::shared_ptr<MelobaseCore::SequencesDB>(
            melobaseScriptModule->sequencesDB(), MDStudio::BypassDeleter<MelobaseCore::SequencesDB>());
        MDStudio::registerElement<MelobaseCore::SequencesDB>(L, sequencesDB);
        return 1;
    });

    script->bindFunction("getSequenceEditor", [](lua_State* L) -> int {
        lua_getglobal(L, "melobaseScriptModule");
        auto melobaseScriptModule = static_cast<MelobaseScriptModule*>(lua_touserdata(L, -1));
        auto sequenceEditor = std::shared_ptr<MelobaseCore::SequenceEditor>(
            melobaseScriptModule->sequenceEditor(), MDStudio::BypassDeleter<MelobaseCore::SequenceEditor>());
        MDStudio::registerElement<MelobaseCore::SequenceEditor>(L, sequenceEditor);
        return 1;
    });

    // TopView
    std::vector<struct luaL_Reg> topViewTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             auto name = luaL_checkstring(L, 1);
             auto studio = MDStudio::getElement<MDStudio::Studio>(L, 2);
             auto eventTickWidth = luaL_checknumber(L, 3);
             auto eventHeight = luaL_checknumber(L, 4);
             auto topView = std::make_shared<TopView>(name, nullptr, studio.get(), eventTickWidth, eventHeight);
             MDStudio::registerElement<TopView, MDStudio::View>(L, topView);
             return 1;
         }},
        {"__gc", MDStudio::destroyElement<TopView, MDStudio::View>},
        {"__eq", [](lua_State* L) -> int {
             auto e1 = MDStudio::getElement<TopView>(L, 1);
             auto e2 = MDStudio::getElement<TopView>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }}};

    script->bindTable<TopView, MDStudio::View>("TopView", {topViewTableDefinition, script->tables()["View"].at(1)});

    // TopViewController
    std::vector<struct luaL_Reg> topViewControllerTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             auto topView = MDStudio::getElement<TopView>(L, 1);
             auto studioController = MDStudio::getElement<MelobaseCore::StudioController>(L, 2);
             auto sequencesDB = MDStudio::getElement<MelobaseCore::SequencesDB>(L, 3);
             auto server = MDStudio::getElement<MelobaseCore::Server>(L, 4);
             auto defaultMIDIInputPortName = luaL_checkstring(L, 5);
             auto defaultMIDIOutputPortName = luaL_checkstring(L, 6);
             if (!lua_isboolean(L, 7)) luaL_error(L, "Boolean expected");
             auto isFadeInEnabled = lua_toboolean(L, 7) != 0;
             auto topViewController =
                 std::make_shared<TopViewController>(topView, studioController.get(), sequencesDB.get(), server.get(),
                                                     defaultMIDIInputPortName, defaultMIDIOutputPortName, isFadeInEnabled);
             MDStudio::registerElement<TopViewController>(L, topViewController);
             return 1;
         }},
        {"__gc", MDStudio::destroyElement<TopViewController>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = MDStudio::getElement<TopViewController>(L, 1);
             auto e2 = MDStudio::getElement<TopViewController>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"undo",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->undo();
             return 0;
         }},
        {"redo",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->redo();
             return 0;
         }},
        {"editCut",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->editCut();
             return 0;
         }},
        {"editCopy",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->editCopy();
             return 0;
         }},
        {"editPaste",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->editPaste();
             return 0;
         }},
        {"editDelete",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->editDelete();
             return 0;
         }},
        {"editQuantize",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->editQuantize();
             return 0;
         }},
        {"editSelectAll",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->editSelectAll();
             return 0;
         }},
        {"setMetronomeSound",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             if (!lua_isboolean(L, 2)) luaL_error(L, "Boolean expected");
             auto state = lua_toboolean(L, 2);
             topViewController->setMetronomeSound(state != 0);
             return 0;
         }},
        {"goToBeginning",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->goToBeginning();
             return 0;
         }},
        {"playPause",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->playPause();
             return 0;
         }},
        {"record",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->record();
             return 0;
         }},
        {"newSequence",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->newSequence();
             return 0;
         }},
        {"newSubfolder",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->newSubfolder();
             return 0;
         }},
        {"convertSequence",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->convertSequence();
             return 0;
         }},
        {"promoteAllSequences",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->promoteAllSequences();
             return 0;
         }},
        {"demoteAllSequences",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->demoteAllSequences();
             return 0;
         }},
        {"setAsPlayedAllSequences",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->setAsPlayedAllSequences();
             return 0;
         }},
        {"cleanupSequences",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->cleanupSequences();
             return 0;
         }},
        {"emptyTrash",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->emptyTrash();
             return 0;
         }},
        {"moveToTrashSequence",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->moveToTrashSequence();
             return 0;
         }},
        {"showPreferences",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->showPreferences();
             return 0;
         }},
        {"about",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->about();
             return 0;
         }},
        {"showHideStudio",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->showHideStudio();
             return 0;
         }},
        {"showHideDatabase",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->showHideDatabase();
             return 0;
         }},
        {"showHideFolders",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->showHideFolders();
             return 0;
         }},
        {"showHideProperties",
         [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->showHideProperties();
             return 0;
         }},
        {"showHideControllerEvents", [](lua_State* L) -> int {
             auto topViewController = MDStudio::getElement<TopViewController>(L);
             topViewController->showHideControllerEvents();
             return 0;
         }}};

    script->bindTable<TopViewController>("TopViewController", {topViewControllerTableDefinition});

    // MIDIHub
    std::vector<struct luaL_Reg> midiHubTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             auto midiHub = std::make_shared<MIDIHub>();
             MDStudio::registerElement<MIDIHub>(L, midiHub);
             return 1;
         }},
        {"__gc", MDStudio::destroyElement<MIDIHub>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = MDStudio::getElement<MIDIHub>(L, 1);
             auto e2 = MDStudio::getElement<MIDIHub>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"defaultMIDIInputPortName",
         [](lua_State* L) -> int {
             auto midiHub = MDStudio::getElement<MIDIHub>(L);
             lua_pushstring(L, midiHub->defaultMIDIInputPortName().c_str());
             return 1;
         }},
        {"defaultMIDIOutputPortName",
         [](lua_State* L) -> int {
             auto midiHub = MDStudio::getElement<MIDIHub>(L);
             lua_pushstring(L, midiHub->defaultMIDIOutputPortName().c_str());
             return 1;
         }},
        {"start", [](lua_State* L) -> int {
             auto midiHub = MDStudio::getElement<MIDIHub>(L);
             auto topViewController = MDStudio::getElement<TopViewController>(L, 2);
             midiHub->start(topViewController.get());
             return 0;
         }}};

    script->bindTable<MIDIHub>("MIDIHub", {midiHubTableDefinition});
}
