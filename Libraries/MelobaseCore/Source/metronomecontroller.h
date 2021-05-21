//
//  metronomecontroller.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2010-11-08.
//  Copyright (c) 2010-2021 Daniel Cliche. All rights reserved.
//

#ifndef METRONOMECONTROLLER_H
#define METRONOMECONTROLLER_H

#include <metronome.h>

#include <functional>

namespace MelobaseCore {

class MetronomeController {
   public:
    typedef std::function<void(MetronomeController* sender)> learningStartedFnType;
    typedef std::function<void(MetronomeController* sender)> learningEndedFnType;
    typedef std::function<void(MetronomeController* sender)> learningAbortedFnType;
    typedef std::function<void(MetronomeController* sender)> didPerformMajorTickFnType;
    typedef std::function<void(MetronomeController* sender)> didPerformMinorTickFnType;

   private:
    MDStudio::Metronome* _metronome;

    double _firstBeatTime;
    double _measureOrBeatPeriod;

    int _tempNumerator;

    int _defaultNumerator;

    // Subscribed notifications
    std::shared_ptr<MDStudio::Metronome::majorTickFnType> _majorTickFn;
    std::shared_ptr<MDStudio::Metronome::minorTickFnType> _minorTickFn;

    // Delegate functions
    learningStartedFnType _learningStartedFn;
    learningEndedFnType _learningEndedFn;
    learningAbortedFnType _learningAbortedFn;
    didPerformMajorTickFnType _didPerformMajorTickFn;
    didPerformMinorTickFnType _didPerformMinorTickFn;

    double getTimestamp();

    void majorTick(MDStudio::Metronome* metronome);
    void minorTick(MDStudio::Metronome* metronome);

   public:
    MetronomeController(MDStudio::Metronome* metronome);
    ~MetronomeController();
    void startLearning();
    void stopLearning(bool aborted);
    void abortLearningDelayed();
    void tapFirstBeat();
    void recordingStarted();
    void tapOtherBeat();
    bool isLearning();
    void reset();
    void setDefaultNumerator(int defaultNumerator) { _defaultNumerator = defaultNumerator; }

    void setLearningStartedFn(learningStartedFnType learningStartedFn) { _learningStartedFn = learningStartedFn; }
    void setLearningEndedFn(learningEndedFnType learningEndedFn) { _learningEndedFn = learningEndedFn; }
    void setLearningAbortedFn(learningAbortedFnType learningAbortedFn) { _learningAbortedFn = learningAbortedFn; }
    void setDidPerformMajorTickFn(didPerformMajorTickFnType didPerformMajorTick) {
        _didPerformMajorTickFn = didPerformMajorTick;
    }
    void setDidPerformMinorTickFn(didPerformMinorTickFnType didPerformMinorTick) {
        _didPerformMinorTickFn = didPerformMinorTick;
    }
};

}  // namespace MelobaseCore

#endif  // METRONOMECONTROLLER_H
