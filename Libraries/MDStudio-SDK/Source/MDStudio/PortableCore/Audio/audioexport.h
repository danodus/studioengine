//
//  audioexport.h
//  MDStudio
//
//  Created by Daniel Cliche on 2015-08-29.
//  Copyright (c) 2015-2020 Daniel Cliche. All rights reserved.
//

#ifndef AUDIOEXPORT_H
#define AUDIOEXPORT_H

#include <thread>

#include "sequencer.h"

namespace MDStudio {

class AudioExport {
   public:
    typedef std::function<void(AudioExport* sender)> audioExportDidStartFnType;
    typedef std::function<void(AudioExport* sender, float progress)> audioExportDidSetProgressFnType;
    typedef std::function<void(AudioExport* sender)> audioExportDidFinishFnType;

   private:
    Sequencer* _sequencer;

    std::thread _exportAudioThread;
    void* _exportAudioAIFFRef;

    void exportAudioThread();
    void exportAudioCompleted();

    audioExportDidStartFnType _audioExportDidStartFn;
    audioExportDidSetProgressFnType _audioExportDidSetProgressFn;
    audioExportDidFinishFnType _audioExportDidFinishFn;

    UInt32 _totalNbTicks;
    Float32 _previousMasterMixerLevel;

    std::atomic<bool> _isAborted;

    void setProgress(float progress);

   public:
    AudioExport(Sequencer* sequencer);
    ~AudioExport();

    bool exportAudio(const std::string& path);

    void setAudioExportDidStartFn(audioExportDidStartFnType audioExportDidStart) {
        _audioExportDidStartFn = audioExportDidStart;
    }
    void setAudioExportDidSetProgressFn(audioExportDidSetProgressFnType audioExportDidSetProgress) {
        _audioExportDidSetProgressFn = audioExportDidSetProgress;
    }
    void setAudioExportDidFinishFn(audioExportDidFinishFnType audioExportDidFinish) {
        _audioExportDidFinishFn = audioExportDidFinish;
    }
};

}  // namespace MDStudio

#endif  // AUDIOEXPORT_H
