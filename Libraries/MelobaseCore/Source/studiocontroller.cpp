//
//  studiocontroller.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2010-11-29.
//  Copyright (c) 2010-2021 Daniel Cliche. All rights reserved.
//

#include "studiocontroller.h"
#include "platform.h"

#include <algorithm>

#define STUDIO_CONTROLLER_INITIAL_STOP_SEQUENCER_PERIOD		16.0		// in seconds
#define STUDIO_CONTROLLER_STOP_SEQUENCER_PERIOD				4.0			// in seconds

#define STUDIO_CONTROLLER_MINIMUM_NB_RECORDED_EVENTS		1			// Minimum number of recorded events in order to save

using namespace MelobaseCore;

// ---------------------------------------------------------------------------------------------------------------------
MelobaseCore::StudioController::StudioController(void *owner, MDStudio::Studio *studio, MDStudio::Sequencer *sequencer) : _owner(owner)
{
    using namespace std::placeholders;

    _notePlayingCounter = 0;
    _sequence = nullptr;
    _lastSequence = nullptr;

    _noteIsPlayingStateFn = nullptr;
    _resetFilterFn = nullptr;
    _recordingDidStartFn = nullptr;
    _recordingDidFinishFn = nullptr;
    _beepFn = nullptr;
    _saveSequencesFn = nullptr;
    _updateMetronomeViewFn = nullptr;
    _updateSequencerStatusFn = nullptr;
    _majorTickFn = nullptr;
    _minorTickFn = nullptr;
    _didGoToBeginningFn = nullptr;
    
    _isAutoStopRecordEnabled = true;
    _isAutoStopRecordBypassed = false;
    _autoStopRecordPeriod = STUDIO_CONTROLLER_STOP_SEQUENCER_PERIOD;
    
    for (int channel = 0; channel < STUDIO_MAX_CHANNELS; ++channel) {
        _userSustainValues[channel] = 0.0f;
        _userPitchBendValues[channel] = 0.0f;
        _userModulationValues[channel] = 0.0f;
    }

    // We set the studioß
    _studio = studio;

    // We set the sequencer
    _sequencer = sequencer;

    // We create the metronome controller
    _metronomeController = new MetronomeController(studio->metronome());
    _metronomeController->setLearningStartedFn(std::bind(&StudioController::metronomeControllerLearningStarted, this, _1));
    _metronomeController->setLearningEndedFn(std::bind(&StudioController::metronomeControllerLearningEnded, this, _1));
    _metronomeController->setLearningAbortedFn(std::bind(&StudioController::metronomeControllerLearningAborted, this, _1));
    _metronomeController->setDidPerformMajorTickFn(std::bind(&StudioController::metronomeControllerDidPerformMajorTick, this, _1));
    _metronomeController->setDidPerformMinorTickFn(std::bind(&StudioController::metronomeControllerDidPerformMinorTick, this, _1));

    // The initial tap beat status
    _firstBeatEnabled = true;
    _otherBeatEnabled = false;

    // The initial status
    _status = StudioControllerStatusAwaitingTempoOrNote;
    _armed = NO;

    // We set the initial metronome sound states
    _isMetronomeSoundEnabledDuringPlayback = false;
    _isMetronomeSoundEnabledDuringRecording = true;

    _lastSequence = nullptr;
    _currentFolder = nullptr;

    //
    // We subscribe to sequencer notifications
    //

    _sequencerPlaybackDidFinishFn = std::shared_ptr<MDStudio::Sequencer::playbackDidFinishFnType>(new MDStudio::Sequencer::playbackDidFinishFnType(std::bind(&StudioController::sequencerPlaybackDidFinish, this, _1)));
    _sequencer->addPlaybackDidFinishFn(_sequencerPlaybackDidFinishFn);

    // Set initial instruments for all the channels

    for (int channel = 0; channel < STUDIO_MAX_CHANNELS; ++channel) {
        _studio->setInstrument(STUDIO_SOURCE_USER, (channel == 1) ? STUDIO_INSTRUMENT_GM_BASS : (channel == 9) ? STUDIO_INSTRUMENT_GM_STANDARD_DRUM_KIT : STUDIO_INSTRUMENT_GM_PIANO, channel);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::startRecording(std::shared_ptr<SequencesFolder> folder)
{
    // We ensure that the playback is stopped
    _sequencer->stop();

    // We keep the last sequence
    _lastSequence = _sequence;

    // We initialize the number of recorded events
    _nbRecordedEvents = 0;

    // We reset the note playing counter
    _notePlayingCounter = 0;
    if (_noteIsPlayingStateFn)
        _noteIsPlayingStateFn(this, false);

    // We ask the delegate to reset its filer
    if (_resetFilterFn)
        _resetFilterFn(this);
    
    // If no tracks are armed or no sequence set to the sequencer
    if (!_sequencer->sequence() || _sequencer->armedTrackIndices().empty()) {
        
        // We create a new sequence entity
        _sequence = std::shared_ptr<Sequence>(new Sequence());
        _sequence->data.tickPeriod = 0.001;
        _sequence->date = _sequence->version = _sequence->dataVersion = MDStudio::getTimestamp();
        
        std::vector<int> armedTrackIndices;
        
        // Always arm the first track
        armedTrackIndices.push_back(0);
        _sequencer->setArmedTrackIndices(armedTrackIndices);

        if (folder) {
            _sequence->folder = folder;
        }

        setSequence(_sequence);
        
        // Do not bypass the auto stop
        _isAutoStopRecordBypassed = false;

    } else {
        
        // We have at least one armed track
        
        // Update the sequencer in case the sequence was modified
        auto lastMetronomeTick = _studio->metronome()->tick();
        _sequencer->setSequence(getStudioSequence(_sequence));
        _studio->metronome()->setTick(lastMetronomeTick);
        
        // If the first track is armed
        bool isFirstTrackArmed = std::find(_sequencer->armedTrackIndices().begin(), _sequencer->armedTrackIndices().end(), 0) != _sequencer->armedTrackIndices().end();

        if (isFirstTrackArmed) {
            
            // Use the first time signature if available at the beginning
            if (_studio->metronome()->timeSignatures().size() > 0 && _studio->metronome()->timeSignatures()[0].first == 0) {
                _studio->metronome()->setTimeSignatures({_studio->metronome()->timeSignatures()[0]});
            } else {
                _studio->metronome()->setTimeSignatures({});
            }
            
            // Use the first tempo if available at the beginning
            if (_studio->metronome()->bpms().size() > 0 && _studio->metronome()->bpms()[0].first == 0) {
                _studio->metronome()->setBPMs({_studio->metronome()->bpms()[0]});
            } else {
                _studio->metronome()->setBPMs({});
            }
        }
        
        // Bypass the auto stop
        _isAutoStopRecordBypassed = true;
    }
    
    // We notify the metronome controller that a recording has started
    _metronomeController->recordingStarted();

    // We set the metronome sound state
    _studio->metronome()->setAreTicksAudible(_isMetronomeSoundEnabledDuringRecording);
    
    // Set the sustain, pitch bend and modulation based on the user state and not simply the studio state
    // We do this in order to avoid having these values initially set by the sequencer even if the user has not requested it
    // Note: The source is set as the sequencer in order to prevent those events from being recorded due to invocation.
    for (int channel = 0; channel < STUDIO_MAX_CHANNELS; ++channel) {
        _studio->setSustain(STUDIO_SOURCE_SEQUENCER, _userSustainValues[channel], channel);
        _studio->setPitchBend(STUDIO_SOURCE_SEQUENCER, _userPitchBendValues[channel], channel);
        _studio->setModulation(STUDIO_SOURCE_SEQUENCER, _userModulationValues[channel], channel);
    }
    
    // We record
    _sequencer->record();

    // We update the status
    _status = StudioControllerStatusRecording;

    // We are armed
    _armed = true;

    // We inform the delegate that the recording session has started
    if (_recordingDidStartFn)
        _recordingDidStartFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
// Stop recording, save sequences and beep
void MelobaseCore::StudioController::stopRecording()
{

    // First, we cancel any pending delayed request for stopping if any
    MDStudio::Platform::sharedInstance()->cancelDelayedInvokes(this);

    // We are no longer armed
    _armed = false;

    // We stop recording, save and beep
    if (_sequencer->isRecording()) {

        // We stop the sequencer
        _sequencer->stop();

        // If we have not reached the minimum number of notes
        if (_nbRecordedEvents < STUDIO_CONTROLLER_MINIMUM_NB_RECORDED_EVENTS) {
            setSequence(_lastSequence);
        } else {
            
            // We ask a beep
            if (_beepFn)
                _beepFn(this);

            // We save the sequences
            if (_saveSequencesFn)
                _saveSequencesFn(this);
        }
    }

    // We update the status
    _status = StudioControllerStatusAwaitingTempoOrNote;

    // We notify the delegate to update that the status has changed
    if (_updateSequencerStatusFn)
        _updateSequencerStatusFn(this);

    if (_recordingDidFinishFn)
        _recordingDidFinishFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::stopRecordingUnlessNotePlaying()
{
    if (_notePlayingCounter == 0) {
        stopRecording();
    } else {
        if (_isAutoStopRecordEnabled && !_isAutoStopRecordBypassed)
            MDStudio::Platform::sharedInstance()->invokeDelayed(this, [=](){stopRecordingUnlessNotePlaying();}, _autoStopRecordPeriod);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// Stop recording session and metronome
void MelobaseCore::StudioController::stopRecordingAndMetronome()
{
    stopRecording();
    _metronomeController->reset();
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::eventReceived()
{
    if (!_armed)
        return;
    
    // We increment the number of recorded events
    _nbRecordedEvents++;
    
    // We perform a stop after a fixed delay unless a note is still playing
    MDStudio::Platform::sharedInstance()->cancelDelayedInvokes(this);
    if (_isAutoStopRecordEnabled && !_isAutoStopRecordBypassed) {
        MDStudio::Platform::sharedInstance()->invokeDelayed(this, [=](){stopRecordingUnlessNotePlaying();}, _autoStopRecordPeriod);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::keyPressed(int pitch, Float32 velocity, int channel)
{
    eventReceived();
    
    // If we are armed
    if (_armed) {
        
        // We play the note
        _studio->playNote(STUDIO_SOURCE_USER, pitch, velocity, channel);

        // We increment the note playing counter
        _notePlayingCounter++;
        if (_noteIsPlayingStateFn)
            MDStudio::Platform::sharedInstance()->invoke([=]{
                _noteIsPlayingStateFn(this, true);
            });
    } else {
        // We are not armed, therefore we simply play the note
        _studio->playNote(STUDIO_SOURCE_USER, pitch, velocity, channel);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::keyReleased(int pitch, Float32 velocity, int channel)
{
    // If we are armed
    if (_armed) {
        if (_notePlayingCounter > 0) {

            _studio->releaseNote(STUDIO_SOURCE_USER, pitch, velocity, channel);

            // We decrease the note playing counter
            _notePlayingCounter--;

            if (_notePlayingCounter == 0) {
                if (_noteIsPlayingStateFn)
                    MDStudio::Platform::sharedInstance()->invoke([=]{
                        _noteIsPlayingStateFn(this, false);
                    });
            }

            // We perform a stop after a fixed delay unless a note is still playing
            MDStudio::Platform::sharedInstance()->cancelDelayedInvokes(this);
            if (_isAutoStopRecordEnabled && !_isAutoStopRecordBypassed) {
                MDStudio::Platform::sharedInstance()->invokeDelayed(this, [=](){stopRecordingUnlessNotePlaying();}, _autoStopRecordPeriod);
            }
        }
    } else {
        // We are not armed, therefore we simply release the note
        _studio->releaseNote(STUDIO_SOURCE_USER, pitch, velocity, channel);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::keyAftertouch(int pitch, Float32 value, int channel)
{
    eventReceived();
    _studio->keyAftertouch(STUDIO_SOURCE_USER, pitch, value, channel);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::channelAftertouch(Float32 value, int channel)
{
    eventReceived();
    _studio->channelAftertouch(STUDIO_SOURCE_USER, value, channel);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::setInstrument(int instrument, int channel)
{
    eventReceived();
    _studio->setInstrument(STUDIO_SOURCE_USER, instrument, channel);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::setSustain(Float32 sustain, int channel)
{
    eventReceived();

    // We only consider on/off changes at the user input for now
    if (sustain >= 0.5f && _userSustainValues[channel] < 0.5f) {
        sustain = 1.0f;
        _userSustainValues[channel] = sustain;
        _studio->setSustain(STUDIO_SOURCE_USER, sustain, channel);
    } else if (sustain < 0.5f && _userSustainValues[channel] >= 0.5f) {
        sustain = 0.0f;
        _userSustainValues[channel] = sustain;
        _studio->setSustain(STUDIO_SOURCE_USER, sustain, channel);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::setPitchBend(Float32 pitchBend, int channel)
{
    eventReceived();
    _userPitchBendValues[channel] = pitchBend;
    _studio->setPitchBend(STUDIO_SOURCE_USER, pitchBend, channel);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::setModulation(Float32 modulation, int channel)
{
    eventReceived();
    _userModulationValues[channel] = modulation;
    _studio->setModulation(STUDIO_SOURCE_USER, modulation, channel);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::setMixerLevel(Float32 level, int channel)
{
    eventReceived();
    _studio->setMixerLevel(STUDIO_SOURCE_USER, level, channel);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::setMixerBalance(Float32 balance, int channel)
{
    eventReceived();
    _studio->setMixerBalance(STUDIO_SOURCE_USER, balance, channel);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::setControlValue(UInt32 control, UInt32 value, int channel)
{
    eventReceived();
    _studio->setControlValue(STUDIO_SOURCE_USER, control, value, channel);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::setBPMAtCurrentTick(unsigned int bpm)
{
    eventReceived();
    _studio->metronome()->setBPMAtCurrentTick(bpm);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::setTimeSignatureAtCurrentTick(std::pair<UInt32, UInt32> timeSignature)
{
    eventReceived();
    _studio->metronome()->setTimeSignatureAtCurrentTick(timeSignature);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::stopAllNotes()
{
    _studio->stopAllNotes(STUDIO_SOURCE_USER);
    _notePlayingCounter = 0;
    if (_noteIsPlayingStateFn)
        _noteIsPlayingStateFn(this, false);
}

// =====================================================================================================================
// Sequencer control
//

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::setSequence(std::shared_ptr<Sequence> sequence)
{
    // First, we make sure that the sequencer is stopped
    _sequencer->stop();

    // We reset the last sequence
    _lastSequence = nullptr;

    // We update the status
    _status = StudioControllerStatusAwaitingTempoOrNote;
    
    _sequence = sequence;
    
    // We set the entity to the sequencer
    _sequencer->setSequence(getStudioSequence(_sequence));

    // We notify the delegate that the status has changed
    if(_updateSequencerStatusFn)
        _updateSequencerStatusFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<Sequence> MelobaseCore::StudioController::sequence()
{
    return _sequence;
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::updateSequence()
{
    // Retrieve the sequence from the sequencer
    auto sequencerSequence = getMelobaseCoreSequence(_sequencer->sequence());
    if (sequencerSequence) {
        _sequence->data.tickPeriod = sequencerSequence->data.tickPeriod;
        _sequence->data.tracks = sequencerSequence->data.tracks;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::stop()
{
    if (_status != StudioControllerStatusAwaitingTempoOrNote) {
        _sequencer->stop();
        _status = StudioControllerStatusAwaitingTempoOrNote;

        // We notify the delegate that the status has changed
        if(_updateSequencerStatusFn)
            _updateSequencerStatusFn(this);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::playPause()
{

    // If the sequencer is recording, we stop it
    if (_sequencer->isRecording())
        stopRecordingAndMetronome();

    // If the sequencer is currently not playing
    if (!_sequencer->isPlaying()) {

        if (_sequence) {
            // We keep track of the last sequence
            _lastSequence = _sequence;
            
            UInt32 lastTick = _studio->metronome()->tick();
            _sequencer->setSequence(getStudioSequence(_sequence));

            // We set the metronome sound state
            _studio->metronome()->setAreTicksAudible(_isMetronomeSoundEnabledDuringPlayback);
            
            // Reset the player
            _sequencer->resetPlayer();
            
            _studio->moveMetronomeToTick(lastTick);

            // We start the playback
            _sequencer->playFromCurrentLocation();
            _status = StudioControllerStatusPlaying;
        }
    } else {
        // The sequencer is playing, so we pause
        stop();
    }

    // We notify the delegate that the status has changed
    if (_updateSequencerStatusFn)
        _updateSequencerStatusFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::goToBeginning()
{
    if (_sequencer->isRecording()) {
        // First, we stop the recording and the metronome
        stopRecordingAndMetronome();

        // We set the metronome position to the beginning
        _studio->metronome()->moveToTick(0);

    } else  {
        bool wasPausedOrStopped = _sequencer->isPaused() || !_sequencer->isPlaying();
        _sequencer->pause();
        _studio->metronome()->moveToTick(0);
        if (!wasPausedOrStopped)
            _sequencer->play();
    }
    
    if (_didGoToBeginningFn)
        _didGoToBeginningFn(this);
}

// =====================================================================================================================
// Metronome controller delegates
//

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::metronomeControllerLearningStarted(MetronomeController *metronomeController)
{
    _status = StudioControllerStatusLearningTempo;
    _firstBeatEnabled = false;
    _otherBeatEnabled = true;
    if(_updateMetronomeViewFn)
        _updateMetronomeViewFn(this);
    if (_updateSequencerStatusFn)
        _updateSequencerStatusFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::metronomeControllerLearningEnded(MetronomeController *metronomeController)
{
    _otherBeatEnabled = false;

    startRecording(_currentFolder);

    // We perform a stop after a fixed delay unless a note is still playing
    MDStudio::Platform::sharedInstance()->cancelDelayedInvokes(this);
    MDStudio::Platform::sharedInstance()->invokeDelayed(this, [=](){stopRecordingUnlessNotePlaying();}, STUDIO_CONTROLLER_INITIAL_STOP_SEQUENCER_PERIOD);

    // We update the status
    _status = StudioControllerStatusRecording;

    if (_updateMetronomeViewFn)
        _updateMetronomeViewFn(this);
    if (_updateSequencerStatusFn)
        _updateSequencerStatusFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::metronomeControllerLearningAborted(MetronomeController *metronomeController)
{
    _otherBeatEnabled = false;

    // We update the status
    _status = StudioControllerStatusAwaitingTempoOrNote;

    if (_updateMetronomeViewFn)
        _updateMetronomeViewFn(this);
    if (_updateSequencerStatusFn)
        _updateSequencerStatusFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::metronomeControllerDidPerformMajorTick(MetronomeController *metronomeController)
{
    if (_majorTickFn)
        _majorTickFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::metronomeControllerDidPerformMinorTick(MetronomeController *metronomeController)
{
    _firstBeatEnabled = true;
    if (_updateMetronomeViewFn)
        _updateMetronomeViewFn(this);
    if (_minorTickFn)
        _minorTickFn(this);
}

// =====================================================================================================================
// Metronome tap beat
//

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::tapFirstBeat()
{
    if (_sequencer->isRecording()) {
        stopRecording();
    } else if (_sequencer->isPlaying()) {
        _sequencer->stop();
    }

    _metronomeController->tapFirstBeat();
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::tapOtherBeat()
{
    _metronomeController->tapOtherBeat();
}

// =====================================================================================================================
// Sequencer notifications
//

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::sequencerPlaybackDidFinish(MDStudio::Sequencer *sequencer)
{
    _status = StudioControllerStatusAwaitingTempoOrNote;
    if (_updateSequencerStatusFn)
        _updateSequencerStatusFn(this);
}

// =====================================================================================================================
// Metronome sound
//

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::setIsMetronomeSoundEnabledDuringPlayback(bool state)
{
    _isMetronomeSoundEnabledDuringPlayback = state;
    if (_sequencer->isPlaying()) {
        _studio->metronome()->setAreTicksAudible(state);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::StudioController::setIsMetronomeSoundEnabledDuringRecording(bool state)
{
    _isMetronomeSoundEnabledDuringRecording = state;
    if (_sequencer->isRecording()) {
        _studio->metronome()->setAreTicksAudible(state);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::StudioController::isTrackArmed(int trackIndex)
{
    return std::find(_sequencer->armedTrackIndices().begin(), _sequencer->armedTrackIndices().end(), trackIndex) != _sequencer->armedTrackIndices().end();
}
