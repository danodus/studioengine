//
//  soundfont2.h
//  MDStudio
//
//  Created by Daniel Cliche on 2018-02-24.
//  Copyright © 2018-2020 Daniel Cliche. All rights reserved.
//

#ifndef SOUNDFONT2_H
#define SOUNDFONT2_H

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

#include "../types.h"

namespace MDStudio {

typedef UInt16 SoundFont2Modulator;
typedef UInt16 SoundFont2Generator;
typedef UInt16 SoundFont2Transform;
typedef UInt16 SoundFont2SampleLink;

#pragma pack(push, 1)

struct SoundFont2Ranges {
    UInt8 lo;
    UInt8 hi;
};

union SoundFont2GenAmount {
    SoundFont2Ranges ranges;
    SInt16 shAmount;
    UInt16 wAmount;
};

struct SoundFont2Version {
    UInt16 minor;
    UInt16 major;
};

struct SoundFont2PresetHeader {
    SInt8 name[20];
    UInt16 preset;
    UInt16 bank;
    UInt16 presetBagNdx;
    UInt32 library;
    UInt32 genre;
    UInt32 morphology;
};

// Preset zones
struct SoundFont2PresetBag {
    UInt16 genNdx;
    UInt16 modNdx;
};

struct SoundFont2ModList {
    SoundFont2Modulator srcOper;
    SoundFont2Generator destOper;
    SInt16 amount;
    SoundFont2Modulator amtSrcOper;
    SoundFont2Transform transOper;
};

struct SoundFont2GenList {
    SoundFont2Generator oper;
    SoundFont2GenAmount amount;
};

struct SoundFont2Inst {
    SInt8 name[20];
    UInt16 bagNdx;
};

struct SoundFont2InstBag {
    UInt16 genNdx;
    UInt16 modNdx;
};

struct SoundFont2InstModList {
    SoundFont2Modulator srcOper;
    SoundFont2Generator destOper;
    SInt16 amount;
    SoundFont2Modulator amtSrcOper;
    SoundFont2Transform transOper;
};

struct SoundFont2InstGenList {
    SoundFont2Generator oper;
    SoundFont2GenAmount amount;
};

struct SoundFont2Sample {
    SInt8 name[20];
    UInt32 start;
    UInt32 end;
    UInt32 startLoop;
    UInt32 endLoop;
    UInt32 sampleRate;
    UInt8 originalKey;
    SInt8 correction;
    UInt16 sampleLink;
    SoundFont2SampleLink sampleType;
};

#pragma pack(pop)

struct SoundFont2Data {
    // Oscillator
    UInt32 start;  // sample start address
    UInt32 end;
    UInt32 startLoop;  // loop start address
    UInt32 endLoop;    // loop end address
    UInt32 sampleRate;
    SInt16 origKeyAndCorr;
    SInt16 sampleModes;
    // SInt16 sampleLink;

    // Pitch
    SInt16 coarseTune;
    SInt16 fineTune;
    // SInt16 scaleTuning;
    // SInt16 modLfoToPitch;        // main fm: modLfo-> pitch
    // SInt16 vibLfoToPitch;        // aux fm:  vibLfo-> pitch
    // SInt16 modEnvToPitch;        // pitch env: modEnv(aux)-> pitch

    // Filter
    SInt16 initialFilterFc;  // initial filter cutoff
    SInt16 initialFilterQ;   // filter Q
    // SInt16 modLfoToFilterFc;     // modLfo -> filter * cutoff
    // SInt16 modEnvToFilterFc;     // mod env(aux)-> filter * cutoff

    // Amplifier
    // SInt16 instVol;
    // SInt16 modLfoToVolume;       // tremolo: modLfo-> volume

    // Effects
    // SInt16 chorusEffectsSend;    // chorus
    // SInt16 reverbEffectsSend;    // reverb
    SInt16 panEffectsSend;  // pan

    // Modulation Low Frequency Oscillator
    // SInt16 delayModLfo;          // delay
    // SInt16 freqModLfo;           // frequency

    // Vibrato (Pitch only) Low Frequency Oscillator
    // SInt16 delayVibLfo;          // delay
    // SInt16 freqVibLfo;           // frequency

    // Modulation Envelope
    // SInt16 delayModEnv;          // delay
    // SInt16 attackModEnv;         // attack
    // SInt16 holdModEnv;           // hold
    // SInt16 decayModEnv;          // decay
    // SInt16 sustainModEnv;        // sustain
    // SInt16 releaseModEnv;        // release
    // SInt16 autoHoldModEnv;
    // SInt16 autoDecayModEnv;

