//
//  fmunit.h
//  MDStudio
//
//  Created by Daniel Cliche on 2018-07-12.
//  Copyright © 2018-2020 Daniel Cliche. All rights reserved.
//

#ifndef FMUNIT_H
#define FMUNIT_H

#include <unit.h>

#include <atomic>

namespace MDStudio {

class FMUnit : public Unit {
    std::atomic<float> _phase;
    std::atomic<float> _phaseDelta;
    std::atomic<float> _velocity;

    float _pitch;

   public:
    FMUnit();

    int renderInput(UInt32 inNumberFrames, GraphSampleType* ioData[2], UInt32 stride = 1) override;

    void noteOn(float pitch, float velocity);
    void noteOff();

    bool isNoteOn() { return _phaseDelta > 0; }

    float pitch() { return _pitch; }
};

}  // namespace MDStudio

#endif  // FMUNIT_H
