//
//  chorusmodel.h
//  MDStudio
//
//  Created by Daniel Cliche on 2017-05-08.
//  Copyright © 2017-2020 Daniel Cliche. All rights reserved.
//

#ifndef CHORUSMODEL_H
#define CHORUSMODEL_H

#include "../types.h"

namespace MDStudio {

enum { kMixMono, kMixWetOnly, kMixWetLeft, kMixWetRight, kMixWetLeft75, kMixWetRight75, kMixStereoWetOnly };

class ChorusModel {
    float _paramSweepRate;   // 0.0-1.0 passed in
    float _paramWidth;       // ditto
    float _paramDelay;       // ditto
    float _paramMixMode;     // ditto
    float _sweepRate;        // actual calc'd sweep rate
    int _sweepSamples;       // sweep width in # of samples
    int _delaySamples;       // number of samples to run behind filling pointer
    float _minSweepSamples;  // lower bound of calculated sweep range, calc'd by setSweep from rate, width, delay
    float _maxSweepSamples;  // upper bound, ditto
    int _mixMode;            // mapped to supported mix modes
    GraphSampleType* _buf;   // stored sound
    int _fp;                 // fill/write pointer
    float _sweep;

    float _lfoPhase;
    float _lfoDeltaPhase;

    // output mixing
    float _mixLeftWet;
    float _mixLeftDry;
    float _mixRightWet;
    float _mixRightDry;
    float _wet;

    void setSweep(void);

   public:
    ChorusModel();
    ~ChorusModel();

    int renderInput(UInt32 inNumberFrames, GraphSampleType* ioData[2], UInt32 stride = 1);

    void setRate(float v);
    void setWidth(float v);
    void setDelay(float v);
    void setMixMode(int mixMode);
    void setWet(float v);

    float rate() { return _paramSweepRate; }
    float width() { return _paramWidth; }
    float delay() { return _paramDelay; }
    int mixMode() { return _paramMixMode; }
    float wet() { return _wet; }
};

}  // namespace MDStudio

#endif  // CHORUSMODEL_H
