//
//  sineunit.h
//  MDStudio
//
//  Created by Daniel Cliche on 2016-08-12.
//  Copyright © 2016-2020 Daniel Cliche. All rights reserved.
//

#ifndef SINEUNIT_H
#define SINEUNIT_H

#include <unit.h>

#include <atomic>

namespace MDStudio {

class SineUnit : public Unit {
    std::atomic<float> _phase;
    std::atomic<float> _phaseDelta;
    std::atomic<float> _velocity;

    float _pitch;

   public:
    SineUnit();

    int renderInput(UInt32 inNumberFrames, GraphSampleType* ioData[2], UInt32 stride = 1) override;

    void noteOn(float pitch, float velocity);
    void noteOff();

    bool isNoteOn() { return _phaseDelta > 0; }

    float pitch() { return _pitch; }
};

}  // namespace MDStudio

#endif  // SINEUNIT_H
