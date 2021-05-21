//
//  lowpassfilterunit.h
//  MDStudio
//
//  Created by Daniel Cliche on 2016-07-23.
//  Copyright (c) 2016-2020 Daniel Cliche. All rights reserved.
//

#ifndef LOWPASSFILTERUNIT_H
#define LOWPASSFILTERUNIT_H

#include <DspFilters/RBJ.h>
#include <DspFilters/SmoothedFilter.h>

#include <atomic>

#include "unit.h"

namespace MDStudio {

class LowPassFilterUnit : public Unit {
    std::atomic<bool> _isBypassed;
    std::atomic<float> _cutoffFreq;
    Dsp::SmoothedFilterDesign<Dsp::RBJ::Design::LowPass, 2>* _filter;

   public:
    LowPassFilterUnit();
    ~LowPassFilterUnit();

    int renderInput(UInt32 inNumberFrames, GraphSampleType* ioData[2], UInt32 stride = 1);

    void setIsBypassed(bool isBypassed) { _isBypassed = isBypassed; }
    void setCutoffFreq(float cutoffFreq) { _cutoffFreq = cutoffFreq; }
};

}  // namespace MDStudio

#endif  // LOWPASSFILTERUNIT_H
