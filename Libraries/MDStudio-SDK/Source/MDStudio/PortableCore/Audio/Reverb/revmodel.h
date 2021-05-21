//
//  revmodel.h
//  MDStudio
//
//  Created by Daniel Cliche on 11-05-24.
//  Copyright 2010-2020 Daniel Cliche. All rights reserved.
//

// Based on public domain code written by Jezar at Dreampoint, June 2000, http://www.dreampoint.co.uk

#ifndef REVMODEL_H
#define REVMODEL_H

#include "comb.h"
#include "tuning.h"

namespace MDStudio {

class RevModel {
    float _gain;
    float _roomSize, _roomSize1;
    float _damp, _damp1;
    float _wet, _wet1, _wet2;
    float _dry;
    float _width;
    float _mode;

    // The following are all declared inline
    // to remove the need for dynamic allocation
    // with its subsequent error-checking messiness

    // Comb filters
    Comb _combs[numCombs];

    // Buffers for the combs
    GraphSampleType _bufComb1[combTuning1];
    GraphSampleType _bufComb2[combTuning2];
    GraphSampleType _bufComb3[combTuning3];
    GraphSampleType _bufComb4[combTuning4];
    GraphSampleType _bufComb5[combTuning5];
    GraphSampleType _bufComb6[combTuning6];
    GraphSampleType _bufComb7[combTuning7];
    GraphSampleType _bufComb8[combTuning8];

   public:
    RevModel();
    void mute();
    void processMix(GraphSampleType* inputL, GraphSampleType* inputR, GraphSampleType* outputL,
                    GraphSampleType* outputR, unsigned long numSamples, unsigned long stride);
    void setRoomSize(float value);
    float roomSize();
    void setDamp(float value);
    float damp();
    void setWet(float value);
    float wet();
    void setDry(float value);
    float dry();
    void setWidth(float value);
    float width();
    void setMode(float value);
    float mode();
    void update();
};

}  // namespace MDStudio

#endif  // REVMODEL_H
