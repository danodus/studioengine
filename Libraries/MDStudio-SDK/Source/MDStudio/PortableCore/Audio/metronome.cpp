//
//  metronome.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2010-11-10.
//  Copyright (c) 2010-2021 Daniel Cliche. All rights reserved.
//

#include "metronome.h"

#include <platform.h>

#include <algorithm>
#include <chrono>

using namespace MDStudio;

const unsigned int defaultBPM = 120;
const std::pair<UInt32, UInt32> defaultTimeSignature{4, 4};

// ---------------------------------------------------------------------------------------------------------------------
Metronome::Metronome() {
    _timeDivision = 480;
    _bpms = {{0, defaultBPM}};
    _timeSignatures = {{0, defaultTimeSignature}};
    _tick = 0;

    _areTicksAudible = false;
    _isRunning = false;
    _stopMetronomeThread = false;

    _playMajorTickFn = nullptr;
    _playMajorTickFn = nullptr;
    _didTickFn = nullptr;
    _bpmDidChangeFn = nullptr;
    _timeSignatureDidChangeFn = nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------
Metronome::~Metronome() {
    if (_isRunning) stop();
}

// ---------------------------------------------------------------------------------------------------------------------
void Metronome::notifyMajorTick() {
    for (auto majorTickFn : _majorTickFns) (*majorTickFn)(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void Metronome::notifyMinorTick() {
    for (auto minorTickFn : _minorTickFns) (*minorTickFn)(this);
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int Metronome::bpmForTick(UInt32 tick) {
    auto bpm = defaultBPM;

    for (auto b : _bpms) {
        if (tick >= b.first) {
            bpm = b.second;
        } else {
            break;
        }
    }

    return bpm;
}

// ---------------------------------------------------------------------------------------------------------------------
std::pair<UInt32, UInt32> Metronome::timeSignatureForTick(UInt32 tick) {
    auto timeSignature = defaultTimeSignature;

    for (auto ts : _timeSignatures) {
        if (tick >= ts.first) {
            timeSignature = ts.second;
        } else {
            break;
        }
    }

    return timeSignature;
}

// ---------------------------------------------------------------------------------------------------------------------
UInt32 Metronome::tickForTime(timePointType startTime, timePointType currentTime) {
    UInt32 currentTick = 0;
    auto bpms = _bpms;

    // Add initial default BPM if required
    if (bpms.size() == 0 || bpms[0].first > 0) bpms.insert(bpms.begin(), {0, defaultBPM});

    for (auto it = bpms.begin(); it != bpms.end(); ++it) {
        auto bpm = it->second;

        double beatPeriod = 60.0 / (double)bpm;
        double tickPeriod = beatPeriod / (double)_timeDivision;

        if ((it + 1) != bpms.end()) {
            UInt32 nextTick = (it + 1)->first;
            double segmentPeriod = (nextTick - it->first) * tickPeriod;
            timePointType nextTime = startTime + doublePrecisionDurationType(segmentPeriod);

            if (currentTime < nextTime) {
                double ticks = (currentTime - startTime) / doublePrecisionDurationType(tickPeriod);
                currentTick += static_cast<UInt32>(ticks);
                break;
            } else {
                double ticks = (nextTime - startTime) / doublePrecisionDurationType(tickPeriod);
                currentTick += static_cast<UInt32>(ticks);
                startTime += doublePrecisionDurationType(segmentPeriod);
            }
        } else {
            // Last segment
            double ticks = (currentTime - startTime) / doublePrecisionDurationType(tickPeriod);
            currentTick += static_cast<UInt32>(ticks);
        }
    }

    return currentTick;
}

// ---------------------------------------------------------------------------------------------------------------------
double Metronome::periodForTicks(UInt32 ticks) {
    UInt32 remainingTicks = ticks;

    double period = 0;
    auto bpms = _bpms;

    // Add initial BPM if required
    if (bpms.size() == 0 || bpms[0].first > 0) bpms.insert(bpms.begin(), {0, defaultBPM});

    for (auto it = bpms.begin(); it != bpms.end(); ++it) {
        auto bpm = it->second;

        double beatPeriod = 60.0 / (double)bpm;
        double tickPeriod = beatPeriod / (double)_timeDivision;

        if ((it + 1) != bpms.end()) {
            UInt32 nextTick = (it + 1)->first;
            UInt32 ticksInSegment = (nextTick - it->first);
            if (remainingTicks <= ticksInSegment) {
                period += remainingTicks * tickPeriod;
                break;
            } else {
                period += ticksInSegment * tickPeriod;
                remainingTicks -= ticksInSegment;
            }
        } else {
            // Last segment
            period += remainingTicks * tickPeriod;
        }
    }

    return period;
}

// ---------------------------------------------------------------------------------------------------------------------
void Metronome::getBeatAndMesureForTick(UInt32 tick, UInt32* beat, UInt32* measure, bool* isExactlyOnMeasure) {
    UInt32 lastBeat = 0;
    UInt32 lastMeasure = 0;
    UInt32 lastTicksPerMeasure = 0;

    for (size_t i = 0; i < _timeSignatures.size(); ++i) {
        auto timeSignature = _timeSignatures[i];

        UInt32 tsTick = timeSignature.first;

        if (tsTick > tick) break;

        UInt32 refTick = tick;
        if (i < (_timeSignatures.size() - 1)) {
            UInt32 maxTick = _timeSignatures[i + 1].first;
            if (refTick > maxTick) refTick = maxTick;
        }

        UInt32 numerator = timeSignature.second.first;
        if (numerator == 0) numerator = 4;

        UInt32 ticksPerMeasure = _timeDivision * numerator;

        if (lastTicksPerMeasure > 0) {
            if (tsTick % _timeDivision != 0) {
                lastBeat++;
            }

            if (tsTick % lastTicksPerMeasure != 0) {
                lastMeasure++;
            }
        }

        *beat = lastBeat + (refTick - tsTick) / _timeDivision;
        *measure = lastMeasure + (refTick - tsTick) / ticksPerMeasure;
        if (isExactlyOnMeasure) *isExactlyOnMeasure = ((refTick - tsTick) % ticksPerMeasure) == 0;

        lastBeat = *beat;
        lastMeasure = *measure;

        lastTicksPerMeasure = ticksPerMeasure;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool Metronome::performTick(timePointType startTime, timePointType currentTime) {
    if (!_didTickFn) return false;

    _tick = tickForTime(startTime, currentTime);
    return _didTickFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
// Metronome thread
void Metronome::metronomeThread() {
    //
    // Metronome main loop
    //

    UInt32 lastBeat = 0, lastMeasure = 0;
    bool isMajorTickForcefullyPlayed = false;

    timePointType startBeatTime;
    std::pair<UInt32, UInt32> lastTimeSignature{0, 0};
    unsigned int lastBPM = 0;

    startBeatTime = std::chrono::high_resolution_clock::now() - doublePrecisionDurationType(periodForTicks(_tick));
    getBeatAndMesureForTick(_tick, &lastBeat, &lastMeasure, &isMajorTickForcefullyPlayed);

    UInt32 lastTick = _tick;

    while (1) {
        // If the play thread is cancelled or if we no longer have event, we stop
        if (_stopMetronomeThread) {
            break;
        }

        _metronomeMutex.lock();

        auto tick = tickForTime(startBeatTime, std::chrono::high_resolution_clock::now());
        _tick = std::max(tick, lastTick);
        lastTick = _tick;

        // Notify if the time signature has changed
        auto timeSignature = timeSignatureForTick(_tick);
        if (lastTimeSignature.first > 0 && timeSignature != lastTimeSignature) {
            if (_timeSignatureDidChangeFn) _timeSignatureDidChangeFn(this, timeSignature);
        }
        lastTimeSignature = timeSignature;

        // Notify if the BPM has changed
        auto bpm = bpmForTick(_tick);
        if (lastBPM > 0 && bpm != lastBPM) {
            if (_bpmDidChangeFn) _bpmDidChangeFn(this, bpm);
        }
        lastBPM = bpm;

        // If a TS value is available at the current tick
        if (_timeSignatures.size() > 0 && _tick >= _timeSignatures[0].first) {
            UInt32 beat = 0, measure = 0;
            getBeatAndMesureForTick(_tick, &beat, &measure, nullptr);

            // We play the major or minor tick
            if (isMajorTickForcefullyPlayed || (measure != lastMeasure)) {
                if (_areTicksAudible) playMajorTick();
                Platform::sharedInstance()->invoke([=]() { notifyMajorTick(); }, this);
                lastMeasure = measure;
                lastBeat = beat;
            } else if (beat != lastBeat) {
                if (_areTicksAudible) playMinorTick();
                Platform::sharedInstance()->invoke([=]() { notifyMinorTick(); }, this);
                lastBeat = beat;
            }

            isMajorTickForcefullyPlayed = false;
        } else {
            // The current tick before the first time signature
            // Forcefully play the major tick when we reach the first time signature (if any)
            isMajorTickForcefullyPlayed = true;
        }

        _metronomeMutex.unlock();

        if (_didTickFn) {
            if (!_didTickFn(this)) break;
        }

        // We wait for the next tick
        double beatPeriod = 60.0 / (double)bpm;
        double tickPeriod = beatPeriod / (double)_timeDivision;
        std::this_thread::sleep_for(doublePrecisionDurationType(tickPeriod));
    }

    _isRunning = true;
}

// ---------------------------------------------------------------------------------------------------------------------
// Stop the metronome thread.
// This method will block until the thread has been stopped and disposed.
void Metronome::stopMetronomeThread() {
    if (!_isRunning) return;

    _stopMetronomeThread = true;
    _metronomeThread.join();

    // The thread may not had time to actually start, so we must isRunning to false ourselves just in case
    _isRunning = false;

    // Make sure that we no longer have remaining invokes from the metronome thread
    MDStudio::Platform::sharedInstance()->cancelDelayedInvokes(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void Metronome::start() {
    // First, we ensure that the metronome thread is no longer running
    stopMetronomeThread();

    _stopMetronomeThread = false;

    // We create a new metronome thread
    _metronomeThread = std::thread(&Metronome::metronomeThread, this);

    // We set the status as running
    _isRunning = true;
}

// ---------------------------------------------------------------------------------------------------------------------
void Metronome::stop() {
    // We stop the metronome thread
    stopMetronomeThread();
}

// ---------------------------------------------------------------------------------------------------------------------
void Metronome::moveToTick(UInt32 tick) {
    auto lastTimeSignature = timeSignatureForTick(_tick);
    auto lastBPM = bpmForTick(_tick);
    _tick = tick;
    auto timeSignature = timeSignatureForTick(_tick);
    auto bpm = bpmForTick(_tick);
    if ((lastTimeSignature != timeSignature) && _timeSignatureDidChangeFn)
        _timeSignatureDidChangeFn(this, timeSignature);
    if ((lastBPM != bpm) && _bpmDidChangeFn) _bpmDidChangeFn(this, bpm);

    if (_didMoveTickFn) {
        _didMoveTickFn(this);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Metronome::setBPMs(std::vector<std::pair<UInt32, unsigned int>> bpms) {
    std::lock_guard<std::mutex> lock(_metronomeMutex);
    _bpms = bpms;
}

// ---------------------------------------------------------------------------------------------------------------------
void Metronome::setTimeSignatures(std::vector<std::pair<UInt32, std::pair<UInt32, UInt32>>> timeSignatures) {
    std::lock_guard<std::mutex> lock(_metronomeMutex);
    _timeSignatures = timeSignatures;
}

// ---------------------------------------------------------------------------------------------------------------------
void Metronome::setBPMAtCurrentTick(unsigned int bpm) {
    std::lock_guard<std::mutex> lock(_metronomeMutex);

    if (bpmForTick(_tick) != bpm) {
        for (auto it = _bpms.begin(); it != _bpms.end(); ++it) {
            if (it->first > _tick) {
                _bpms.insert(it, {_tick, bpm});
                return;
            }
        }
        _bpms.push_back({_tick, bpm});
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Metronome::setTimeSignatureAtCurrentTick(std::pair<UInt32, UInt32> timeSignature) {
    std::lock_guard<std::mutex> lock(_metronomeMutex);

    if (timeSignatureForTick(_tick) != timeSignature) {
        for (auto it = _timeSignatures.begin(); it != _timeSignatures.end(); ++it) {
            if (it->first > _tick) {
                _timeSignatures.insert(it, {_tick, timeSignature});
                return;
            }
        }
        _timeSignatures.push_back({_tick, timeSignature});
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Metronome::playMajorTick() {
    if (_playMajorTickFn) {
        _playMajorTickFn(this);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Metronome::playMinorTick() {
    if (_playMinorTickFn) {
        _playMinorTickFn(this);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Metronome::addMajorTickFn(std::shared_ptr<majorTickFnType> majorTickFn) { _majorTickFns.push_back(majorTickFn); }

// ---------------------------------------------------------------------------------------------------------------------
void Metronome::removeMajorTickFn(std::shared_ptr<majorTickFnType> majorTickFn) {
    _majorTickFns.erase(std::find(_majorTickFns.begin(), _majorTickFns.end(), majorTickFn));
}

// ---------------------------------------------------------------------------------------------------------------------
void Metronome::addMinorTickFn(std::shared_ptr<minorTickFnType> minorTickFn) { _minorTickFns.push_back(minorTickFn); }

// ---------------------------------------------------------------------------------------------------------------------
void Metronome::removeMinorTickFn(std::shared_ptr<minorTickFnType> minorTickFn) {
    _minorTickFns.erase(std::find(_minorTickFns.begin(), _minorTickFns.end(), minorTickFn));
}
