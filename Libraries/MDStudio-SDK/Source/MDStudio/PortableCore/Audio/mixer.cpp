//
//  mixer.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-21.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#include "mixer.h"

#ifdef WIN32
#define NOMINMAX
#include <Windows.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <memory>

#define SAMPLE_RATE (44100)

#define INPUT_NUM_CHANNELS 1

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
// This routine will be called by the PortAudio engine when audio is needed.
// It may called at interrupt level on some machines so don't do anything
// that could mess up the system like calling malloc() or free().
static int renderCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
    Mixer* mixer = (Mixer*)userData;

    //
    // Output
    //

    float* ob = (float*)outputBuffer;
    float* out[2];
    out[0] = ob;
    out[1] = ob + 1;
    (void)inputBuffer; /* Prevent unused argument warning. */

    mixer->renderInput(static_cast<UInt32>(framesPerBuffer), out, 2);

    //
    // Input
    //

    AudioBuffer* data = mixer->inputAudioBuffer();
    const float* rptr = (const float*)inputBuffer;
    float* wptr = &data->samples[data->frameIndex * INPUT_NUM_CHANNELS];
    long i;

    (void)outputBuffer; /* Prevent unused variable warnings. */
    (void)timeInfo;
    (void)statusFlags;
    (void)userData;

    data->mutex.lock();

    if (inputBuffer != NULL) {
        for (i = 0; i < MIXER_FRAMES_PER_BUFFER; i++) {
            *wptr++ = *rptr++;                              /* left */
            if (INPUT_NUM_CHANNELS == 2) *wptr++ = *rptr++; /* right */
        }
    }

    data->frameIndex = 0;

    data->mutex.unlock();

    return paContinue;
}

// ---------------------------------------------------------------------------------------------------------------------
static void PrintSupportedStandardSampleRates(const PaStreamParameters* inputParameters,
                                              const PaStreamParameters* outputParameters) {
    static double standardSampleRates[] = {
        8000.0,  9600.0,  11025.0, 12000.0, 16000.0, 22050.0,  24000.0,
        32000.0, 44100.0, 48000.0, 88200.0, 96000.0, 192000.0, -1 /* negative terminated  list */
    };
    int i, printCount;
    PaError err;

    printCount = 0;
    for (i = 0; standardSampleRates[i] > 0; i++) {
        err = Pa_IsFormatSupported(inputParameters, outputParameters, standardSampleRates[i]);
        if (err == paFormatIsSupported) {
            if (printCount == 0) {
                printf("\t%8.2f", standardSampleRates[i]);
                printCount = 1;
            } else if (printCount == 4) {
                printf(",\n\t%8.2f", standardSampleRates[i]);
                printCount = 1;
            } else {
                printf(", %8.2f", standardSampleRates[i]);
                ++printCount;
            }
        }
    }
    if (!printCount)
        printf("None\n");
    else
        printf("\n");
}

// ---------------------------------------------------------------------------------------------------------------------
Mixer::Mixer() {
    _stream = nullptr;
    _isRunning = false;
    _level = 0.5f;  // -3 dB

    Pa_Initialize();

    PaDeviceIndex outputDevice;
    outputDevice = Pa_GetDefaultOutputDevice();
    _outputDeviceName = Pa_GetDeviceInfo(outputDevice)->name;
    _outputLatency = Pa_GetDeviceInfo(outputDevice)->defaultLowOutputLatency;

    _inputAudioBuffer.samples = NULL;
}

// ---------------------------------------------------------------------------------------------------------------------
Mixer::~Mixer() {
    if (_isRunning) stop();

    Pa_Terminate();

    if (_inputAudioBuffer.samples) free(_inputAudioBuffer.samples);
}

// ---------------------------------------------------------------------------------------------------------------------
void Mixer::printDevices() {
    int i, numDevices, defaultDisplayed;
    const PaDeviceInfo* deviceInfo;
    PaStreamParameters inputParameters, outputParameters;

    printf("PortAudio version number = %d\nPortAudio version text = '%s'\n", Pa_GetVersion(), Pa_GetVersionText());

    numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        printf("ERROR: Pa_GetDeviceCount returned 0x%x\n", numDevices);
        return;
    }

    printf("Number of devices = %d\n", numDevices);
    for (i = 0; i < numDevices; i++) {
        deviceInfo = Pa_GetDeviceInfo(i);
        printf("--------------------------------------- device #%d\n", i);

        /* Mark global and API specific default devices */
        defaultDisplayed = 0;
        if (i == Pa_GetDefaultInputDevice()) {
            printf("[ Default Input");
            defaultDisplayed = 1;
        } else if (i == Pa_GetHostApiInfo(deviceInfo->hostApi)->defaultInputDevice) {
            const PaHostApiInfo* hostInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);
            printf("[ Default %s Input", hostInfo->name);
            defaultDisplayed = 1;
        }

        if (i == Pa_GetDefaultOutputDevice()) {
            printf((defaultDisplayed ? "," : "["));
            printf(" Default Output");
            defaultDisplayed = 1;
        } else if (i == Pa_GetHostApiInfo(deviceInfo->hostApi)->defaultOutputDevice) {
            const PaHostApiInfo* hostInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);
            printf((defaultDisplayed ? "," : "["));
            printf(" Default %s Output", hostInfo->name);
            defaultDisplayed = 1;
        }

        if (defaultDisplayed) printf(" ]\n");

            /* print device info fields */
