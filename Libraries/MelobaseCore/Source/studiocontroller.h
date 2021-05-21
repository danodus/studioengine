//
//  studiocontroller.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2010-11-29.
//  Copyright (c) 2010-2021 Daniel Cliche. All rights reserved.
//

#ifndef STUDIOCONTROLLER_H
#define STUDIOCONTROLLER_H

#include "melobasecore_sequence.h"
#include "studio.h"
#include "sequencer.h"
#include "metronomecontroller.h"

#include <memory>

namespace MelobaseCore {

class StudioController
{
public:
    typedef std::function<void(StudioController *sender, bool state)>noteIsPlayingStateFnType;
    typedef std::function<void(StudioController *sender)>resetFilterFnType;
    typedef std::function<void(StudioController *sender)>recordingDidStartFnType;
    typedef std::function<void(StudioController *sender)>recordingDidFinishFnType;
    typedef std::function<void(StudioController *sender)>beepFnType;
    typedef std::function<void(StudioController *sender)>saveSequencesFnType;
    typedef std::function<void(StudioController *sender)>updateMetronomeViewFnType;
    typedef std::function<void(StudioController *sender)>updateSequencerStatusFnType;
    typedef std::function<void(StudioController *sender)>majorTickFnType;
    typedef std::function<void(StudioController *sender)>minorTickFnType;
    typedef std::function<void(StudioController *sender)>didGoToBeginningFnType;

    enum { StudioControllerStatusAwaitingTempoOrNote, StudioControllerStatusLearningTempo, StudioControllerStatusRecording, StudioControllerStatusPlaying };

private:

    void *_owner;

    MDStudio::Studio *_studio;
    MDStudio::Sequencer *_sequencer;
    MetronomeController *_metronomeController;

    std::atomic<int> _notePlayingCounter;
    int _nbRecordedEvents;

    std::shared_ptr<Sequence> _sequence;
    std::shared_ptr<Sequence> _lastSequence;
    std::shared_ptr<SequencesFolder> _currentFolder;

    bool _firstBeatEnabled, _otherBeatEnabled;
    std::atomic<int> _status;

    bool _isMetronomeSoundEnabledDuringPlayback, _isMetronomeSoundEnabledDuringRecording;
    std::atomic<bool> _armed;
    
    bool _isAutoStopRecordEnabled, _isAutoStopRecordBypassed;
    float _autoStopRecordPeriod;
    
    std::atomic<Float32> _userSustainValues[STUDIO_MAX_CHANNELS];
    std::atomic<Float32> _userPitchBendValues[STUDIO_MAX_CHANNELS];
    std::atomic<Float32> _userModulationValues[STUDIO_MAX_CHANNELS];

    // Subscribed notifications
    std::shared_ptr<MDStudio::Sequencer::playbackDidFinishFnType> _sequencerPlaybackDidFinishFn;

    // Delegate functions
    noteIsPlayingStateFnType _noteIsPlayingStateFn;
    resetFilterFnType _resetFilterFn;
    recordingDidStartFnType _recordingDidStartFn;
    recordingDidFinishFnType _recordingDidFinishFn;
    beepFnType _beepFn;
    saveSequencesFnType _saveSequencesFn;
    updateMetronomeViewFnType _updateMetronomeViewFn;
    updateSequencerStatusFnType _updateSequencerStatusFn;
    majorTickFnType _majorTickFn;
    minorTickFnType _minorTickFn;
    didGoToBeginningFnType _didGoToBeginningFn;

    void sequencerPlaybackDidFinish(MDStudio::Sequencer *sequencer);

    void stopRecordingUnlessNotePlaying();

    void metronomeControllerLearningStarted(MetronomeController *metronomeController);
    void metronomeControllerLearningEnded(MetronomeController *metronomeController);
    void metronomeControllerLearningAborted(MetronomeController *metronomeController);
    void metronomeControllerDidPerformMajorTick(MetronomeController *metronomeController);
    void metronomeControllerDidPerformMinorTick(MetronomeController *metronomeController);
    
