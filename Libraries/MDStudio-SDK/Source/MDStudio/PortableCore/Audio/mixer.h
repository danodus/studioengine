//
//  mixer.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-21.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef MIXER_H
#define MIXER_H

#include "unit.h"
#if !TARGET_OS_IPHONE
#include <portaudio.h>
#endif
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#define MIXER_FRAMES_PER_BUFFER 256

namespace MDStudio {

struct AudioBuffer {
    std::mutex mutex;
    int frameIndex; /* Index into sample array. */
    int maxFrameIndex;
    Float32* samples;
};

class Mixer {
#if !TARGET_OS_IPHONE
    PaStream* _stream;
#endif
    std::vector<std::shared_ptr<Unit>> _units;

    bool _isRunning;

    std::string _outputDeviceName;
    double _outputLatency;

    std::atomic<Float32> _level;
    std::atomic<bool> _isAGCEnabled;

    AudioBuffer _inputAudioBuffer;

   public:
    Mixer();
    ~Mixer();

    void printDevices();

    std::vector<std::shared_ptr<Unit>>::iterator unitsBegin() { return _units.begin(); }
    std::vector<std::shared_ptr<Unit>>::iterator unitsEnd() { return _units.end(); }

    std::vector<std::pair<std::string, double>> outputDevices();

    double outputLatency() { return _outputLatency; }
    void setOutputLatency(double outputLatency) { _outputLatency = outputLatency; }

    std::string outputDeviceName() { return _outputDeviceName; }
    void setOutputDeviceName(const std::string& outputDeviceName) { _outputDeviceName = outputDeviceName; }

    bool start(bool isInputEnabled = false);
    void stop();
    void addUnit(std::shared_ptr<Unit> unit);

    bool isRunning() { return _isRunning; }

    void setLevel(Float32 level) { _level = level; }
    Float32 level() { return _level; }

#if TARGET_OS_IPHONE
    void setIsAGCEnabled(bool isAGCEnabled) { _isAGCEnabled = isAGCEnabled; }
    bool isAGCEnabled() { return _isAGCEnabled; }
#endif

    int renderInput(UInt32 inNumberFrames, GraphSampleType* ioData[2], UInt32 stride = 1);

    AudioBuffer* inputAudioBuffer() { return &_inputAudioBuffer; }
};

}  // namespace MDStudio

#endif  // MIXER_H
