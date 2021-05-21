//
//  instrument.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-21.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <memory>
#include <string>

#include "sample.h"
#include "types.h"

namespace MDStudio {

class Instrument {
    // Audio data
    std::string _audioFileName;  // name of the audio file
    Float32 _loopStart;          // loop position (set to length for no looping)
    Float32 _loopEnd;            // loop position (set to length for no looping)
    Float32 _basePitch;          // base pitch of the sample (69 = A4)
    Float32 _baseVelocity;       // base velocity of the note (0 = min, 1 = max)

    // Sample
    std::shared_ptr<Sample> _sample;  // sample data associated to this instrument

    // Pitch range
    Float32 _lowestPitch;   // Lowest pitch playable by the instrument (69 = A4)
    Float32 _highestPitch;  // Highest pitch playable by the instrument (69 = A4)

    // Velocity range
    Float32 _lowestVelocity;   // Lowest velocity playable by the instrument
    Float32 _highestVelocity;  // Highest velocity playable by the instrument

    // Balance range
    Float32 _lowestBalance;   // Volume balance for lowest pitch (-1.0 = left, 1.0 = right)
    Float32 _highestBalance;  // Volume balance for highest pitch (-1.0 = left, 1.0 = right)

    // ASDR volume envelope
    Float32 _attackRate;  // in seconds
    Float32 _holdTime;    // in seconds
    Float32 _decayRate;   // in seconds
    Float32 _sustainLevel;
    Float32 _releaseRate;  // in seconds

    Float64 _SF2SampleRate;
    SInt64 _SF2SampleStart;
    SInt64 _SF2SampleEnd;
    SInt64 _SF2SampleBasePos;

    // Filter
    Float32 _filterFc;  // Cut-off frequency
    Float32 _filterQ;   // Resonance

   public:
    Instrument();

    std::shared_ptr<Sample> loadSF2Sample(const std::string& path);

    // Audio data
    void setSample(std::shared_ptr<Sample> sample) { _sample = sample; }
    std::shared_ptr<Sample> sample() { return _sample; }

    void setLoopStart(Float32 loopStart) { _loopStart = loopStart; }
    Float32 loopStart() { return _loopStart; }

    void setLoopEnd(Float32 loopEnd) { _loopEnd = loopEnd; }
    Float32 loopEnd() { return _loopEnd; }

    void setAudioFilename(std::string audioFileName) { _audioFileName = audioFileName; }

    // Pitch range

    void setBasePitch(Float32 basePitch) { _basePitch = basePitch; }
    Float32 basePitch() { return _basePitch; }

    void setLowestPitch(Float32 lowestPitch) { _lowestPitch = lowestPitch; }
    Float32 lowestPitch() { return _lowestPitch; }

    void setHighestPitch(Float32 highestPitch) { _highestPitch = highestPitch; }
    Float32 highestPitch() { return _highestPitch; }

    // Velocity range

    void setBaseVelocity(Float32 baseVelocity) { _baseVelocity = baseVelocity; }
    Float32 baseVelocity() { return _baseVelocity; }

    void setLowestVelocity(Float32 lowestVelocity) { _lowestVelocity = lowestVelocity; }
    Float32 lowestVelocity() { return _lowestVelocity; }

    void setHighestVelocity(Float32 highestVelocity) { _highestVelocity = highestVelocity; }
    Float32 highestVelocity() { return _highestVelocity; }

    // Balance range

    void setLowestBalance(Float32 lowestBalance) { _lowestBalance = lowestBalance; }
    Float32 lowestBalance() { return _lowestBalance; }

    void setHighestBalance(Float32 highestBalance) { _highestBalance = highestBalance; }
    Float32 highestBalance() { return _highestBalance; }

    // ASDR volume envelope

    void setAttackRate(Float32 attackRate) { _attackRate = attackRate; }
    Float32 attackRate() { return _attackRate; }

    void setHoldTime(Float32 holdTime) { _holdTime = holdTime; }
    Float32 holdTime() { return _holdTime; }

    void setDecayRate(Float32 decayRate) { _decayRate = decayRate; }
    Float32 decayRate() { return _decayRate; }

    void setSustainLevel(Float32 sustainLevel) { _sustainLevel = sustainLevel; }
    Float32 sustainLevel() { return _sustainLevel; }

    void setReleaseRate(Float32 releaseRate) { _releaseRate = releaseRate; }
    Float32 releaseRate() { return _releaseRate; }

    // Filter

    void setFilterFc(Float32 filterFc) { _filterFc = filterFc; }
    Float32 filterFc() { return _filterFc; }

    void setFilterQ(Float32 filterQ) { _filterQ = filterQ; }
    Float32 filterQ() { return _filterQ; }

    // SF2

    void setSF2SampleRate(Float64 SF2SampleRate) { _SF2SampleRate = SF2SampleRate; }
    Float64 SF2SampleRate() { return _SF2SampleRate; }

    void setSF2SampleStart(SInt64 SF2SampleStart) { _SF2SampleStart = SF2SampleStart; }
    SInt64 SF2SampleStart() { return _SF2SampleStart; }

    void setSF2SampleEnd(SInt64 SF2SampleEnd) { _SF2SampleEnd = SF2SampleEnd; }
    SInt64 SF2SampleEnd() { return _SF2SampleEnd; }

    void setSF2SampleBasePos(SInt64 SF2SampleBasePos) { _SF2SampleBasePos = SF2SampleBasePos; }
    SInt64 SF2SampleBasePos() { return _SF2SampleBasePos; }
};

}  // namespace MDStudio

#endif  // INSTRUMENT_H
