//
//  metronome.h
//  MDStudio
//
//  Created by Daniel Cliche on 2010-11-10.
//  Copyright (c) 2010-2021 Daniel Cliche. All rights reserved.
//

#ifndef METRONOME_H
#define METRONOME_H

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

#include "../types.h"

namespace MDStudio {

class Metronome {
   public:
    typedef std::function<void(Metronome* sender)> majorTickFnType;
    typedef std::function<void(Metronome* sender)> minorTickFnType;
    typedef std::function<void(Metronome* sender)> didMoveTickFnType;
    typedef std::function<void(Metronome* sender)> playMajorTickFnType;
    typedef std::function<void(Metronome* sender)> playMinorTickFnType;

    typedef std::function<bool(Metronome* sender)> didTickFnType;

    typedef std::function<void(Metronome* sender, unsigned int bpm)> bpmDidChangeFnType;
    typedef std::function<void(Metronome* sender, std::pair<UInt32, UInt32> timeSignature)>
        timeSignatureDidChangeFnType;

    typedef std::chrono::duration<double> doublePrecisionDurationType;
    typedef std::chrono::time_point<std::chrono::high_resolution_clock, doublePrecisionDurationType> timePointType;

   private:
    UInt32 _timeDivision;

    bool _stopMetronomeThread;

    std::vector<std::pair<UInt32, unsigned int>> _bpms;
    std::vector<std::pair<UInt32, std::pair<UInt32, UInt32>>> _timeSignatures;

    std::atomic<UInt32> _tick;

    std::thread _metronomeThread;

    bool _areTicksAudible;
    std::atomic<bool> _isRunning;

    std::mutex _metronomeMutex;

    void notifyMajorTick();
    void notifyMinorTick();
    void metronomeThread();
    void stopMetronomeThread();

    std::vector<std::shared_ptr<majorTickFnType>> _majorTickFns;
    std::vector<std::shared_ptr<minorTickFnType>> _minorTickFns;
    didMoveTickFnType _didMoveTickFn;
    playMajorTickFnType _playMajorTickFn;
    playMinorTickFnType _playMinorTickFn;
    didTickFnType _didTickFn;
    bpmDidChangeFnType _bpmDidChangeFn;
    timeSignatureDidChangeFnType _timeSignatureDidChangeFn;

    UInt32 tickForTime(timePointType startTime, timePointType currentTime);
    double periodForTicks(UInt32 ticks);
    void getBeatAndMesureForTick(UInt32 tick, UInt32* beat, UInt32* measure, bool* isExactlyOnMeasure);

   public:
    Metronome();
    ~Metronome();

    // Performs a metronome tick manually
    // Returns true if more events are available, false otherwise
    bool performTick(timePointType startTime, timePointType currentTime);

    void start();
    void stop();
    void moveToTick(UInt32 tick);
    void setTick(UInt32 tick) { _tick = tick; }

    // The following functions can be called while the metronome is running
    void setBPMs(std::vector<std::pair<UInt32, unsigned int>> bpms);
    void setTimeSignatures(std::vector<std::pair<UInt32, std::pair<UInt32, UInt32>>> timeSignatures);
    void setBPMAtCurrentTick(unsigned int bpm);
    void setTimeSignatureAtCurrentTick(std::pair<UInt32, UInt32> timeSignature);

    void setTimeDivision(UInt32 timeDivision) { _timeDivision = timeDivision; }
    UInt32 timeDivision() { return _timeDivision; }

    UInt32 tick() { return _tick; }
    std::vector<std::pair<UInt32, unsigned int>> bpms() { return _bpms; }
    std::vector<std::pair<UInt32, std::pair<UInt32, UInt32>>> timeSignatures() { return _timeSignatures; }
    unsigned int bpmForTick(UInt32 tick);
    std::pair<UInt32, UInt32> timeSignatureForTick(UInt32 tick);

    void addMajorTickFn(std::shared_ptr<majorTickFnType> majorTickFn);
    void removeMajorTickFn(std::shared_ptr<majorTickFnType> majorTickFn);

    void addMinorTickFn(std::shared_ptr<minorTickFnType> minorTickFn);
    void removeMinorTickFn(std::shared_ptr<minorTickFnType> minorTickFn);

    void setPlayMajorTickFn(playMajorTickFnType playMajorTickFn) { _playMajorTickFn = playMajorTickFn; }
    void setPlayMinorTickFn(playMinorTickFnType playMinorTickFn) { _playMinorTickFn = playMinorTickFn; }

    void setDidTickFn(didTickFnType didTickFn) { _didTickFn = didTickFn; }
    void setDidMoveTickFn(didMoveTickFnType didMoveTickFn) { _didMoveTickFn = didMoveTickFn; }

    // Warning: not called from the main thread
    void setBPMDidChangeFn(bpmDidChangeFnType bpmDidChangeFn) { _bpmDidChangeFn = bpmDidChangeFn; }
    void setTimeSignatureDidChangeFn(timeSignatureDidChangeFnType timeSignatureDidChangeFn) {
        _timeSignatureDidChangeFn = timeSignatureDidChangeFn;
    }

    bool isRunning() { return _isRunning; }

    void playMajorTick();
    void playMinorTick();

    void setAreTicksAudible(bool areTicksAudible) { _areTicksAudible = areTicksAudible; }
};

}  // namespace MDStudio

#endif  // METRONOME_H
