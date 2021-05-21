//
//  audioplayerunit.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-11-27.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef AUDIOPLAYERUNIT_H
#define AUDIOPLAYERUNIT_H

#include <mutex>
#include <string>

#include "unit.h"

namespace MDStudio {

class AudioPlayerUnit : public Unit {
    Float32* _samples;
    size_t _nbSamples;
    size_t _samplePos;
    std::mutex _renderMutex;

    Float32 _level;

   public:
    AudioPlayerUnit();
    ~AudioPlayerUnit();

    bool playAudioFile(const std::string& path);

    void setLevel(Float32 level);
    Float32 level();

    int renderInput(UInt32 inNumberFrames, GraphSampleType* ioData[2], UInt32 stride = 1);
};

}  // namespace MDStudio

#endif  // AUDIOPLAYERUNIT_H
