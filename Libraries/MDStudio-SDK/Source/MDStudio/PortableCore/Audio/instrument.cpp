//
//  instrument.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-21.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#include "instrument.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
Instrument::Instrument() {
    _basePitch = 69.0f;  // A3 (440 Hz)
    _baseVelocity = 0.8f;
    _lowestPitch = 0.0f;
    _highestPitch = 128.0f;
    _lowestVelocity = 0.0f;
    _highestVelocity = 127.0f;
    _lowestBalance = 0.0f;
    _highestBalance = 0.0f;
    _loopStart = _loopEnd = 0.0f;
    _attackRate = 1.0f;
    _holdTime = 0.0f;
    _decayRate = 0.0f;
    _sustainLevel = 1.0f;
    _releaseRate = 1.0f;
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<Sample> Instrument::loadSF2Sample(const std::string& path) {
    // Create a new sample
    _sample = std::shared_ptr<Sample>(new Sample());

    // Load the audio
    _sample->setSF2SampleRate(_SF2SampleRate);
    _sample->setSF2SampleStart(_SF2SampleStart);
    _sample->setSF2SampleEnd(_SF2SampleEnd);
    _sample->setSF2SampleBasePos(_SF2SampleBasePos);

    _sample->loadSF2Audio(path);

    return _sample;
}
