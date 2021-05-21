//
//  chorusmodel.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2017-05-08.
//  Copyright © 2017-2020 Daniel Cliche. All rights reserved.
//

#include "chorusmodel.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>

#define GRAPH_SAMPLE_RATE 44100
#define ROUND(n) ((int)((float)(n) + 0.5))
#define BSZ 8192  // must be about 1/5 of a second at given sample rate
#define MODF(n, i, f) ((i) = (int)(n), (f) = (n) - (float)(i))

using namespace MDStudio;

#define NUM_MIX_MODES 7

// ---------------------------------------------------------------------------------------------------------------------
ChorusModel::ChorusModel() {
    _paramWidth = 0.8f;
    _paramDelay = 0.2f;
    _paramMixMode = 0;
    _sweepSamples = 0;
    _delaySamples = 22;
    _lfoPhase = 0.0f;

    setRate(0.2f);
    setWidth(0.5f);

    _fp = 0;
    _sweep = 0.0;

    setMixMode(kMixStereoWetOnly);

    _wet = 0.0f;

    // allocate the buffer
    _buf = new GraphSampleType[BSZ];
    memset(_buf, 0, sizeof(GraphSampleType) * BSZ);
}

// ---------------------------------------------------------------------------------------------------------------------
ChorusModel::~ChorusModel() {
    if (_buf) delete[] _buf;
}

// ---------------------------------------------------------------------------------------------------------------------
void ChorusModel::setRate(float rate) {
    _paramSweepRate = rate;
    _lfoDeltaPhase = 2 * M_PI * rate / GRAPH_SAMPLE_RATE;

    // map into param onto desired sweep range with log curve
    _sweepRate = pow(10.0, (double)_paramSweepRate);
    _sweepRate -= 1.0;
    _sweepRate *= 1.1f;
    _sweepRate += 0.1f;

    // finish setup
    setSweep();
}

// ---------------------------------------------------------------------------------------------------------------------
// Maps 0.0-1.0 input to calculated width in samples from 0ms to 50ms
void ChorusModel::setWidth(float v) {
    _paramWidth = v;

    // map so that we can spec between 0ms and 50ms
    _sweepSamples = ROUND(v * 0.05 * GRAPH_SAMPLE_RATE);

    // finish setup
    setSweep();
}

// ---------------------------------------------------------------------------------------------------------------------
// Expects input from 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0
void ChorusModel::setDelay(float v) {
    _paramDelay = v;

    // make onto desired values applying log curve
    double delay = pow(10.0, (double)v * 2.0) / 1000.0;  // map logarithmically and convert to seconds
    _delaySamples = ROUND(delay * GRAPH_SAMPLE_RATE);

    // finish setup
    setSweep();
}

// ---------------------------------------------------------------------------------------------------------------------
// Sets up sweep based on rate, depth, and delay as they're all interrelated
// Assumes _sweepRate, _sweepSamples, and _delaySamples have all been set by
// setRate, setWidth, and setDelay
void ChorusModel::setSweep() {
    // calc min and max sweep now
    _minSweepSamples = _delaySamples;
    _maxSweepSamples = _delaySamples + _sweepSamples;

    // set intial sweep pointer to midrange
    _sweep = (_minSweepSamples + _maxSweepSamples) / 2;
}

// ---------------------------------------------------------------------------------------------------------------------
void ChorusModel::setMixMode(int mixMode) {
    _paramMixMode = mixMode;
    switch (mixMode) {
        case kMixMono:
        default:
            _mixLeftWet = _mixRightWet = 1.0;
            _mixLeftDry = _mixRightDry = 1.0f;
            break;
        case kMixWetOnly:
            _mixLeftWet = _mixRightWet = 1.0f;
            _mixLeftDry = _mixRightDry = 0.0;
            break;
        case kMixWetLeft:
            _mixLeftWet = 1.0f;
            _mixLeftDry = 0.0f;
            _mixRightWet = 0.0f;
            _mixRightDry = 1.0f;
            break;
        case kMixWetRight:
            _mixLeftWet = 0.0f;
            _mixLeftDry = 1.0f;
            _mixRightWet = 1.0f;
            _mixRightDry = 0.0f;
            break;
        case kMixStereoWetOnly:
            _mixLeftWet = 1.0f;
            _mixLeftDry = 0.0f;
            _mixRightWet = -1.0f;
            _mixRightDry = 0.0f;
            break;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ChorusModel::setWet(float v) { _wet = v; }

// ---------------------------------------------------------------------------------------------------------------------
int ChorusModel::renderInput(UInt32 inNumberFrames, GraphSampleType* ioData[2], UInt32 stride) {
    float* io1 = ioData[0];
    float* io2 = ioData[1];

    for (int i = 0; i < inNumberFrames; ++i) {
        // assemble mono input value and store it in circle queue
        float inval = (*io1 + *io2) / 2.0f;
        _buf[_fp] = inval;
        _fp = (_fp + 1) & (BSZ - 1);

        // build the two emptying pointers and do linear interpolation
        int ep1, ep2;
        float w1, w2;
        float ep = _fp - _sweep;
        MODF(ep, ep1, w2);
        ep1 &= (BSZ - 1);
        ep2 = ep1 + 1;
        ep2 &= (BSZ - 1);
        w1 = 1.0 - w2;
        GraphSampleType outval = _buf[ep1] * w1 + _buf[ep2] * w2;

        // develop output mix
        *io1 = (float)(_mixLeftDry * *io1 + _mixLeftWet * _wet * outval);
        *io2 = (float)(_mixRightDry * *io2 + _mixRightWet * _wet * outval);

        _sweep = _minSweepSamples + _sweepSamples * (sinf(_lfoPhase) + 1.0f) / 2.0f;
        _lfoPhase += _lfoDeltaPhase;

        _lfoPhase = fmod(_lfoPhase, 2 * M_PI);

        io1 += stride;
        io2 += stride;
    }

    return 0;
}
