//
//  comb.h
//  MDStudio
//
//  Created by Daniel Cliche on 11-05-24.
//  Copyright 2010-2020 Daniel Cliche. All rights reserved.
//

// Based on public domain code written by Jezar at Dreampoint, June 2000, http://www.dreampoint.co.uk

#ifndef COMB_H
#define COMB_H

#include "../types.h"
#include "denormals.h"

namespace MDStudio {

class Comb {
    float _feedback;
    GraphSampleType _filterStore;
    float _damp1;
    float _damp2;
    GraphSampleType* _buffer;
    int _bufSize;
    int _bufIdx;

   public:
    Comb();
    void setBuffer(GraphSampleType* buf, int size);
    inline GraphSampleType process(GraphSampleType inp);
    void mute();
    void setDamp(float val);
    float damp();
    void setFeedback(float val);
    float feedback();
};

// Big to inline - but crucial for speed

inline GraphSampleType Comb::process(GraphSampleType input) {
    GraphSampleType output;

    output = _buffer[_bufIdx];
    undenormalise(output);

    _filterStore = (output * _damp2) + (_filterStore * _damp1);
    undenormalise(_filterStore);

    _buffer[_bufIdx] = input + (_filterStore * _feedback);

    if (++_bufIdx >= _bufSize) _bufIdx = 0;

    return output;
}

}  // namespace MDStudio

#endif  //_comb_
