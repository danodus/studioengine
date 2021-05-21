//
//  samplerunit.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-21.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#ifndef SAMPLERUNIT_H
#define SAMPLERUNIT_H

#include <memory>
#include <mutex>

#include "Chorus/chorusmodel.h"
#include "DspFilters/Filter.h"
#include "DspFilters/RBJ.h"
#include "Reverb/revmodel.h"
#include "multiinstrument.h"
#include "readerwriterqueue.h"
#include "types.h"
#include "unit.h"
#include "voice.h"

#define SAMPLER_MAX_VOICES 64
#define SAMPLER_MAX_CHANNELS 16

namespace MDStudio {

// ---------------------------------------------------------------------------------------------------------------------
struct SamplerContext {
    struct Voice voices[SAMPLER_MAX_VOICES];
    struct Voice oldVoices[SAMPLER_MAX_VOICES];

    Dsp::FilterDesign<Dsp::RBJ::Design::LowPass, 2>* lowPassFilters[SAMPLER_MAX_VOICES];
    RevModel reverbModel;
    ChorusModel chorusModel;

    Float32 crossFadeFactors[SAMPLER_MAX_VOICES];
    int baseMixerInput;
    UInt32 nbVoices;

    Float32 balance[SAMPLER_MAX_CHANNELS];
    Float32 level[SAMPLER_MAX_CHANNELS];
    Float32 pitchBendValues[SAMPLER_MAX_CHANNELS];
    Float32 modulationValues[SAMPLER_MAX_CHANNELS];
    Float32 expressionValues[SAMPLER_MAX_CHANNELS];
    Float32 reverbValues[SAMPLER_MAX_CHANNELS];
    Float32 chorusValues[SAMPLER_MAX_CHANNELS];
    Float32 pitchBendFactors[SAMPLER_MAX_CHANNELS];

    Float32 lfoFrequency;  // LFO frequency in Hz
    Float32 lfoPhase;      // Normalized phase [0 .. 1]
};

// ---------------------------------------------------------------------------------------------------------------------
struct SamplerCmd {
    UInt32 op;
    Float32 p1;
    Float32 p2;
    UInt32 channel;
    std::shared_ptr<MultiInstrument> multiInstrument;
};

// ---------------------------------------------------------------------------------------------------------------------
class SamplerUnit : public Unit {
    struct SamplerContext _context;
    int _nbVoices;

    Float32 _sustainValues[SAMPLER_MAX_CHANNELS];
    Float32 _balanceValues[SAMPLER_MAX_CHANNELS];
    Float32 _levelValues[SAMPLER_MAX_CHANNELS];
    Float32 _pitchBendValues[SAMPLER_MAX_CHANNELS];
    Float32 _modulationValues[SAMPLER_MAX_CHANNELS];
    Float32 _reverbValues[SAMPLER_MAX_CHANNELS];
    Float32 _chorusValues[SAMPLER_MAX_CHANNELS];
    Float32 _expressionValues[SAMPLER_MAX_CHANNELS];
    Float32 _pitchBendFactors[SAMPLER_MAX_CHANNELS];

    unsigned int _currentNotesGroupID;

    moodycamel::ReaderWriterQueue<SamplerCmd> _cmdQueue, _cmdOutQueue;

    void playNoteCmd(Float32 pitch, Float32 velocity, std::shared_ptr<MultiInstrument> multiInstrument, UInt32 channel);
    void releaseNoteCmd(Float32 pitch, UInt32 channel);
    void stopNoteCmd(Float32 pitch, UInt32 channel);
    void releaseAllNotesCmd(Float32 pitch, UInt32 channel);
    void stopAllNotesPitchCmd(Float32 pitch, UInt32 channel);
    void stopAllNotesCmd(UInt32 channel);
    void setSustainStateCmd(bool state, UInt32 channel);
    void setPitchBendCmd(Float32 pitchBend, UInt32 channel);
    void setModulationCmd(Float32 modulation, UInt32 channel);
    void isNotePlayingCmd(UInt32 channel);
    void isNotePlayingPitchCmd(Float32 pitch, UInt32 channel);
    void clearVoicesCmd(UInt32 channel);
    void clearAllVoicesCmd();
    void isMultiInstrumentInUseCmd(std::shared_ptr<MultiInstrument> multiInstrument);
    void setLevelCmd(Float32 levelValue, UInt32 channel);
    void setBalanceCmd(Float32 balanceValue, UInt32 channel);
    void setReverbCmd(Float32 reverbValue, UInt32 channel);
    void setChorusCmd(Float32 reverbValue, UInt32 channel);
    void setExpressionCmd(Float32 expressionValue, UInt32 channel);
    void setPitchBendFactorCmd(Float32 pitchBendFactor, UInt32 channel);

   public:
    SamplerUnit(UInt32 nbVoices);
    ~SamplerUnit();

    void setNbVoices(UInt32 nbVoices);

    static void renderVoice(struct Voice* voice, UInt64 rate, GraphSampleType* out);
    int renderInput(UInt32 inNumberFrames, GraphSampleType* ioData[2], UInt32 stride = 1) override;

    void playNote(Float32 pitch, Float32 velocity, std::shared_ptr<MultiInstrument> multiInstrument, UInt32 channel);
    void releaseNote(Float32 pitch, UInt32 channel);
    void stopNote(Float32 pitch, UInt32 channel);
    void releaseAllNotes(Float32 pitch, UInt32 channel);
    void stopAllNotes(Float32 pitch, UInt32 channel);
    void stopAllNotes(UInt32 channel);

    void setSustain(Float32 sustain, UInt32 channel);
    Float32 sustain(UInt32 channel);

    void setPitchBend(Float32 pitchBend, UInt32 channel);
    Float32 pitchBend(UInt32 channel);

    void setModulation(Float32 modulation, UInt32 channel);
    Float32 modulation(UInt32 channel);

    bool isNotePlaying(UInt32 channel);
    bool isNotePlaying(Float32 pitch, UInt32 channel);

    void clearVoices(UInt32 channel);
    void clearAllVoices();

    bool isMultiInstrumentInUse(std::shared_ptr<MultiInstrument> multiInstrument);

    void setLevel(Float32 levelValue, UInt32 channel);
    Float32 level(UInt32 channel);

    void setBalance(Float32 balanceValue, UInt32 channel);
    Float32 balance(UInt32 channel);

    void setReverb(Float32 reverbValue, UInt32 channel);
    Float32 reverb(UInt32 channel);

    void setChorus(Float32 chorusValue, UInt32 channel);
    Float32 chorus(UInt32 channel);

    void setExpression(Float32 expressionValue, UInt32 channel);
    Float32 expression(UInt32 channel);

    void setPitchBendFactor(Float32 pitchBendFactor, UInt32 channel);
    Float32 pitchBendFactor(UInt32 channel);
};

}  // namespace MDStudio

#endif  // SAMPLERUNIT_H
