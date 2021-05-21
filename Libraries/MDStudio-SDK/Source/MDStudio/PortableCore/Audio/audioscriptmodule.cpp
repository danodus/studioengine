//
//  audioscriptmodule.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2018-04-07.
//  Copyright © 2018-2020 Daniel Cliche. All rights reserved.
//

#include "audioscriptmodule.h"

#include <platform.h>

// Audio

#include "instrumentmanager.h"
#include "metronome.h"
#include "mixer.h"
#include "samplerunit.h"
#include "sineunit.h"
#if !TARGET_OS_IPHONE
#include "midi.h"
#endif
#include "midifile.h"
#include "sequencer.h"
#include "studio.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
void AudioScriptModule::init(Script* script) {
    // Mixer
    std::vector<struct luaL_Reg> mixerTableDefinition = {{"new",
                                                          [](lua_State* L) -> int {
                                                              auto mixer = std::make_shared<Mixer>();
                                                              registerElement<Mixer>(L, mixer);
                                                              return 1;
                                                          }},
                                                         {"__gc", destroyElement<Mixer>},
                                                         {"__eq",
                                                          [](lua_State* L) -> int {
                                                              auto e1 = getElement<Mixer>(L, 1);
                                                              auto e2 = getElement<Mixer>(L, 2);
                                                              lua_pushboolean(L, e1 == e2);
                                                              return 1;
                                                          }},
                                                         {"setOutputLatency",
                                                          [](lua_State* L) -> int {
                                                              auto mixer = getElement<Mixer>(L);
                                                              auto outputLatency = luaL_checknumber(L, 2);
                                                              mixer->setOutputLatency(outputLatency);
                                                              return 0;
                                                          }},
                                                         {"outputLatency",
                                                          [](lua_State* L) -> int {
                                                              auto mixer = getElement<Mixer>(L);
                                                              lua_pushnumber(L, mixer->outputLatency());
                                                              return 1;
                                                          }},
                                                         {"start",
                                                          [](lua_State* L) -> int {
                                                              auto mixer = getElement<Mixer>(L);
                                                              mixer->start();
                                                              return 0;
                                                          }},
                                                         {"addUnit", [](lua_State* L) -> int {
                                                              auto mixer = getElement<Mixer>(L);
                                                              auto unit = getElement<Unit>(L, 2);
                                                              mixer->addUnit(unit);
                                                              return 0;
                                                          }}};
    script->bindTable<Mixer>("Mixer", {mixerTableDefinition});

    // MultiInstrument
    std::vector<struct luaL_Reg> multiInstrumentTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             auto multiInstrument = std::make_shared<MultiInstrument>();
             registerElement<MultiInstrument>(L, multiInstrument);
             return 1;
         }},
        {"__gc", destroyElement<MultiInstrument>},
        {"__eq", [](lua_State* L) -> int {
             auto e1 = getElement<MultiInstrument>(L, 1);
             auto e2 = getElement<MultiInstrument>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }}};
    script->bindTable<MultiInstrument>("MultiInstrument", {multiInstrumentTableDefinition});

    // InstrumentManager
    std::vector<struct luaL_Reg> instrumentManagerTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             auto instrumentManager = std::make_shared<InstrumentManager>(Platform::sharedInstance()->resourcesPath());
             registerElement<InstrumentManager>(L, instrumentManager);
             return 1;
         }},
        {"__gc", destroyElement<InstrumentManager>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<InstrumentManager>(L, 1);
             auto e2 = getElement<InstrumentManager>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"loadSF2MultiInstrument",
         [](lua_State* L) -> int {
             auto instrumentManager = getElement<InstrumentManager>(L);
             auto name = luaL_checkstring(L, 2);
             auto presetBank = luaL_checkinteger(L, 3);
             auto preset = luaL_checkinteger(L, 4);
             auto multiInstrument = instrumentManager->loadSF2MultiInstrument(name, static_cast<int>(presetBank),
                                                                              static_cast<int>(preset));
             registerElement<MultiInstrument>(L, multiInstrument);
             return 1;
         }},
        {"unloadMultiInstrument",
         [](lua_State* L) -> int {
             auto instrumentManager = getElement<InstrumentManager>(L);
             auto multiInstrument = getElement<MultiInstrument>(L, 2);
             instrumentManager->unloadMultiInstrument(multiInstrument);
             return 0;
         }},
        {"getSF2Presets", [](lua_State* L) -> int {
             auto instrumentManager = getElement<InstrumentManager>(L);
             auto name = luaL_checkstring(L, 2);
             auto presets = instrumentManager->getSF2Presets(name);

             lua_newtable(L);

             lua_Integer i = 1;
             for (auto& preset : presets) {
                 lua_pushinteger(L, i);

                 lua_newtable(L);

                 lua_pushinteger(L, preset._bank);
                 lua_setfield(L, -2, "bank");

                 lua_pushinteger(L, preset._number);
                 lua_setfield(L, -2, "number");

                 lua_pushstring(L, preset._name.c_str());
                 lua_setfield(L, -2, "name");

                 lua_settable(L, -3);

                 ++i;
             }

             return 1;
         }}};
    script->bindTable<InstrumentManager>("InstrumentManager", {instrumentManagerTableDefinition});

    // SineUnit
    std::vector<struct luaL_Reg> sineUnitTableDefinition = {{"new",
                                                             [](lua_State* L) -> int {
                                                                 auto sineUnit = std::make_shared<SineUnit>();
                                                                 registerElement<SineUnit, Unit>(L, sineUnit);
                                                                 return 1;
                                                             }},
                                                            {"__gc", destroyElement<SineUnit, Unit>},
                                                            {"__eq",
                                                             [](lua_State* L) -> int {
                                                                 auto e1 = getElement<SineUnit>(L, 1);
                                                                 auto e2 = getElement<SineUnit>(L, 2);
                                                                 lua_pushboolean(L, e1 == e2);
                                                                 return 1;
                                                             }},
                                                            {"noteOn",
                                                             [](lua_State* L) -> int {
                                                                 auto sineUnit = getElement<SineUnit>(L);
                                                                 auto pitch = luaL_checknumber(L, 2);
                                                                 auto velocity = luaL_checknumber(L, 3);
                                                                 sineUnit->noteOn(pitch, velocity);
                                                                 return 0;
                                                             }},
                                                            {"noteOff", [](lua_State* L) -> int {
                                                                 auto sineUnit = getElement<SineUnit>(L);
                                                                 sineUnit->noteOff();
                                                                 return 0;
                                                             }}};
    script->bindTable<SineUnit, Unit>("SineUnit", {sineUnitTableDefinition});

    // SamplerUnit
    std::vector<struct luaL_Reg> samplerUnitTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             auto nbVoices = luaL_checkinteger(L, 1);
             auto samplerUnit = std::make_shared<SamplerUnit>(nbVoices);
             registerElement<SamplerUnit, Unit>(L, samplerUnit);
             return 1;
         }},
        {"__gc", destroyElement<SamplerUnit, Unit>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<SamplerUnit>(L, 1);
             auto e2 = getElement<SamplerUnit>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"playNote",
         [](lua_State* L) -> int {
             auto samplerUnit = getElement<SamplerUnit>(L);
             auto pitch = luaL_checknumber(L, 2);
             auto velocity = luaL_checknumber(L, 3);
             auto instrument = getElement<MultiInstrument>(L, 4);
             auto channel = luaL_checkinteger(L, 5);
             samplerUnit->playNote(pitch, velocity, instrument, static_cast<UInt32>(channel));
             return 0;
         }},
        {"releaseNote",
         [](lua_State* L) -> int {
             auto samplerUnit = getElement<SamplerUnit>(L);
             auto pitch = luaL_checknumber(L, 2);
             auto channel = luaL_checkinteger(L, 3);
             samplerUnit->releaseNote(pitch, static_cast<UInt32>(channel));
             return 0;
         }},
        {"setPitchBend",
         [](lua_State* L) -> int {
             auto samplerUnit = getElement<SamplerUnit>(L);
             auto pitchBend = luaL_checknumber(L, 2);
             auto channel = luaL_checkinteger(L, 3);
             samplerUnit->setPitchBend(pitchBend, static_cast<UInt32>(channel));
             return 0;
         }},
        {"setSustain",
         [](lua_State* L) -> int {
             auto samplerUnit = getElement<SamplerUnit>(L);
             auto sustain = luaL_checknumber(L, 2);
             auto channel = luaL_checkinteger(L, 3);
             samplerUnit->setSustain(sustain, static_cast<UInt32>(channel));
             return 0;
         }},
        {"setModulation",
         [](lua_State* L) -> int {
             auto samplerUnit = getElement<SamplerUnit>(L);
             auto modulation = luaL_checknumber(L, 2);
             auto channel = luaL_checkinteger(L, 3);
             samplerUnit->setModulation(modulation, static_cast<UInt32>(channel));
             return 0;
         }},
        {"setLevel",
         [](lua_State* L) -> int {
             auto samplerUnit = getElement<SamplerUnit>(L);
             auto level = luaL_checknumber(L, 2);
             auto channel = luaL_checkinteger(L, 3);
             samplerUnit->setLevel(level, static_cast<UInt32>(channel));
             return 0;
         }},
        {"setBalance",
         [](lua_State* L) -> int {
             auto samplerUnit = getElement<SamplerUnit>(L);
             auto balance = luaL_checknumber(L, 2);
             auto channel = luaL_checkinteger(L, 3);
             samplerUnit->setBalance(balance, static_cast<UInt32>(channel));
             return 0;
         }},
        {"setReverb",
         [](lua_State* L) -> int {
             auto samplerUnit = getElement<SamplerUnit>(L);
             auto reverb = luaL_checknumber(L, 2);
             auto channel = luaL_checkinteger(L, 3);
             samplerUnit->setReverb(reverb, static_cast<UInt32>(channel));
             return 0;
         }},
        {"setChorus",
         [](lua_State* L) -> int {
             auto samplerUnit = getElement<SamplerUnit>(L);
             auto chorus = luaL_checknumber(L, 2);
             auto channel = luaL_checkinteger(L, 3);
             samplerUnit->setChorus(chorus, static_cast<UInt32>(channel));
             return 0;
         }},
        {"setExpression", [](lua_State* L) -> int {
             auto samplerUnit = getElement<SamplerUnit>(L);
             auto expression = luaL_checknumber(L, 2);
             auto channel = luaL_checkinteger(L, 3);
             samplerUnit->setExpression(expression, static_cast<UInt32>(channel));
             return 0;
         }}};
    script->bindTable<SamplerUnit, Unit>("SamplerUnit", {samplerUnitTableDefinition});

    // Metronome
    std::vector<struct luaL_Reg> metronomeTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             auto metronome = std::make_shared<Metronome>();
             registerElement<Metronome>(L, metronome);
             return 1;
         }},
        {"__gc", destroyElement<Metronome>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<Metronome>(L, 1);
             auto e2 = getElement<Metronome>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"start",
         [](lua_State* L) -> int {
             auto metronome = getElement<Metronome>(L);
             metronome->start();
             return 0;
         }},
        {"stop",
         [](lua_State* L) -> int {
             auto metronome = getElement<Metronome>(L);
             metronome->stop();
             return 0;
         }},
        {"setDidTickFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto metronome = getElement<Metronome>(L);
             lua_getglobal(L, "script");
             MDStudio::Script* script = (MDStudio::Script*)lua_touserdata(L, -1);
             metronome->setDidTickFn([=](Metronome*) -> bool {
                 Platform::sharedInstance()->invoke([=] { script->callCallback(L, callbackRef); }, script);
                 return true;
             });
             return 0;
         }},
        {"tick", [](lua_State* L) -> int {
             auto metronome = getElement<Metronome>(L);
             lua_pushinteger(L, metronome->tick());
             return 1;
         }}};
    script->bindTable<Metronome>("Metronome", {metronomeTableDefinition});

