//
//  audioplayerunit.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-11-27.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#include "audioplayerunit.h"

extern "C" {
#include <libaiff/libaiff.h>
}

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
AudioPlayerUnit::AudioPlayerUnit() { _samples = nullptr; }

// ---------------------------------------------------------------------------------------------------------------------
AudioPlayerUnit::~AudioPlayerUnit() {
    if (_samples) delete[] _samples;
}

// ---------------------------------------------------------------------------------------------------------------------
bool AudioPlayerUnit::playAudioFile(const std::string& path) {
    AIFF_Ref aiffRef = AIFF_OpenFile(path.c_str(), F_RDONLY);
    if (aiffRef == NULL) {
        return false;
    }

    uint64_t nbSamples;
    int channels;
    double samplingRate;
    int bitsPerSample;
    int segmentSize;

    if (AIFF_GetAudioFormat(aiffRef, &nbSamples, &channels, &samplingRate, &bitsPerSample, &segmentSize) < 1) {
        AIFF_CloseFile(aiffRef);
        return false;
    }

    // We support only one channel for now
    if (channels > 1) {
        AIFF_CloseFile(aiffRef);
        return false;
    }

    _nbSamples = static_cast<size_t>(nbSamples);

    std::lock_guard<std::mutex> lock(_renderMutex);

    if (_samples) {
        delete[] _samples;
        _samples = nullptr;
    }

    _samples = new Float32[_nbSamples];

    if (AIFF_ReadSamplesFloat(aiffRef, _samples, static_cast<int>(_nbSamples)) < 0) {
        delete[] _samples;
        _samples = nullptr;
        AIFF_CloseFile(aiffRef);
        return false;
    }

    AIFF_CloseFile(aiffRef);

    _samplePos = 0;

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
int AudioPlayerUnit::renderInput(UInt32 inNumberFrames, GraphSampleType* ioData[2], UInt32 stride) {
    std::lock_guard<std::mutex> lock(_renderMutex);

    GraphSampleType* outA = (GraphSampleType*)ioData[0];
    GraphSampleType* outB = (GraphSampleType*)ioData[1];

    for (UInt32 i = 0; i < inNumberFrames; ++i) {
        if (_samples && (_samplePos < _nbSamples)) {
            GraphSampleType s;

            s = _samples[_samplePos] * _level;

            *outA += s;
            *outB += s;
            ++_samplePos;
        }
        outA += stride;
        outB += stride;
    }

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
void AudioPlayerUnit::setLevel(Float32 level) {
    std::lock_guard<std::mutex> lock(_renderMutex);
    _level = level;
}

// ---------------------------------------------------------------------------------------------------------------------
Float32 AudioPlayerUnit::level() { return _level; }
