//
//  revmodel.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 11-05-24.
//  Copyright 2010-2020 Daniel Cliche. All rights reserved.
//

// Based on public domain code written by Jezar at Dreampoint, June 2000, http://www.dreampoint.co.uk

#include "revmodel.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
RevModel::RevModel() {
    // Tie the components to their buffers
    _combs[0].setBuffer(_bufComb1, combTuning1);
    _combs[1].setBuffer(_bufComb2, combTuning2);
    _combs[2].setBuffer(_bufComb3, combTuning3);
    _combs[3].setBuffer(_bufComb4, combTuning4);
    _combs[4].setBuffer(_bufComb5, combTuning5);
    _combs[5].setBuffer(_bufComb6, combTuning6);
    _combs[6].setBuffer(_bufComb7, combTuning7);
    _combs[7].setBuffer(_bufComb8, combTuning8);

    // Set default values
    setWet(initialWet);
    setRoomSize(initialRoom);
    setDry(initialDry);
    setDamp(initialDamp);
    setWidth(initialWidth);
    setMode(initialMode);

    // Buffer will be full of rubbish - so we MUST mute them
    mute();
}

// ---------------------------------------------------------------------------------------------------------------------
void RevModel::mute() {
    if (mode() >= freezeMode) return;

    int i;
    for (i = 0; i < numCombs; ++i) {
        _combs[i].mute();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void RevModel::processMix(GraphSampleType* inputL, GraphSampleType* inputR, GraphSampleType* outputL,
                          GraphSampleType* outputR, unsigned long numSamples, unsigned long stride) {
    GraphSampleType outL, outR, input;

    while (numSamples-- > 0) {
        outL = outR = 0;
        input = (*inputL + *inputR) * _gain;

        int i;

        // Accumulate comb filters in parallel
        for (i = 0; i < numCombs; i++) {
            GraphSampleType out = _combs[i].process(input);
            outL += out;
            outR += out;
        }

        // Calculate output MIXING with anything already there
        *outputL += outL * _wet1 + outR * _wet2 + *inputL * _dry;
        *outputR += outR * _wet1 + outL * _wet2 + *inputR * _dry;

        // Increment sample pointers
        inputL += stride;
        inputR += stride;
        outputL += stride;
        outputR += stride;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void RevModel::update() {
    // Recalculate internal values after parameter change

    int i;

    _wet1 = _wet * (_width / 2 + 0.5f);
    _wet2 = _wet * ((1 - _width) / 2);

    if (_mode >= freezeMode) {
        _roomSize1 = 1;
        _damp1 = 0;
        _gain = muted;
    } else {
        _roomSize1 = _roomSize;
        _damp1 = _damp;
        _gain = fixedGain;
    }

    for (i = 0; i < numCombs; ++i) {
        _combs[i].setFeedback(_roomSize1);
    }

    for (i = 0; i < numCombs; ++i) {
        _combs[i].setDamp(_damp1);
    }
}

// The following get/set functions are not inlined, because
// speed is never an issue when calling them, and also
// because as you develop the reverb model, you may
// wish to take dynamic action when they are called.

// ---------------------------------------------------------------------------------------------------------------------
void RevModel::setRoomSize(float value) { _roomSize = value; }

// ---------------------------------------------------------------------------------------------------------------------
float RevModel::roomSize() { return _roomSize; }

// ---------------------------------------------------------------------------------------------------------------------
void RevModel::setDamp(float value) { _damp = (value * 0.01f) * scaleDamp; }

// ---------------------------------------------------------------------------------------------------------------------
float RevModel::damp() { return (_damp * 100.0f) / scaleDamp; }

// ---------------------------------------------------------------------------------------------------------------------
void RevModel::setWet(float value) { _wet = value; }

// ---------------------------------------------------------------------------------------------------------------------
float RevModel::wet() { return _wet; }

// ---------------------------------------------------------------------------------------------------------------------
void RevModel::setDry(float value) { _dry = value; }

// ---------------------------------------------------------------------------------------------------------------------
float RevModel::dry() { return _dry; }

// ---------------------------------------------------------------------------------------------------------------------
void RevModel::setWidth(float value) { _width = (value * 0.01f); }

// ---------------------------------------------------------------------------------------------------------------------
float RevModel::width() { return (_width * 100.0f); }

// ---------------------------------------------------------------------------------------------------------------------
void RevModel::setMode(float value) { _mode = value; }

// ---------------------------------------------------------------------------------------------------------------------
float RevModel::mode() {
    if (_mode >= freezeMode)
        return 1;
    else
        return 0;
}