#if !TARGET_OS_IPHONE

    // MIDIIn
    std::vector<struct luaL_Reg> midiInTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             auto midiIn = std::make_shared<MIDIIn>();
             registerElement<MIDIIn>(L, midiIn);
             return 1;
         }},
        {"__gc", destroyElement<MIDIIn>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<MIDIIn>(L, 1);
             auto e2 = getElement<MIDIIn>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"getPortCount",
         [](lua_State* L) -> int {
             auto midiIn = getElement<MIDIIn>(L);
             lua_pushinteger(L, midiIn->getPortCount());
             return 1;
         }},
        {"getPortName",
         [](lua_State* L) -> int {
             auto midiIn = getElement<MIDIIn>(L);
             auto port = static_cast<unsigned int>(luaL_checkinteger(L, 2));
             lua_pushstring(L, midiIn->getPortName(port).c_str());
             return 1;
         }},
        {"openPort",
         [](lua_State* L) -> int {
             auto midiIn = getElement<MIDIIn>(L);
             auto port = static_cast<unsigned int>(luaL_checkinteger(L, 2));
             midiIn->openPort(port);
             return 1;
         }},
        {"closePort",
         [](lua_State* L) -> int {
             auto midiIn = getElement<MIDIIn>(L);
             midiIn->closePort();
             return 1;
         }},
        {"setCallback", [](lua_State* L) -> int {
             // Stack: midiIn, callback

             // store the reference to the Lua function in a variable to be used later
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);

             // Stack: midiIn

             auto midiIn = getElement<MIDIIn>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);

             midiIn->setDidReceiveMessageFn(
                 [=](MIDIIn* sender, double deltatime, const std::vector<unsigned char>& message) {
                     MDStudio::Platform::sharedInstance()->invoke([=]() {
                         // Push the callback onto the stack using the Lua reference we stored in the registry
                         lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                         registerElement<MIDIIn>(L, std::shared_ptr<MIDIIn>(sender, BypassDeleter<MIDIIn>()));
                         lua_pushnumber(L, deltatime);

                         lua_newtable(L);
                         lua_Integer i = 1;
                         for (auto m : message) {
                             lua_pushinteger(L, i);
                             lua_pushinteger(L, m);
                             lua_settable(L, -3);
                             ++i;
                         }

                         if (lua_pcall(L, 3, 0, 0) != 0) {
                             script->error(lua_tostring(L, -1));
                         }
                     });
                 });

             return 0;
         }}};
    script->bindTable<MIDIIn>("MIDIIn", {midiInTableDefinition});

