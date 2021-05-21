//
//  audioexport.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2015-08-29.
//  Copyright (c) 2015-2020 Daniel Cliche. All rights reserved.
//

#include "audioexport.h"

#include "platform.h"

extern "C" {
#include <libaiff/libaiff.h>
}

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
AudioExport::AudioExport(Sequencer* sequencer) : _sequencer(sequencer) {
    _audioExportDidStartFn = nullptr;
    _audioExportDidSetProgressFn = nullptr;
    _audioExportDidFinishFn = nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------
AudioExport::~AudioExport() {
    if (_exportAudioThread.joinable()) {
        _isAborted = true;
        _exportAudioThread.join();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// Audio export thread
void AudioExport::exportAudioThread() {
    AIFF_Ref exportAudioAIFFRef = static_cast<AIFF_Ref>(_exportAudioAIFFRef);

    Metronome::timePointType startTime = std::chrono::high_resolution_clock::now();
    Metronome::timePointType currentTime = startTime;
    Metronome::doublePrecisionDurationType period = Metronome::doublePrecisionDurationType(1.0 / 44100.0);

    AIFF_SetAudioFormat(exportAudioAIFFRef, 2, 44100.0, 16);
    AIFF_StartWritingSamples(exportAudioAIFFRef);

    bool isDone = false;
    bool isMetronomeDone = false;
    int nbFramesAtEnd = 44100;

    float lastProgress = 0.0f;
    while (!isDone && !_isAborted) {
        if (!isMetronomeDone) {
            if (!_sequencer->studio()->metronome()->performTick(startTime, currentTime)) isMetronomeDone = true;
        }

        GraphSampleType outA, outB;
        GraphSampleType* ioData[2];
        ioData[0] = &outA;
        ioData[1] = &outB;
        // Render a single frame
        _sequencer->studio()->mixer()->renderInput(1, ioData, 1);
        int32_t samples[2] = {(int32_t)(outA * 2147483647.0f), (int32_t)(outB * 2147483647.0f)};

        AIFF_WriteSamples32Bit(exportAudioAIFFRef, samples, 2);
        currentTime += period;

        if (isMetronomeDone) {
            if (nbFramesAtEnd == 0) {
                isDone = true;
            } else {
                nbFramesAtEnd--;
            }
        } else {
            float progress =
                static_cast<float>(_sequencer->studio()->metronome()->tick()) / static_cast<float>(_totalNbTicks);
            if (progress - lastProgress > 0.01f) {
                Platform::sharedInstance()->invoke([=] { setProgress(progress); });
                lastProgress = progress;
            }
        }
    }

    AIFF_EndWritingSamples(exportAudioAIFFRef);

    Platform::sharedInstance()->invoke([=] { exportAudioCompleted(); });
}

// ---------------------------------------------------------------------------------------------------------------------
bool AudioExport::exportAudio(const std::string& path) {
    if (_audioExportDidStartFn) _audioExportDidStartFn(this);

    // Calculate the total nb of ticks

    _totalNbTicks = 0;
    for (auto& track : _sequencer->sequence()->data.tracks) {
        UInt32 totalNbTicksInTrack = 0;
        for (auto event : track.events) totalNbTicksInTrack += event.tickCount;
        if (totalNbTicksInTrack > _totalNbTicks) _totalNbTicks = totalNbTicksInTrack;
    }

    _sequencer->stop();
    _sequencer->studio()->metronome()->moveToTick(0);

    _previousMasterMixerLevel = _sequencer->studio()->masterMixerLevel();
    _sequencer->studio()->setMasterMixerLevel(STUDIO_SOURCE_USER, 0.5f);

    _exportAudioAIFFRef = AIFF_OpenFile(path.c_str(), F_WRONLY);
    if (_exportAudioAIFFRef == nullptr) return false;

    // Start the sequencer for audio export
    _sequencer->playAudioExport();

    // Create a new audio export thread
    _isAborted = false;
    _exportAudioThread = std::thread(&AudioExport::exportAudioThread, this);

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
void AudioExport::exportAudioCompleted() {
    _exportAudioThread.join();

    AIFF_Ref exportAudioAIFFRef = static_cast<AIFF_Ref>(_exportAudioAIFFRef);
    AIFF_CloseFile(exportAudioAIFFRef);

    _sequencer->studio()->setMasterMixerLevel(STUDIO_SOURCE_USER, _previousMasterMixerLevel);

    if (_audioExportDidFinishFn) _audioExportDidFinishFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void AudioExport::setProgress(float progress) {
    if (_audioExportDidSetProgressFn) _audioExportDidSetProgressFn(this, progress);
}