#ifdef WIN32
        { /* Use wide char on windows, so we can show UTF-8 encoded device names */
            wchar_t wideName[MAX_PATH];
            MultiByteToWideChar(CP_UTF8, 0, deviceInfo->name, -1, wideName, MAX_PATH - 1);
            wprintf(L"Name                        = %s\n", wideName);
        }
#else
        printf("Name                        = %s\n", deviceInfo->name);
#endif
        printf("Host API                    = %s\n", Pa_GetHostApiInfo(deviceInfo->hostApi)->name);
        printf("Max inputs = %d", deviceInfo->maxInputChannels);
        printf(", Max outputs = %d\n", deviceInfo->maxOutputChannels);

        printf("Default low input latency   = %8.4f\n", deviceInfo->defaultLowInputLatency);
        printf("Default low output latency  = %8.4f\n", deviceInfo->defaultLowOutputLatency);
        printf("Default high input latency  = %8.4f\n", deviceInfo->defaultHighInputLatency);
        printf("Default high output latency = %8.4f\n", deviceInfo->defaultHighOutputLatency);

#ifdef WIN32
#if PA_USE_ASIO
        /* ASIO specific latency information */
        if (Pa_GetHostApiInfo(deviceInfo->hostApi)->type == paASIO) {
            long minLatency, maxLatency, preferredLatency, granularity;

            err = PaAsio_GetAvailableLatencyValues(i, &minLatency, &maxLatency, &preferredLatency, &granularity);

            printf("ASIO minimum buffer size    = %ld\n", minLatency);
            printf("ASIO maximum buffer size    = %ld\n", maxLatency);
            printf("ASIO preferred buffer size  = %ld\n", preferredLatency);

            if (granularity == -1)
                printf("ASIO buffer granularity     = power of 2\n");
            else
                printf("ASIO buffer granularity     = %ld\n", granularity);
        }
