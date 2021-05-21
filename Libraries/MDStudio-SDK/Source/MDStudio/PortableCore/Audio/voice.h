//
//  voice.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-21.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef VOICE_H
#define VOICE_H

#include <memory>

#include "instrument.h"
#include "types.h"

#define VOICE_FRACTION_BITS 44

namespace MDStudio {

struct Voice {
    UInt32 channel;

    std::shared_ptr<Instrument> instrument;

    Float32 instrumentBasePitch;
    Float64 instrumentSampleRate;

    SampleType* data;
    UInt64 length;
    UInt64 loopStart;
    UInt64 loopEnd;

    UInt64 pos;                // current position
    Float32 pitch;             // pitch of the note
    Float32 velocity;          // velocity of the note
    bool isNoteSustained;      // is the note currently sustained?
    Float32 balance;           // Volume balance of the note (-1.0 = left, 1.0 = right)
    Float32 volEnvFactor;      // volume envelope factor
    UInt32 volEnvHoldCounter;  // volume envelope hold counter

    bool isNoteOn;   // is the note on?
    bool isPlaying;  // is the note currently playing?

    Float32 volumeA, volumeB;      // pre-calculated volume
    Float32 decayRatePerSample;    // pre-calculated decay rate per sample
    Float32 sustainLevel;          // sustain level
    Float32 releaseRatePerSample;  // pre-calculated release rate per sample

    Float32 filterFc;  // Cut-off frequency
    Float32 filterQ;   // Resonance

    GraphSampleType maxOutput;

    unsigned int notesGroupID;  // ID for a group of notes played together at a note on event

    bool isLocked;  // When true, cannot be stolen
};

}  // namespace MDStudio

#endif  // VOICE_H
