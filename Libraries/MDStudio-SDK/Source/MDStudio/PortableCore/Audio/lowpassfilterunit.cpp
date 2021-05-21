//
//  lowpassfilterunit.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2016-07-23.
//  Copyright (c) 2016-2020 Daniel Cliche. All rights reserved.
//

#include "lowpassfilterunit.h"

#include "mixer.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
LowPassFilterUnit::LowPassFilterUnit() {
    _isBypassed = false;
    _cutoffFreq = 4000.0f;
    _filter = new Dsp::SmoothedFilterDesign<Dsp::RBJ::Design::LowPass, 2>(1024);
}

// ---------------------------------------------------------------------------------------------------------------------
LowPassFilterUnit::~LowPassFilterUnit() { delete _filter; }

// ---------------------------------------------------------------------------------------------------------------------
int LowPassFilterUnit::renderInput(UInt32 inNumberFrames, GraphSampleType* ioData[2], UInt32 stride) {
    if (_isBypassed) return 0;

    float a[MIXER_FRAMES_PER_BUFFER];
    float b[MIXER_FRAMES_PER_BUFFER];

    GraphSampleType* outA = (GraphSampleType*)ioData[0];
    GraphSampleType* outB = (GraphSampleType*)ioData[1];

    for (UInt32 i = 0; i < inNumberFrames; ++i) {
        a[i] = *outA;
        b[i] = *outB;

        outA += stride;
        outB += stride;
    }

    Dsp::Params params;
    params[0] = 44100;  // sample rate
    params[1] = _cutoffFreq;
    params[2] = 1.25;  // Q
    _filter->setParams(params);

    float* audio[2] = {a, b};
    _filter->process(inNumberFrames, audio);

    outA = (GraphSampleType*)ioData[0];
    outB = (GraphSampleType*)ioData[1];

    for (UInt32 i = 0; i < inNumberFrames; ++i) {
        *outA = a[i];
        *outB = b[i];

        outA += stride;
        outB += stride;
    }

    return 0;
}