#endif /* PA_USE_ASIO */
#endif /* WIN32 */

        printf("Default sample rate         = %8.2f\n", deviceInfo->defaultSampleRate);

        /* poll for standard sample rates */
        inputParameters.device = i;
        inputParameters.channelCount = deviceInfo->maxInputChannels;
        inputParameters.sampleFormat = paInt16;
        inputParameters.suggestedLatency = 0; /* ignored by Pa_IsFormatSupported() */
        inputParameters.hostApiSpecificStreamInfo = NULL;

        outputParameters.device = i;
        outputParameters.channelCount = deviceInfo->maxOutputChannels;
        outputParameters.sampleFormat = paInt16;
        outputParameters.suggestedLatency = 0; /* ignored by Pa_IsFormatSupported() */
        outputParameters.hostApiSpecificStreamInfo = NULL;

        if (inputParameters.channelCount > 0) {
            printf("Supported standard sample rates\n for half-duplex 16 bit %d channel input = \n",
                   inputParameters.channelCount);
            PrintSupportedStandardSampleRates(&inputParameters, NULL);
        }

        if (outputParameters.channelCount > 0) {
            printf("Supported standard sample rates\n for half-duplex 16 bit %d channel output = \n",
                   outputParameters.channelCount);
            PrintSupportedStandardSampleRates(NULL, &outputParameters);
        }

        if (inputParameters.channelCount > 0 && outputParameters.channelCount > 0) {
            printf("Supported standard sample rates\n for full-duplex 16 bit %d channel input, %d channel output = \n",
                   inputParameters.channelCount, outputParameters.channelCount);
            PrintSupportedStandardSampleRates(&inputParameters, &outputParameters);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<std::pair<std::string, double>> Mixer::outputDevices() {
    std::vector<std::pair<std::string, double>> outputDevices;

    int i, numDevices;
    const PaDeviceInfo* deviceInfo;

    numDevices = Pa_GetDeviceCount();
    if (numDevices > 0) {
        for (i = 0; i < numDevices; ++i) {
            deviceInfo = Pa_GetDeviceInfo(i);
            // If the device has at least one output, we consider it as an output device
            if (deviceInfo->maxOutputChannels > 0)
                outputDevices.push_back(std::make_pair(std::string(deviceInfo->name), deviceInfo->defaultLowOutputLatency));
        }
    }

    return outputDevices;
}

// ---------------------------------------------------------------------------------------------------------------------
bool Mixer::start(bool isInputEnabled) {
    // If already running, we return right away
    if (_isRunning) return true;

    PaStreamParameters outputParameters;
    PaStreamParameters inputParameters;
    PaError err;
    PaDeviceIndex outputDevice;

    outputDevice = Pa_GetDefaultOutputDevice();

    if (_outputDeviceName != "") {
        int i, numDevices;
        const PaDeviceInfo* deviceInfo;

        numDevices = Pa_GetDeviceCount();
        if (numDevices > 0) {
            for (i = 0; i < numDevices; ++i) {
                deviceInfo = Pa_GetDeviceInfo(i);
                if (deviceInfo->name == _outputDeviceName) {
                    outputDevice = i;
                    break;
                }
            }
        }
    }

    outputParameters.device = outputDevice;
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr, "Error: No default output device.\n");
        goto error;
    }
    outputParameters.channelCount = 2;         /* stereo output */
    outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
    outputParameters.suggestedLatency =
        _outputLatency < 0.0f ? Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency : _outputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    // input

    if (isInputEnabled) {
        inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
        if (inputParameters.device == paNoDevice) {
            fprintf(stderr, "Error: No default input device.\n");
            goto error;
        }

        inputParameters.channelCount = 1; /* mono input */
        inputParameters.sampleFormat = paFloat32;
        inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
        inputParameters.hostApiSpecificStreamInfo = NULL;

        int totalFrames;
        int numSamples;
        int numBytes;

        _inputAudioBuffer.maxFrameIndex = totalFrames = MIXER_FRAMES_PER_BUFFER; /* Record for a few seconds. */
        _inputAudioBuffer.frameIndex = 0;
        numSamples = totalFrames * INPUT_NUM_CHANNELS;
        numBytes = numSamples * sizeof(float);
        _inputAudioBuffer.samples = (float*)malloc(numBytes); /* From now on, recordedSamples is initialised. */
        if (_inputAudioBuffer.samples == NULL) {
            printf("Could not allocate record array.\n");
            goto error;
        }
        for (int i = 0; i < numSamples; ++i) _inputAudioBuffer.samples[i] = 0;
    }  // if input is enabled

    err = Pa_OpenStream(&_stream, isInputEnabled ? &inputParameters : NULL, &outputParameters, /* As above. */
                        SAMPLE_RATE, MIXER_FRAMES_PER_BUFFER,                                  /* Frames per buffer. */
                        paClipOff, /* No out of range samples expected. */
                        renderCallback, this);
    if (err != paNoError) goto error;

    err = Pa_StartStream(_stream);
    if (err != paNoError) goto error;

    _isRunning = true;

    for (auto unit : _units) unit->setIsRunning(true);

    return true;
error:
    Pa_Terminate();
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
void Mixer::stop() {
    if (_stream != nullptr) {
        Pa_CloseStream(_stream);

        for (auto unit : _units) unit->setIsRunning(false);
    }

    _isRunning = false;
}

// ---------------------------------------------------------------------------------------------------------------------
void Mixer::addUnit(std::shared_ptr<Unit> unit) { _units.push_back(unit); }

// ---------------------------------------------------------------------------------------------------------------------
int Mixer::renderInput(UInt32 inNumberFrames, GraphSampleType* ioData[2], UInt32 stride) {
    // For each sample
    float* p = ioData[0];
    for (UInt32 i = 0; i < inNumberFrames; ++i) {
        *p = 0.0f;
        p += stride;
    }
    p = ioData[1];
    for (UInt32 i = 0; i < inNumberFrames; ++i) {
        *p = 0.0f;
        p += stride;
    }

    std::vector<Unit*>::iterator it;

    // Render the units
    for (auto unit : _units) unit->renderInput(inNumberFrames, ioData, stride);

    float gain = level();

    // For each sample
    p = ioData[0];
    for (UInt32 i = 0; i < inNumberFrames; ++i) {
        // We apply the gain
        *p *= gain;
        p += stride;
    }
    p = ioData[1];
    for (UInt32 i = 0; i < inNumberFrames; ++i) {
        // We apply the gain
        *p *= gain;
        p += stride;
    }

    return 0;
}
