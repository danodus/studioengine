//
//  mixer_ios.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-21.
//  Copyright (c) 2015-2020 Daniel Cliche. All rights reserved.
//

#if TARGET_OS_IPHONE

#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <memory>

#include "mixer.h"

#define MIXER_NB_INPUTS 64
#define MIXER_MAX_CHANNELS MD_MIXER_NB_INPUTS

#define MIXER_GRAPH_SAMPLE_RATE 44100.0
#define MIXER_GRAPH_SAMPLE_PERIOD (1.0 / MD_MIXER_GRAPH_SAMPLE_RATE)

#define MIXER_MAX_LEVEL 1.0

#define MIXER_AGC_MAX_GAIN 4.0  // Maximum gain for the AGC
#define MIXER_AGC_MIN_GAIN 0.5  // Minimum gain for the AGC

using namespace MDStudio;

int currentInput;

AUGraph graph;
AudioUnit outputAU;

AUNode outputNode;

AURenderCallbackStruct renderCallbacks[MIXER_NB_INPUTS];
bool applyReverbStates[MIXER_NB_INPUTS];

// ---------------------------------------------------------------------------------------------------------------------
static void renderData(Mixer* mixer, UInt32 nbFrames, GraphSampleType* ioData[2], UInt32 stride,
                       bool bypassAGC = false) {
    GraphSampleType* outA = ioData[0];
    GraphSampleType* outB = ioData[1];

    memset(outA, 0, nbFrames * sizeof(GraphSampleType));
    memset(outB, 0, nbFrames * sizeof(GraphSampleType));

    // Render the units
    for (auto it = mixer->unitsBegin(); it != mixer->unitsEnd(); it++) {
        GraphSampleType* out[2];
        out[0] = outA;
        out[1] = outB;
        (*it)->renderInput(static_cast<UInt32>(nbFrames), out, stride);
    }

    //
    // Apply level and automatic gain control
    //

    static Float32 gain = 1.0f;
    GraphSampleType maxOutput = 0;

    Float32 level = gain * mixer->level();

    // For each sample
    for (UInt32 i = 0; i < nbFrames; i += stride) {
        // We apply the level
        outA[i] *= level;
        outB[i] *= level;

        GraphSampleType absOutA = fabs(outA[i]);
        GraphSampleType absOutB = fabs(outB[i]);

        // We get the maximum output value
        if (absOutA > maxOutput) maxOutput = absOutA;
        if (absOutB > maxOutput) maxOutput = absOutB;
    }

    if (!bypassAGC && mixer->isAGCEnabled()) {
        // We adjust the gain based on the maximum output value
        if (maxOutput < 1.0f) {
            gain += 0.01f;
        } else {
            gain -= 0.01f;
        }

        // We clamp the gain
        if (gain > MIXER_AGC_MAX_GAIN) {
            gain = MIXER_AGC_MAX_GAIN;
        } else if (gain < MIXER_AGC_MIN_GAIN) {
            gain = MIXER_AGC_MIN_GAIN;
        }
    } else {
        gain = 1.0f;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
static OSStatus renderInput(void* inRefCon, AudioUnitRenderActionFlags* ioActionFlags,
                            const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames,
                            AudioBufferList* ioData) {
    Mixer* mixer = static_cast<Mixer*>(inRefCon);

    GraphSampleType* buffers[2];
    buffers[0] = (GraphSampleType*)ioData->mBuffers[0].mData;
    buffers[1] = (GraphSampleType*)ioData->mBuffers[1].mData;
    renderData(mixer, inNumberFrames, buffers, 1);

    return noErr;
}

// ---------------------------------------------------------------------------------------------------------------------
static bool initialize(void* controller) {
    OSStatus result = noErr;
    currentInput = 0;

    // load audio

    // create a new AUGraph
    result = NewAUGraph(&graph);
    if (result) return false;

    // output unit
    AudioComponentDescription outputDesc;

    outputDesc.componentType = kAudioUnitType_Output;
#if TARGET_OS_IPHONE
    outputDesc.componentSubType = kAudioUnitSubType_RemoteIO;
#else
    outputDesc.componentSubType = kAudioUnitSubType_DefaultOutput;
#endif
    outputDesc.componentManufacturer = kAudioUnitManufacturer_Apple;
    outputDesc.componentFlags = 0;
    outputDesc.componentFlagsMask = 0;

    // Create output node in the graph
    result = AUGraphAddNode(graph, &outputDesc, &outputNode);
    if (result) return false;

    // Open the graph
    result = AUGraphOpen(graph);
    if (result) return false;

    AudioStreamBasicDescription desc = {0};

    desc.mSampleRate = MIXER_GRAPH_SAMPLE_RATE;
    desc.mFormatID = kAudioFormatLinearPCM;
    desc.mFormatFlags = kAudioFormatFlagsNativeFloatPacked | kAudioFormatFlagIsNonInterleaved;
    desc.mBytesPerPacket = sizeof(GraphSampleType);
    desc.mBytesPerFrame = sizeof(GraphSampleType);
    desc.mFramesPerPacket = 1;
    desc.mChannelsPerFrame = 2;
    desc.mBitsPerChannel = 8 * sizeof(GraphSampleType);

    // Get output node info
    result = AUGraphNodeInfo(graph, outputNode, NULL, &outputAU);
    if (result) return false;

    // Set format
    result =
        AudioUnitSetProperty(outputAU, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &desc, sizeof(desc));
    if (result) return false;

    // Set a callback for the specified node's specified input
    AURenderCallbackStruct rcbs;
    rcbs.inputProc = &renderInput;
    rcbs.inputProcRefCon = controller;

    result = AudioUnitSetProperty(outputAU, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Global, 0, &rcbs,
                                  sizeof(rcbs));
    if (result) return false;

    // We initialize the AUGraph
    result = AUGraphInitialize(graph);
    if (result) return false;

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
static bool addRenderCallback(AURenderCallbackStruct rcbs, bool applyReverb) {
    if (currentInput >= MIXER_NB_INPUTS) return false;

    // Set a callback for the specified node's specified input
    renderCallbacks[currentInput] = rcbs;

    // Set the reverb state
    applyReverbStates[currentInput] = applyReverb;

    currentInput++;

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
static bool isRunning() {
    Boolean isRunning;
    AUGraphIsRunning(graph, &isRunning);
    if (isRunning) return true;
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
static bool start(Mixer* mixer) {
    if (!isRunning()) {
        OSStatus result = AUGraphStart(graph);
        if (result) return false;
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
static bool stop(Mixer* mixer) {
    Boolean isRunning;
    AUGraphIsRunning(graph, &isRunning);
    if (isRunning) {
        OSStatus result = AUGraphStop(graph);
        if (result) return false;
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
static void dispose() {
    // Dispose AU graph
    DisposeAUGraph(graph);
}

// ---------------------------------------------------------------------------------------------------------------------
Mixer::Mixer() {
    _isRunning = false;

    _level = 0.5f;  // -3 dB
    _isAGCEnabled = false;

    ::initialize(this);
}

// ---------------------------------------------------------------------------------------------------------------------
Mixer::~Mixer() { ::dispose(); }

// ---------------------------------------------------------------------------------------------------------------------
void Mixer::printDevices() {
    // Not available on iOS
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<std::pair<std::string, double>> Mixer::outputDevices() {
    // Not available on iOS

    std::vector<std::pair<std::string, double>> outputDevices;
    return outputDevices;
}

// ---------------------------------------------------------------------------------------------------------------------
bool Mixer::start(bool isInputEnabled) {
    // If already running, we return right away
    if (_isRunning) return true;

    ::start(this);

    _isRunning = true;

    for (auto unit : _units) unit->setIsRunning(true);

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
void Mixer::stop() {
    ::stop(this);

    _isRunning = false;

    for (auto unit : _units) unit->setIsRunning(false);
}

// ---------------------------------------------------------------------------------------------------------------------
void Mixer::addUnit(std::shared_ptr<Unit> unit) { _units.push_back(unit); }

// ---------------------------------------------------------------------------------------------------------------------
int Mixer::renderInput(UInt32 inNumberFrames, GraphSampleType* ioData[2], UInt32 stride) {
    renderData(this, inNumberFrames, ioData, stride, true);
    return 0;
}

#endif  // TARGET_OS_IPHONE
