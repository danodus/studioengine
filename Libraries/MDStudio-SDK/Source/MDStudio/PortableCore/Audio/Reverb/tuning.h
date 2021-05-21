//
//  tuning.h
//  MDStudio
//
//  Created by Daniel Cliche on 11-05-24.
//  Copyright 2010-2020 Daniel Cliche. All rights reserved.
//

// Reverb model tuning values
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#ifndef TUNING_H
#define TUNING_H

namespace MDStudio {

const int numCombs = 8;
const int numAllPasses = 4;
const float muted = 0.0f;
const float fixedGain = 0.015f;
const float scaleWet = 3.0f;
const float scaleDry = 2.0f;
const float scaleDamp = 0.4f;
const float scaleRoom = 0.28f;
const float offsetRoom = 0.7f;
const float initialRoom = 0.9f;
const float initialDamp = 0.5f;
const float initialWet = 1.0f;
const float initialDry = 0.0f;
const float initialWidth = 1.0f;
const float initialMode = 0.0f;
const float freezeMode = 1.0f;

// These values assume 44.1KHz sample rate
// they will probably be OK for 48KHz sample rate
// but would need scaling for 96KHz (or other) sample rates.
// The values were obtained by listening tests.
const int combTuning1 = 1116;
const int combTuning2 = 1188;
const int combTuning3 = 1277;
const int combTuning4 = 1356;
const int combTuning5 = 1422;
const int combTuning6 = 1491;
const int combTuning7 = 1557;
const int combTuning8 = 1617;
const int allPassTuning1 = 556;
const int allPassTuning2 = 441;
const int allPassTuning3 = 341;
const int allPassTuning4 = 225;

}  // namespace MDStudio

#endif  // TUNING_H
