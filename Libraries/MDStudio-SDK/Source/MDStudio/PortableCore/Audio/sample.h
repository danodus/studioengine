//
//  sample.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-21.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef SAMPLE_H
#define SAMPLE_H

#include <string>

#include "../types.h"

namespace MDStudio {

class Sample {
    SampleType* _data;
    Float64 _sampleRate;

    // Audio data
    std::string* _audioFileName;  // name of the audio file
    Float32 _length;              // length of the buffer in frames
    SampleType* _dataWithMargin;  // Internal use only

    Float64 _SF2SampleRate;
    SInt64 _SF2SampleStart;
    SInt64 _SF2SampleEnd;
    SInt64 _SF2SampleBasePos;

   public:
    Sample();
    ~Sample();

    void setSF2SampleRate(Float64 sampleRate) { _SF2SampleRate = sampleRate; }
    Float64 SF2SampleRate() { return _SF2SampleRate; }

    void setSF2SampleStart(SInt64 sampleStart) { _SF2SampleStart = sampleStart; }
    SInt64 SF2SampleStart() { return _SF2SampleStart; }

    void setSF2SampleEnd(SInt64 sampleEnd) { _SF2SampleEnd = sampleEnd; }
    SInt64 SF2SampleEnd() { return _SF2SampleEnd; }

    void setSF2SampleBasePos(SInt64 sampleBasePos) { _SF2SampleBasePos = sampleBasePos; }
    SInt64 SF2SampleBasePos() { return _SF2SampleBasePos; }

    bool loadSF2Audio(const std::string& path);

    SampleType* data() { return _data; }
    Float64 sampleRate() { return _sampleRate; }
    Float32 length() { return _length; }
};

}  // namespace MDStudio

#endif  // SAMPLE_H