    // Attenuation (Volume only) Envelope
    // SInt16 delayVolEnv;          // delay
    SInt16 attackVolEnv;   // attack
    SInt16 holdVolEnv;     // hold
    SInt16 decayVolEnv;    // decay
    SInt16 sustainVolEnv;  // sustain
    SInt16 releaseVolEnv;  // release
    // SInt16 autoHoldVolEnv;
    // SInt16 autoDecayVolEnv;

    // Zone limits
    SInt16 minPitch;
    SInt16 maxPitch;
    SInt16 minVelocity;
    SInt16 maxVelocity;

    //
    // Additional internal elements
    //

    bool isInitialFilterFcSet;
    bool isInitialFilterQSet;
    bool isPanEffectsSendSet;
    bool isAttackVolEnvSet;
    bool isHoldVolEnvSet;
    bool isDecayVolEnvSet;
    bool isSustainVolEnvSet;
    bool isReleaseVolEnvSet;
    bool isKeyRangeSet;
    bool isVelocityRangeSet;
    bool isRootKeySet;
    bool isCoarseTuneSet;
    bool isFineTuneSet;
    bool isSampleModesSet;
    bool isStartLoopOffsetSet;
    bool isEndLoopOffsetSet;

    SInt16 rootKey;
    SInt16 startLoopOffset;
    SInt16 endLoopOffset;
};

struct RIFFChunk {
    UInt8 id[4] = {0, 0, 0, 0};  // Identifies the type of data within the chunk
    UInt32 size = 0;             // Size of the chunk data bytes, excluding any pad byte
};

class SoundFont2 {
    typedef enum {
        Unknown = -1,
        RIFF,
        LIST,
        SFBK,
        INFO,
        SDTA,
        PDTA,
        IFIL,
        ISNG,
        IROM,
        IVER,
        INAM,
        ICRD,
        IENG,
        IPRD,
        ICOP,
        ICMT,
        ISFT,
        SMPL,
        PHDR,
        PBAG,
        PMOD,
        PGEN,
        INST,
        IBAG,
        IMOD,
        IGEN,
        SHDR
    } Types;
    Types getType(UInt8 id[4]);

    bool readTop(std::ifstream& ifs);
    bool readSFBK(std::ifstream& ifs);

    bool readChunk(std::vector<char>* data, Types expectedType, std::ifstream& ifs);
    bool readChunk(char* data, size_t size, Types expectedType, std::ifstream& ifs);
    bool readChunk(std::string* s, Types expectedType, std::ifstream& ifs);
    template <class T>
    bool readChunk(std::vector<T>* v, Types expectedType, bool isTerminalSkipped, std::ifstream& ifs);

    void skipChunk(RIFFChunk& riffChunk, std::ifstream& ifs);

    void processGenOperator(SoundFont2Data* datum, SoundFont2Generator oper, SoundFont2GenAmount amount);
    void combineDatum(SoundFont2Data* destDatum, SoundFont2Data* srcDatum, bool isAdded);

    SoundFont2Version _fileVersion;
    std::string _soundEngine;
    std::string _name;
    std::string _creationDate;
    std::string _engineers;
    std::string _product;
    std::string _copyright;
    std::string _comments;
    std::string _editingSoftware;
    std::vector<SoundFont2PresetHeader> _presetHeaders;
    std::vector<SoundFont2PresetBag> _presetBags;
    std::vector<SoundFont2ModList> _modLists;
    std::vector<SoundFont2GenList> _genLists;
    std::vector<SoundFont2Inst> _instruments;
    std::vector<SoundFont2InstBag> _instBags;
    std::vector<SoundFont2InstModList> _instModLists;
    std::vector<SoundFont2InstGenList> _instGenLists;
    std::vector<SoundFont2Sample> _samples;

    UInt32 _samplesOffset;

   public:
    bool load(std::string path);

    SoundFont2Version fileVersion() { return _fileVersion; }
    std::string soundEngine() { return _soundEngine; }
    std::string name() { return _name; }
    std::string creationDate() { return _creationDate; }
    std::string engineers() { return _engineers; }
    std::string product() { return _product; }
    std::string copyright() { return _copyright; }
    std::string comments() { return _comments; }
    std::string editingSoftware() { return _editingSoftware; }

    std::vector<SoundFont2PresetHeader> presetHeaders();
    UInt32 samplesOffset() { return _samplesOffset; }

    std::vector<SoundFont2Data> dataForPreset(UInt16 bank, UInt16 preset);
};

}  // namespace MDStudio

#endif  // SOUNDFONT2_H
