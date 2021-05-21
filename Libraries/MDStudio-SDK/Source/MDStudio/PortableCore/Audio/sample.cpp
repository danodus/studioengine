//
//  sample.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-21.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#include "sample.h"

#include <assert.h>

#include "stdio.h"
#include "stdlib.h"

#define SAMPLE_DATA_MARGIN_LENGTH 32  // Number of zeros before and after the data buffer for convolution
#define MD_SF2_SAMPLE_GAIN 0.95f      // Gain on the samples

using namespace MDStudio;

static const Float64 kDefaultSampleRate = 44100.0;

// ---------------------------------------------------------------------------------------------------------------------
Sample::Sample() {
    _data = _dataWithMargin = NULL;
    _sampleRate = kDefaultSampleRate;
}

// ---------------------------------------------------------------------------------------------------------------------
bool Sample::loadSF2Audio(const std::string& path) {
    _sampleRate = _SF2SampleRate;

    // Calculate the number of frames
    UInt32 numFrames = (UInt32)(_SF2SampleEnd - _SF2SampleStart);
    _length = numFrames;

    // Open the SF2 bank
    FILE* f = fopen(path.c_str(), "rb");
    if (f == NULL) {
        // Unable to open the audio file
        return false;
    }

    // Allocate the input data buffer
    SInt16* inputData = (SInt16*)calloc(numFrames, sizeof(SInt16));
    if (inputData == NULL) {
        fclose(f);
        return false;
    }

    // Read the data
    fseek(f, _SF2SampleBasePos + _SF2SampleStart * sizeof(SInt16), SEEK_SET);
    fread(inputData, numFrames, sizeof(SInt16), f);

    // Close the SF2 bank
    fclose(f);

    // Calculate the number of samples at the output
    UInt32 numOutputSamples = numFrames;

    // Allocate the output data buffer
    _dataWithMargin = (SampleType*)calloc(numOutputSamples + 2 * SAMPLE_DATA_MARGIN_LENGTH, sizeof(SampleType));
    if (_dataWithMargin == NULL) {
        free(inputData);
        return false;
    }

    assert(_dataWithMargin);

    _data = _dataWithMargin + SAMPLE_DATA_MARGIN_LENGTH;

    // Clear the margins
    for (UInt32 i = 0; i < SAMPLE_DATA_MARGIN_LENGTH; i++) {
        _dataWithMargin[i] = 0;
        _dataWithMargin[numOutputSamples + 2 * SAMPLE_DATA_MARGIN_LENGTH - i - 1] = 0;
    }

    for (UInt32 i = 0; i < numFrames; i++) {
        _dataWithMargin[i + SAMPLE_DATA_MARGIN_LENGTH] = (SampleType)(inputData[i]) / 65536.0f * MD_SF2_SAMPLE_GAIN;
    }

    // Dispose the input buffer
    free(inputData);

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
Sample::~Sample() {
    if (_dataWithMargin) {
        free(_dataWithMargin);
        _dataWithMargin = _data = NULL;
    }
}