    void eventReceived();

public:
    StudioController(void *owner, MDStudio::Studio *studio, MDStudio::Sequencer *sequencer);
    
    void *owner() { return _owner; }

    void startRecording(std::shared_ptr<SequencesFolder> folder);
    void stopRecording();
    void stopRecordingAndMetronome();
    void stop();
    void playPause();
    void goToBeginning();

    void keyPressed(int pitch, Float32 velocity, int channel);
    void keyReleased(int pitch, Float32 velocity, int channel);
    void keyAftertouch(int pitch, Float32 value, int channel);
    void channelAftertouch(Float32 value, int channel);
    void setInstrument(int instrument, int channel);
    void setSustain(Float32 sustain, int channel);
    void setPitchBend(Float32 pitchBend, int channel);
    void setModulation(Float32 modulation, int channel);
    void setMixerLevel(Float32 level, int channel);
    void setMixerBalance(Float32 balance, int channel);
    void setControlValue(UInt32 control, UInt32 value, int channel);
    void setBPMAtCurrentTick(unsigned int bpm);
    void setTimeSignatureAtCurrentTick(std::pair<UInt32, UInt32> timeSignature);

    void stopAllNotes();

    void tapFirstBeat();
    void tapOtherBeat();
    
    bool firstBeatEnabled() { return _firstBeatEnabled; }
    bool otherBeatEnabled() { return _otherBeatEnabled; }

    void setSequence(std::shared_ptr<Sequence> sequence);
    std::shared_ptr<Sequence> sequence();
    void updateSequence();

    void setIsMetronomeSoundEnabledDuringPlayback(bool state);
    void setIsMetronomeSoundEnabledDuringRecording(bool state);

    void setSaveSequencesFn(saveSequencesFnType saveSequencesFn) { _saveSequencesFn = saveSequencesFn; }
    void setRecordingDidStartFn(recordingDidStartFnType recordingDidStartFn) { _recordingDidStartFn = recordingDidStartFn; }
    void setRecordingDidFinishFn(recordingDidFinishFnType recordingDidFinishFn) { _recordingDidFinishFn = recordingDidFinishFn; }
    void setUpdateSequencerStatusFn(updateSequencerStatusFnType updateSequencerStatusFn) { _updateSequencerStatusFn = updateSequencerStatusFn; }
    void setMajorTickFn(majorTickFnType majorTickFn) { _majorTickFn = majorTickFn; }
    void setMinorTickFn(minorTickFnType minorTickFn) { _minorTickFn = minorTickFn; }
    void setDidGoToBeginningFn(didGoToBeginningFnType didGoToBeginningFn) { _didGoToBeginningFn = didGoToBeginningFn; }
    void setUpdateMetronomeViewFn(updateMetronomeViewFnType updateMetronomeViewFn) { _updateMetronomeViewFn = updateMetronomeViewFn; }
    void setBeepFn(beepFnType beepFn) { _beepFn = beepFn; }
    
    MDStudio::Studio *studio() { return _studio; }
    MDStudio::Sequencer *sequencer() { return _sequencer; }
    MelobaseCore::MetronomeController *metronomeController() { return _metronomeController; }

    int status() { return _status; }
    
    bool isTrackArmed(int trackIndex);
    
    void setCurrentFolder(std::shared_ptr<SequencesFolder> folder) { _currentFolder = folder; }
    std::shared_ptr<SequencesFolder> currentFolder() { return _currentFolder; }
    
    void setIsAutoStopRecordEnabled(bool isEnabled) { _isAutoStopRecordEnabled = isEnabled; }
    void setAutoStopRecordPeriod(float period) { _autoStopRecordPeriod = period; }

};
    
} // namespace MelobaseCore

#endif // STUDIOCONTROLLER_H
