//
//  comb.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 11-05-24.
//  Copyright 2010-2020 Daniel Cliche. All rights reserved.
//

// Based on public domain code written by Jezar at Dreampoint, June 2000, http://www.dreampoint.co.uk

#include "comb.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
Comb::Comb() {
    _filterStore = 0;
    _bufIdx = 0;
}

// ---------------------------------------------------------------------------------------------------------------------
void Comb::setBuffer(GraphSampleType* buf, int size) {
    _buffer = buf;
    _bufSize = size;
}

// ---------------------------------------------------------------------------------------------------------------------
void Comb::mute() {
    for (int i = 0; i < _bufSize; ++i) _buffer[i] = 0;
}

// ---------------------------------------------------------------------------------------------------------------------
void Comb::setDamp(float val) {
    _damp1 = val;
    _damp2 = 1 - val;
}

// ---------------------------------------------------------------------------------------------------------------------
float Comb::damp() { return _damp1; }

// ---------------------------------------------------------------------------------------------------------------------
void Comb::setFeedback(float val) { _feedback = val; }

// ---------------------------------------------------------------------------------------------------------------------
float Comb::feedback() { return _feedback; }
