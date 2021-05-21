//
//  metronomecontroller.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2010-11-08.
//  Copyright (c) 2010-2021 Daniel Cliche. All rights reserved.
//

#include "metronomecontroller.h"

#include "platform.h"

#define METRONOME_CONTROLLER_ABORT_LEARNING_PERIOD 8.0

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
MelobaseCore::MetronomeController::MetronomeController(Metronome* metronome) {
    _metronome = metronome;
    _measureOrBeatPeriod = 0;
    _defaultNumerator = _tempNumerator = 4;
    _firstBeatTime = 0.0;

    _learningStartedFn = nullptr;

    _learningStartedFn = nullptr;
    _learningEndedFn = nullptr;
    _learningAbortedFn = nullptr;
    _didPerformMajorTickFn = nullptr;
    _didPerformMinorTickFn = nullptr;

    // We subscribe to metronome notifications

    using namespace std::placeholders;
    _majorTickFn = std::shared_ptr<Metronome::majorTickFnType>(
        new Metronome::majorTickFnType(std::bind(&MetronomeController::majorTick, this, _1)));
    _metronome->addMajorTickFn(_majorTickFn);
    _minorTickFn = std::shared_ptr<Metronome::majorTickFnType>(
        new Metronome::minorTickFnType(std::bind(&MetronomeController::minorTick, this, _1)));
    _metronome->addMinorTickFn(_minorTickFn);
}

// ---------------------------------------------------------------------------------------------------------------------
MelobaseCore::MetronomeController::~MetronomeController() {
    _metronome->removeMajorTickFn(_majorTickFn);
    _metronome->removeMinorTickFn(_minorTickFn);
}

// ---------------------------------------------------------------------------------------------------------------------
double MelobaseCore::MetronomeController::getTimestamp() {
    typedef std::chrono::duration<double> doublePrecisionDurationType;
    typedef std::chrono::time_point<std::chrono::high_resolution_clock, doublePrecisionDurationType> timePointType;

    timePointType timestampTP = std::chrono::high_resolution_clock::now();
    doublePrecisionDurationType duration = timestampTP.time_since_epoch();
    double timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() / 1000.0;

    return timestamp;
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::MetronomeController::startLearning() {
    if (_firstBeatTime == 0.0) {
        // We record the time of the first beat
        _firstBeatTime = getTimestamp();

        // We notify the delegate that the learning has started
        if (_learningStartedFn) _learningStartedFn(this);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::MetronomeController::stopLearning(bool aborted) {
    if (_firstBeatTime != 0.0) {
        // We clear the first beat time
        _firstBeatTime = 0.0;

        // We notify the delegate that the learning has ended
        if (!aborted) {
            if (_learningEndedFn) _learningEndedFn(this);
        } else {
            if (_learningAbortedFn) _learningAbortedFn(this);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::MetronomeController::abortLearningDelayed() {
    _metronome->stop();

    // If we still are learning, we stop
    stopLearning(true);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::MetronomeController::tapFirstBeat() {
    int bpm = 120;
    int numerator = _defaultNumerator;

    // If we are learning
    if (_firstBeatTime != 0.0) {
        _measureOrBeatPeriod = getTimestamp() - _firstBeatTime;

        if (_tempNumerator == 1) {
            // Twice the same tap so use the default numerator and the beat period
            bpm = (60.0f / _measureOrBeatPeriod);
        } else if (_tempNumerator == 2 || _tempNumerator == 3 || _tempNumerator == 4 || _tempNumerator == 5 ||
                   _tempNumerator == 7 || _tempNumerator == 11 || _tempNumerator == 6 || _tempNumerator == 9 ||
                   _tempNumerator == 12 || _tempNumerator == 15 || _tempNumerator == 21 || _tempNumerator == 33) {
            // Use the learned numerator and the measure period
            numerator = _tempNumerator;
            bpm = (60.0f * (float)numerator / _measureOrBeatPeriod);
        } else {
            // Unknown numerator, use 4 instead
            numerator = 4;
            bpm = (60.0f * (float)numerator / _measureOrBeatPeriod);
        }

        // We are no longer learning, so we set the metronome
        _metronome->setBPMs({{0, bpm}});
        _metronome->setTimeSignatures({{0, {numerator, 4}}});

        // We cancel any pending delayed request for stopping if any
        Platform::sharedInstance()->cancelDelayedInvokes(this);

        stopLearning(false);

    } else {
        // We want to learn, so we reset
        reset();

        // We play the major tick
        _metronome->playMajorTick();
        if (_didPerformMajorTickFn) _didPerformMajorTickFn(this);

        // We start learning
        startLearning();

        // We cancel any pending delayed request for stopping if any
        Platform::sharedInstance()->cancelDelayedInvokes(this);
        Platform::sharedInstance()->invokeDelayed(
            this, [=]() { abortLearningDelayed(); }, METRONOME_CONTROLLER_ABORT_LEARNING_PERIOD);
    }

    _tempNumerator = 1;
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::MetronomeController::recordingStarted() {
    // First, we stop learning
    stopLearning(false);

    // We make sure that we will not interfere
    Platform::sharedInstance()->cancelDelayedInvokes(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::MetronomeController::tapOtherBeat() {
    // If we are learning
    if (_firstBeatTime != 0.0) {
        _tempNumerator++;

        // We cancel any pending delayed request for stopping if any
        Platform::sharedInstance()->cancelDelayedInvokes(this);
        Platform::sharedInstance()->invokeDelayed(
            this, [=]() { abortLearningDelayed(); }, METRONOME_CONTROLLER_ABORT_LEARNING_PERIOD);

        // We play the minor tick
        _metronome->playMinorTick();
        if (_didPerformMinorTickFn) _didPerformMinorTickFn(this);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::MetronomeController::isLearning() { return (_firstBeatTime != 0.0); }

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::MetronomeController::reset() {
    stopLearning(true);
    Platform::sharedInstance()->cancelDelayedInvokes(this);
    if (_metronome->isRunning()) _metronome->stop();
    _metronome->setTick(0);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::MetronomeController::majorTick(Metronome* metronome) {
    if (_didPerformMajorTickFn) _didPerformMajorTickFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::MetronomeController::minorTick(Metronome* metronome) {
    if (_didPerformMinorTickFn) _didPerformMinorTickFn(this);
}