#endif  // !TARGET_OS_IPHONE

    // Studio
    std::vector<struct luaL_Reg> studioTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             auto nbChannels = luaL_checkinteger(L, 1);
             auto nbVoices = luaL_checkinteger(L, 2);
             auto studio = std::make_shared<Studio>(nbChannels, Platform::sharedInstance()->resourcesPath(), nbVoices);
             registerElement<Studio>(L, studio);
             return 1;
         }},
        {"__gc", destroyElement<Studio>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<Studio>(L, 1);
             auto e2 = getElement<Studio>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"startMixer", [](lua_State* L) -> int {
             auto studio = getElement<Studio>(L);
             studio->startMixer();
             return 0;
         }}};
    script->bindTable<Studio>("Studio", {studioTableDefinition});

    // Sequence
    std::vector<struct luaL_Reg> sequenceTableDefinition = {{"new",
                                                             [](lua_State* L) -> int {
                                                                 auto sequence = std::make_shared<Sequence>();
                                                                 registerElement<Sequence>(L, sequence);
                                                                 return 1;
                                                             }},
                                                            {"__gc", destroyElement<Sequence>},
                                                            {"__eq", [](lua_State* L) -> int {
                                                                 auto e1 = getElement<Sequence>(L, 1);
                                                                 auto e2 = getElement<Sequence>(L, 2);
                                                                 lua_pushboolean(L, e1 == e2);
                                                                 return 1;
                                                             }}};
    script->bindTable<Sequence>("Sequence", {sequenceTableDefinition});

    // Sequencer
    std::vector<struct luaL_Reg> sequencerTableDefinition = {{"new",
                                                              [](lua_State* L) -> int {
                                                                  auto studio = getElement<Studio>(L, 1);
                                                                  auto sequencer = std::make_shared<Sequencer>(
                                                                      nullptr, studio.get());
                                                                  registerElement<Sequencer>(L, sequencer);
                                                                  return 1;
                                                              }},
                                                             {"__gc", destroyElement<Sequencer>},
                                                             {"__eq",
                                                              [](lua_State* L) -> int {
                                                                  auto e1 = getElement<Sequencer>(L, 1);
                                                                  auto e2 = getElement<Sequencer>(L, 2);
                                                                  lua_pushboolean(L, e1 == e2);
                                                                  return 1;
                                                              }},
                                                             {"setSequence",
                                                              [](lua_State* L) -> int {
                                                                  auto sequencer = getElement<Sequencer>(L, 1);
                                                                  auto sequence = getElement<Sequence>(L, 2);
                                                                  sequencer->setSequence(sequence);
                                                                  return 0;
                                                              }},
                                                             {"play", [](lua_State* L) -> int {
                                                                  auto sequencer = getElement<Sequencer>(L, 1);
                                                                  sequencer->play();
                                                                  return 0;
                                                              }}};
    script->bindTable<Sequencer>("Sequencer", {sequencerTableDefinition});

    script->bindFunction("readMIDIFile", [](lua_State* L) -> int {
        auto path = luaL_checkstring(L, 1);
        auto sequence = readMIDIFile(path);
        registerElement<Sequence>(L, sequence);
        return 1;
    });
}
