//
//  sequencer.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2010-08-12.
//  Copyright (c) 2010-2021 Daniel Cliche. All rights reserved.
//

#include "sequencer.h"

#include <math.h>
#include <time.h>

#include <algorithm>
#include <cassert>
#include <iostream>

#include "platform.h"

#define IMPLEMENT_ADD_NOTIFICATION(_name_, _variable_)                                    \
    void Sequencer::add##_name_##Fn(std::shared_ptr<_variable_##FnType> _variable_##Fn) { \
        _##_variable_##Fns.push_back(_variable_##Fn);                                     \
    }

#define IMPLEMENT_REMOVE_NOTIFICATION(_name_, _variable_)                                                          \
    void Sequencer::remove##_name_##Fn(std::shared_ptr<_variable_##FnType> _variable_##Fn) {                       \
        _##_variable_##Fns.erase(std::find(_##_variable_##Fns.begin(), _##_variable_##Fns.end(), _variable_##Fn)); \
    }

#define IMPLEMENT_NOTIFICATION(_name_, _variable_) \
    IMPLEMENT_ADD_NOTIFICATION(_name_, _variable_) \
    IMPLEMENT_REMOVE_NOTIFICATION(_name_, _variable_)

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
Sequencer::Sequencer(void* owner, Studio* studio) : _owner(owner), _studio(studio) {
    _isPlaying = false;
    _isRecording = false;
    _isPaused = false;
    _soloTrackIndex = -1;
    _isLastNoteOffDetected = false;
}

// ---------------------------------------------------------------------------------------------------------------------
Sequencer::~Sequencer() { stop(); }

// ---------------------------------------------------------------------------------------------------------------------
// Sent by the playing thread when no more events are available
void Sequencer::reportPlaybackDidFinish() {
    for (auto playbackDidFinishFn : _playbackDidFinishFns) (*playbackDidFinishFn)(this);
}

// ---------------------------------------------------------------------------------------------------------------------
// Sent by the playing thread when the playback was interrupted
void Sequencer::reportPlaybackWasInterrupted() {
    for (auto playbackWasInterruptedFn : _playbackWasInterruptedFns) (*playbackWasInterruptedFn)(this);
}

// =====================================================================================================================
// Player
//

// ---------------------------------------------------------------------------------------------------------------------
bool Sequencer::processMetaEvent(const Event& event) {
    bool ret = true;

    switch (event.type) {
        case EVENT_TYPE_META_END_OF_TRACK:
            ret = false;
            break;
    }

    return ret;
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::processEvent(const Event& event, bool ignoreNotes, bool isPreloadingOnly,
                             bool* isLoadingInstrumentInBackground) {
    // We play the event
    switch (event.type) {
        case EVENT_TYPE_NOTE_ON:
            if (!ignoreNotes)
                _studio->playNote(STUDIO_SOURCE_SEQUENCER, event.param1,
                                  (event.param2 < 0) ? 1.0f : (Float32)event.param2 / 127.0f, event.channel);
            break;
        case EVENT_TYPE_NOTE_OFF:
            if (!ignoreNotes)
                _studio->releaseNote(STUDIO_SOURCE_SEQUENCER, event.param1,
                                     (event.param2 < 0) ? 1.0f : (Float32)event.param2 / 127.0f, event.channel);
            break;
        case EVENT_TYPE_KEY_AFTERTOUCH:
            if (!ignoreNotes)
                _studio->keyAftertouch(STUDIO_SOURCE_SEQUENCER, event.param1, (Float32)event.param2 / 127.0f,
                                       event.channel);
            break;
        case EVENT_TYPE_CHANNEL_AFTERTOUCH:
            if (!ignoreNotes)
                _studio->channelAftertouch(STUDIO_SOURCE_SEQUENCER, (Float32)event.param1 / 127.0f, event.channel);
            break;
        case EVENT_TYPE_SUSTAIN:
            _studio->setSustain(STUDIO_SOURCE_SEQUENCER, (Float32)event.param1 / 127.0f, event.channel);
            break;
        case EVENT_TYPE_PROGRAM_CHANGE: {
            int instrument = STUDIO_INSTRUMENT_GM_PIANO;
            if (event.param2 == -1) {
                // Compatibility conversion
                if (event.param1 >= 128) {
                    instrument = STUDIO_INSTRUMENT_FROM_PRESET(0, event.param1 - 128);
                } else {
                    switch (event.param1) {
                        case 1:  // STUDIO_INSTRUMENT_PIANO
                            instrument = STUDIO_INSTRUMENT_GM_PIANO;
                            break;
                        case 2:  // STUDIO_INSTRUMENT_HARPSICHORD
                            instrument = STUDIO_INSTRUMENT_GM_HARPSICHORD;
                            break;
                        case 3:  // STUDIO_INSTRUMENT_BASS
                            instrument = STUDIO_INSTRUMENT_GM_BASS;
                            break;
                        case 4:  // STUDIO_INSTRUMENT_STRINGS
                            instrument = STUDIO_INSTRUMENT_GM_STRINGS;
                            break;
                    }
                }
            } else {
                instrument = event.param1;
            }
            _studio->setInstrument(STUDIO_SOURCE_SEQUENCER,
                                   (event.channel == 9) ? STUDIO_INSTRUMENT_GM_STANDARD_DRUM_KIT : instrument,
                                   event.channel, isPreloadingOnly, isLoadingInstrumentInBackground);
        } break;
        case EVENT_TYPE_MIXER_LEVEL_CHANGE:
            _studio->setMixerLevel(STUDIO_SOURCE_SEQUENCER, (Float32)event.param1 / (event.param2 == 0 ? 127.0f : 100),
                                   event.channel);
            break;
        case EVENT_TYPE_MIXER_BALANCE_CHANGE:
            if (event.param2 == 0) {
                _studio->setMixerBalance(STUDIO_SOURCE_SEQUENCER, ((Float32)event.param1 / 127.0f) * 2.0f - 1.0f,
                                         event.channel);
            } else {
                _studio->setMixerBalance(STUDIO_SOURCE_SEQUENCER, (Float32)event.param1 / 100.0f, event.channel);
            }
            break;
        case EVENT_TYPE_PITCH_BEND: {
            Float32 multiplier = event.param2 > 0 ? (Float32)event.param2 : 1.0f;
            _studio->setPitchBend(STUDIO_SOURCE_SEQUENCER,
                                  multiplier * (((Float32)event.param1 - 0.5f) * 2.0f / 16383.0f - 1.0f),
                                  event.channel);
        } break;
        case EVENT_TYPE_MODULATION:
            _studio->setModulation(STUDIO_SOURCE_SEQUENCER, (Float32)event.param1 / 127.0f, event.channel);
            break;
        case EVENT_TYPE_CONTROL_CHANGE:
            _studio->setControlValue(STUDIO_SOURCE_SEQUENCER, event.param1, event.param2, event.channel);
            break;
        case EVENT_TYPE_SYSTEM_EXCLUSIVE:
            _studio->sendSystemExclusive(STUDIO_SOURCE_SEQUENCER, event.data, event.channel);
            break;
    }
}

// =====================================================================================================================
// Metronome delegates
//

// ---------------------------------------------------------------------------------------------------------------------
// This delegate is called at each tick from the metronome thread
bool Sequencer::metronomeDidTick(Metronome* metronome) {
    bool ret = true;

    if (_isRecording && _armedTrackIndices.empty()) return ret;

    // For each track
    int trackIndex = 0;
    int nbEndedTracks = 0;
    for (auto& track : _sequence->data.tracks) {
        // If we are recording and the track is armed, skip it
        if (_isRecording &&
            std::find(_armedTrackIndices.begin(), _armedTrackIndices.end(), trackIndex) != _armedTrackIndices.end()) {
            ++trackIndex;
            continue;
        }

        // If an EOT was detected for this track or if the last note off is detected with the detection enabled, we skip
        // it
        if (_playerEOTDetected[trackIndex] || (_isLastNoteOffDetected && metronome->tick() >= _lastNoteOffTick)) {
            ++nbEndedTracks;
            ++trackIndex;
            continue;
        }

        bool isMuted = false;

        // If the solo track is set and is not the current one, we are muted
        if ((_soloTrackIndex >= 0) && (trackIndex != _soloTrackIndex)) isMuted = true;

        // Check if track is muted
        if (std::find(_muteTrackIndices.begin(), _muteTrackIndices.end(), trackIndex) != _muteTrackIndices.end())
            isMuted = true;

        bool isEndOfTrackDetected = false;

        // If we are playing and the metronome tick has reached the current player tick
        while ((_isPlaying || _isRecording) && !_isPaused && !isEndOfTrackDetected &&
               (metronome->tick() >= _playerTicks[trackIndex])) {
            Event event;
            // If we have an event
            if (_playerEventIndices[trackIndex] < track.events.size()) {
                // We get the event
                event = track.events[_playerEventIndices[trackIndex]];

                //
                // We play the event
                //

                // printf("Metronome tick:%d, player tick: %d, Event: %02x\n", metronome->tick(), _playerTick,
                // event.type);

                // If this is a meta event
                if (event.type & 0xF0) {
                    // We process the meta event
                    if (!processMetaEvent(event)) {
                        isEndOfTrackDetected = true;
                        _playerEOTDetected[trackIndex] = true;
                    }
                } else {
                    // Rechannelize the event unless this is a mult-channel track
                    if (track.channel != SEQUENCE_TRACK_MULTI_CHANNEL) event.channel = track.channel;

                    // We process the event
                    processEvent(event, isMuted);
                }

                // We go to the next event
                if (!isEndOfTrackDetected) _playerEventIndices[trackIndex]++;

            }  // if we have an event

            // If we reached the end of the sequence or an end of track has been detected
            if (_playerEventIndices[trackIndex] >= track.events.size() || isEndOfTrackDetected) {
                ++nbEndedTracks;
                isEndOfTrackDetected = true;
                _playerEOTDetected[trackIndex] = true;
            } else {
                // We increase the player tick based on the next event tick
                event = track.events[_playerEventIndices[trackIndex]];
                _playerTicks[trackIndex] += event.tickCount;
            }
        }
        ++trackIndex;
    }  // for each track

    if (_isPlaying && (nbEndedTracks == _sequence->data.tracks.size())) {
        // We stop all the notes
        _studio->stopAllNotes(STUDIO_SOURCE_SEQUENCER);

        // The state is no longer playing
        _isPlaying = false;

        // We report on the main thread that the playback has finished
        Platform::sharedInstance()->invoke([=]() { reportPlaybackDidFinish(); });

        // We indicate to the metronome that we no longer want to run
        ret = false;
    }

    return ret;
}

// ---------------------------------------------------------------------------------------------------------------------
// This delegate is called when the metronome has moved to another specific tick value
// We must re-calculate the player tick position and event index based on the new tick.
// Special care must be taken in order to have a proper studio state.
void Sequencer::metronomeDidMoveTick(Metronome* metronome) {
    for (size_t i = 0; i < _sequence->data.tracks.size(); ++i) {
        _playerTicks[i] = 0;
        _playerEventIndices[i] = 0;
        _playerEOTDetected[i] = false;
    }

    // Make sure we do not send states to MIDI output
    bool wasMIDIOutputEnabled = _studio->isMIDIOutputEnabled();
    _studio->setIsMIDIOutputEnabled(false);

    // Disable studio state notifications to prevent build-up
    _studio->disableStateNotifications();

    // For each track
    int trackIndex = 0;
    for (auto track : _sequence->data.tracks) {
        // For each event
        for (auto event : track.events) {
            // If an EOT was detected for this track, we skip it
            if (_playerEOTDetected[trackIndex]) continue;

            // We increase the tick
            _playerTicks[trackIndex] += event.tickCount;

            // If we reach the desired metronome tick, we break
            if (_playerTicks[trackIndex] >= metronome->tick()) break;

            // If the even is a meta event, we must process it in order to keep the sequencer in a proper state
            if (event.type & 0xF0) {
                // We process the meta event
                if (!processMetaEvent(event)) {
                    _playerEOTDetected[trackIndex] = true;
                }
            } else {
                // We must process every non-note events

                // Rechannelize the event unless this is a mult-channel track
                if (track.channel != SEQUENCE_TRACK_MULTI_CHANNEL) event.channel = track.channel;

                processEvent(event, true);
            }

            // We go to the next event unless we are at the last one
            if (_playerEventIndices[trackIndex] < (track.events.size() - 1)) _playerEventIndices[trackIndex]++;
        }
        ++trackIndex;
    }  // for each track

    programMetronome();

    // Enable studio state notifications and dump current state
    _studio->enableStateNotifications();
    _studio->dumpStates(STUDIO_SOURCE_SEQUENCER);

    // Re-enable midi output if previously enabled
    _studio->setIsMIDIOutputEnabled(wasMIDIOutputEnabled);
}

// =====================================================================================================================
// Sequencer control
//

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::record() {
    if (_isRecording && _isPaused) {
        _isPaused = false;
        for (auto recordDidResume : _recordDidResumeFns) (*recordDidResume)(this);
        return;
    }

    if (_isPlaying || _isRecording) stop();

    auto lastMetronomeTick = _studio->metronome()->tick();

    // Reset the metronome
    _studio->resetMetronome();

    // If a track other than the first one is armed, program the metronome instead of using the default metronome states
    for (auto indice : _armedTrackIndices) {
        if (indice > 0) {
            programMetronome();
            _studio->moveMetronomeToTick(lastMetronomeTick);
            break;
        }
    }

    _isRecording = true;

    _sequence->data.tickPeriod = 60.0f / (_studio->metronome()->timeDivision() * 125.0f);

    // Remove the content of all armed tracks
    for (auto trackIndex : _armedTrackIndices) {
        assert(trackIndex < _sequence->data.tracks.size());
        _sequence->data.tracks[trackIndex].events.clear();
    }

    _recordStartTimestamp = getTimestamp();

    // We observe the states of the studio
    using namespace std::placeholders;
    _didSetMixerLevelFn = std::shared_ptr<Studio::didSetMixerLevelFnType>(
        new Studio::didSetMixerLevelFnType(std::bind(&Sequencer::didSetMixerLevel, this, _1, _2, _3, _4, _5, _6)));
    _studio->addDidSetMixerLevelFn(_didSetMixerLevelFn);
    _didSetMixerBalanceFn = std::shared_ptr<Studio::didSetMixerBalanceFnType>(
        new Studio::didSetMixerBalanceFnType(std::bind(&Sequencer::didSetMixerBalance, this, _1, _2, _3, _4, _5, _6)));
    _studio->addDidSetMixerBalanceFn(_didSetMixerBalanceFn);
    _didSetInstrumentFn = std::shared_ptr<Studio::didSetInstrumentFnType>(
        new Studio::didSetInstrumentFnType(std::bind(&Sequencer::didSetInstrument, this, _1, _2, _3, _4, _5, _6)));
    _studio->addDidSetInstrumentFn(_didSetInstrumentFn);
    _didPlayNoteFn = std::shared_ptr<Studio::didPlayNoteFnType>(
        new Studio::didPlayNoteFnType(std::bind(&Sequencer::didPlayNote, this, _1, _2, _3, _4, _5, _6, _7)));
    _studio->addDidPlayNoteFn(_didPlayNoteFn);
    _didReleaseNoteFn = std::shared_ptr<Studio::didReleaseNoteFnType>(
        new Studio::didReleaseNoteFnType(std::bind(&Sequencer::didReleaseNote, this, _1, _2, _3, _4, _5, _6, _7)));
    _studio->addDidReleaseNoteFn(_didReleaseNoteFn);
    _didSetSustainFn = std::shared_ptr<Studio::didSetSustainFnType>(
        new Studio::didSetSustainFnType(std::bind(&Sequencer::didSetSustain, this, _1, _2, _3, _4, _5, _6)));
    _studio->addDidSetSustainFn(_didSetSustainFn);
    _didSetPitchBendFn = std::shared_ptr<Studio::didSetPitchBendFnType>(
        new Studio::didSetPitchBendFnType(std::bind(&Sequencer::didSetPitchBend, this, _1, _2, _3, _4, _5, _6)));
    _studio->addDidSetPitchBendFn(_didSetPitchBendFn);
    _didSetModulationFn = std::shared_ptr<Studio::didSetModulationFnType>(
        new Studio::didSetModulationFnType(std::bind(&Sequencer::didSetModulation, this, _1, _2, _3, _4, _5, _6)));
    _studio->addDidSetModulationFn(_didSetModulationFn);
    _didSetControlValueFn = std::shared_ptr<Studio::didSetControlValueFnType>(new Studio::didSetControlValueFnType(
        std::bind(&Sequencer::didSetControlValue, this, _1, _2, _3, _4, _5, _6, _7)));
    _studio->addDidSetControlValueFn(_didSetControlValueFn);
    _didPerformKeyAftertouchFn =
        std::shared_ptr<Studio::didPerformKeyAftertouchFnType>(new Studio::didPerformKeyAftertouchFnType(
            std::bind(&Sequencer::didPerformKeyAftertouch, this, _1, _2, _3, _4, _5, _6, _7)));
    _studio->addDidPerformKeyAftertouchFn(_didPerformKeyAftertouchFn);
    _didPerformChannelAftertouchFn =
        std::shared_ptr<Studio::didPerformChannelAftertouchFnType>(new Studio::didPerformChannelAftertouchFnType(
            std::bind(&Sequencer::didPerformChannelAftertouch, this, _1, _2, _3, _4, _5, _6)));
    _studio->addDidPerformChannelAftertouchFn(_didPerformChannelAftertouchFn);
    _didSetTempoFn = std::shared_ptr<Studio::didSetTempoFnType>(
        new Studio::didSetTempoFnType(std::bind(&Sequencer::didSetTempo, this, _1, _2, _3, _4, _5)));
    _studio->addDidSetTempoFn(_didSetTempoFn);
    _didSetTimeSignatureFn = std::shared_ptr<Studio::didSetTimeSignatureFnType>(new Studio::didSetTimeSignatureFnType(
        std::bind(&Sequencer::didSetTimeSignature, this, _1, _2, _3, _4, _5, _6)));
    _studio->addDidSetTimeSignatureFn(_didSetTimeSignatureFn);

    // We dump the studio states for initial values
    // Note: We do so as a user in order to be recorded
    _studio->dumpStates(STUDIO_SOURCE_USER);

    MDStudio::Platform::sharedInstance()->invoke([=] {
        // Initialize the record activity monitor
        for (int channel = 0; channel < STUDIO_MAX_CHANNELS; ++channel) _hasChannelActivity[channel] = false;
    });

    // Start the metronome
    _studio->startMetronome();

    for (auto recordDidStart : _recordDidStartFns) (*recordDidStart)(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::resumePlayback() {
    // We start the mixer and metronome
    _studio->startMixer();
    _studio->startMetronome();

    // We are no longer paused
    _isPaused = false;

    // We notify that the playback did resume
    for (auto playbackDidResume : _playbackDidResumeFns) (*playbackDidResume)(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::preloadInstruments(bool* isLoadingInstrumentInBackground) {
    for (auto& track : _sequence->data.tracks) {
        auto rit = track.events.rbegin();
        for (; rit != track.events.rend(); ++rit) {
            auto event = *(rit);
            if (event.type == EVENT_TYPE_PROGRAM_CHANGE) {
                // Rechannelize the event unless this is a mult-channel track
                if (track.channel != SEQUENCE_TRACK_MULTI_CHANNEL) event.channel = track.channel;

                processEvent(event, true, true, isLoadingInstrumentInBackground);
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::resetPlayer() {
    for (int channel = 0; channel < STUDIO_MAX_CHANNELS; ++channel) {
        _studio->setSustain(STUDIO_SOURCE_SEQUENCER, 0.0f, channel);
        _studio->setPitchBend(STUDIO_SOURCE_SEQUENCER, 0.0f, channel);
        _studio->setModulation(STUDIO_SOURCE_SEQUENCER, 0.0f, channel);
        _studio->setMixerLevel(STUDIO_SOURCE_SEQUENCER, 0.6f, channel);
        _studio->setMixerBalance(STUDIO_SOURCE_SEQUENCER, 0.0f, channel);
        _studio->setControlValue(STUDIO_SOURCE_SEQUENCER, STUDIO_CONTROL_REVERB, 40, channel);  // Reverb
        _studio->setControlValue(STUDIO_SOURCE_SEQUENCER, STUDIO_CONTROL_CHORUS, 0, channel);   // Chorus
        _studio->setControlValue(STUDIO_SOURCE_SEQUENCER, STUDIO_CONTROL_EXPRESSION_CTRL, 127,
                                 channel);  // Expression control
        // Pitch bend range
        _studio->setControlValue(STUDIO_SOURCE_SEQUENCER, STUDIO_CONTROL_REG_PARAM_NUM_MSB, 0, channel);
        _studio->setControlValue(STUDIO_SOURCE_SEQUENCER, STUDIO_CONTROL_REG_PARAM_NUM_LSB, 0, channel);
        _studio->setControlValue(STUDIO_SOURCE_SEQUENCER, STUDIO_CONTROL_DATA_ENTRY_MSB, 2, channel);
        _studio->setControlValue(STUDIO_SOURCE_SEQUENCER, STUDIO_CONTROL_DATA_ENTRY_LSB, 0, channel);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::programMetronome() {
    // Program the metronome with the BPMs
    std::vector<std::pair<UInt32, unsigned int>> bpms;
    std::vector<std::pair<UInt32, std::pair<UInt32, UInt32>>> timeSignatures;
    UInt32 tick = 0;
    for (auto event : _sequence->data.tracks[0].events) {
        tick += event.tickCount;
        if (event.type == EVENT_TYPE_META_SET_TEMPO) {
            bpms.push_back({tick, 60000000.0f / (float)(event.param1)});
        } else if (event.type == EVENT_TYPE_META_TIME_SIGNATURE) {
            timeSignatures.push_back({tick, {event.param1, event.param2}});
        }
    }

    _studio->setMetronomeBPMs(STUDIO_SOURCE_SEQUENCER, bpms);
    _studio->setMetronomeTimeSignatures(STUDIO_SOURCE_SEQUENCER, timeSignatures);
}

// ---------------------------------------------------------------------------------------------------------------------
// Start or resume the playback.
// If the sequencer is stopped, we start from the beginning of the sequence.
void Sequencer::play() {
    // If the sequencer is paused
    if (_isPlaying && _isPaused) {
        // We simply resume the playback
        resumePlayback();

    } else {
        // If we are playing, we stop
        if (_isPlaying || _isRecording) stop();

        if (_studio->isMetronomeRunning()) _studio->stopMetronome();

        _isPaused = false;
        _isPlaying = true;

        _studio->startMixer();

        // We initialize the player
        int trackIndex = 0;
        for (auto track : _sequence->data.tracks) {
            _playerEventIndices[trackIndex] = 0;
            if (track.events.size() > 0) {
                Event event = track.events[0];
                _playerTicks[trackIndex] = event.tickCount;
            } else {
                _playerTicks[trackIndex] = 0;
            }
            _playerEOTDetected[trackIndex] = false;

            ++trackIndex;
        }

        // Program the metronome
        programMetronome();

        // Preload instruments
        bool isLoadingInstrumentsInBackground = false;
        preloadInstruments(&isLoadingInstrumentsInBackground);

        // We start the metronome
        _studio->setMetronomeTimeDivision(roundf(60.0f / (_sequence->data.tickPeriod * 125.0f)));
        _studio->resetMetronome();

        // If we are not loading instruments in background, we start the metronome right away
        if (!isLoadingInstrumentsInBackground) _studio->startMetronome();

        // We notify that the playback did start
        for (auto playbackDidStart : _playbackDidStartFns) (*playbackDidStart)(this);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::playAudioExport() {
    stop();
    resetPlayer();
    programMetronome();
    _isPlaying = true;
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::playFromCurrentLocation() {
    // If the sequencer is paused
    if (_isPlaying && _isPaused) {
        // We simply resume the playback
        resumePlayback();

    } else {
        // If we are playing, we stop
        if (_isPlaying || _isRecording) stop();

        _studio->stopMetronome();

        // If a sequence is assigned, we start the playback
        if (_sequence) {
            _isPaused = false;
            _isPlaying = true;

            _studio->startMixer();

            // Program the metronome
            programMetronome();

            // Ensure that the MIDI output is in proper state
            if (_studio->isMIDIOutputEnabled()) _studio->dumpStates(STUDIO_SOURCE_SEQUENCER);

            // Preload instruments
            bool isLoadingInstrumentsInBackground = false;
            preloadInstruments(&isLoadingInstrumentsInBackground);

            // We start the metronome
            _studio->startMetronome();

            // We notify that the playback did start
            for (auto playbackDidStart : _playbackDidStartFns) (*playbackDidStart)(this);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::pause() {
    if ((_isPlaying || _isRecording) && !_isPaused) {
        // We stop the metronome
        _studio->stopMetronome();

        // We stop all the notes
        _studio->stopAllNotes(STUDIO_SOURCE_SEQUENCER);

        _isPaused = true;
        if (_isPlaying) {
            for (auto playbackDidPause : _playbackDidPauseFns) (*playbackDidPause)(this);
        } else {
            for (auto recordDidPause : _recordDidPauseFns) (*recordDidPause)(this);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::removeEventsWithoutActivity(int trackIndex) {
    std::vector<Event> eventsToRemove;
    for (auto it = _sequence->data.tracks[trackIndex].events.begin();
         it != _sequence->data.tracks[trackIndex].events.end();) {
        Event event = (*it);
        if (event.type != EVENT_TYPE_META_END_OF_TRACK && event.type != EVENT_TYPE_META_SET_TEMPO &&
            event.type != EVENT_TYPE_META_TIME_SIGNATURE) {
            if (!_hasChannelActivity[event.channel]) {
                auto removedTickCount = it->tickCount;
                it = _sequence->data.tracks[trackIndex].events.erase(it);
                if (it != _sequence->data.tracks[trackIndex].events.end()) it->tickCount += removedTickCount;
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::stop() {
    if (_isPlaying) {
        //
        // We stop the player
        //

        // We stop all the notes
        _studio->stopAllNotes(STUDIO_SOURCE_SEQUENCER);

        // We stop the metronome
        _studio->stopMetronome();

        _isPlaying = _isPaused = false;

        for (auto playbackWasInterrupted : _playbackWasInterruptedFns) (*playbackWasInterrupted)(this);

    } else if (_isRecording) {
        // We stop all the notes
        // Note: This will record note off events if any
        _studio->stopAllNotes(STUDIO_SOURCE_USER);

        // We stop the metronome
        _studio->stopMetronome();

        // Add an end of track event
        addEvent(STUDIO_SOURCE_USER, EVENT_TYPE_META_END_OF_TRACK, getTimestamp(), _studio->metronome()->tick(), 0, -1,
                 -1);

        // We no longer observe the studio events
        _studio->removeDidSetMixerLevelFn(_didSetMixerLevelFn);
        _studio->removeDidSetMixerBalanceFn(_didSetMixerBalanceFn);
        _studio->removeDidSetInstrumentFn(_didSetInstrumentFn);
        _studio->removeDidPlayNoteFn(_didPlayNoteFn);
        _studio->removeDidReleaseNoteFn(_didReleaseNoteFn);
        _studio->removeDidSetSustainFn(_didSetSustainFn);
        _studio->removeDidSetPitchBendFn(_didSetPitchBendFn);
        _studio->removeDidSetModulationFn(_didSetModulationFn);
        _studio->removeDidSetControlValueFn(_didSetControlValueFn);
        _studio->removeDidPerformKeyAftertouchFn(_didPerformKeyAftertouchFn);
        _studio->removeDidPerformChannelAftertouchFn(_didPerformChannelAftertouchFn);
        _studio->removeDidSetTempoFn(_didSetTempoFn);
        _studio->removeDidSetTimeSignatureFn(_didSetTimeSignatureFn);

        _isRecording = _isPaused = false;

        // Remove all the events from the channels without any activity
        if (_armedTrackIndices.empty()) {
            for (int trackIndex = 0; trackIndex < _sequence->data.tracks.size(); ++trackIndex)
                removeEventsWithoutActivity(trackIndex);
        } else {
            for (auto trackIndex : _armedTrackIndices) removeEventsWithoutActivity(trackIndex);
        }
        updateCurrentSequence();

        for (auto recordDidFinish : _recordDidFinishFns) (*recordDidFinish)(this);
    }
}

// =====================================================================================================================
// Add events
//

// ---------------------------------------------------------------------------------------------------------------------
// Add an event with given parameters
void Sequencer::addEvent(int source, UInt8 type, Float64 timestamp, UInt32 tick, UInt8 channel, SInt32 param1,
                         SInt32 param2) {
    // If the source is from the sequencer, do nothing
    if (source == STUDIO_SOURCE_SEQUENCER) return;

    if (_isRecording && !_isPaused) {
        // We add the event
        Event event;
        event.type = type;
        event.channel = channel;
        event.param1 = param1;
        event.param2 = param2;

        for (auto trackIndex : _armedTrackIndices) {
            if ((_sequence->data.format == SEQUENCE_DATA_FORMAT_MULTI_TRACK) &&
                (event.type != EVENT_TYPE_META_END_OF_TRACK)) {
                // If the event is a tempo or time signature event, we only add it to the first track
                // We do not accept any other type of events on the first track
                if ((event.type == EVENT_TYPE_META_TIME_SIGNATURE) || (event.type == EVENT_TYPE_META_SET_TEMPO)) {
                    if (trackIndex != 0) continue;
                } else {
                    // Any event type except time signature or tempo
                    if (trackIndex == 0) continue;
                }
            }

            bool isAdded = true;

            // If this is a channel event, only consider the events from the channel of the track unless it is
            // a multi-channel track
            if (_sequence->data.tracks[trackIndex].channel != SEQUENCE_TRACK_MULTI_CHANNEL) {
                if ((event.type < 0xF0) && (_sequence->data.tracks[trackIndex].channel != event.channel))
                    isAdded = false;
            }

            if (isAdded) {
                // We calculate the tick count delta for the event
                event.tickCount = tick - _recordLastTicks[trackIndex];
                _sequence->data.tracks[trackIndex].addEvent(event);
                // The new last timestamp is now
                _recordLastTicks[trackIndex] = tick;
            }
        }  // for each armed track index
    }
}

// =====================================================================================================================
// Studio notifications
//

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::didSetMixerLevel(Studio* sender, int source, double timestamp, UInt32 tick, int channel,
                                 Float32 mixerLevel) {
    if (_isRecording) {
        _hasChannelActivity[channel] = true;
        addEvent(source, EVENT_TYPE_MIXER_LEVEL_CHANGE, timestamp, tick, channel, roundf(mixerLevel * 127.0f), 0);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::didSetMixerBalance(Studio* sender, int source, double timestamp, UInt32 tick, int channel,
                                   Float32 mixerBalance) {
    if (_isRecording) {
        _hasChannelActivity[channel] = true;
        addEvent(source, EVENT_TYPE_MIXER_BALANCE_CHANGE, timestamp, tick, channel,
                 roundf((mixerBalance + 1.0f) / 2.0f * 127.0f), 0);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::didSetInstrument(Studio* sender, int source, double timestamp, UInt32 tick, int channel,
                                 int instrument) {
    if (_isRecording) {
        _hasChannelActivity[channel] = true;
        addEvent(source, EVENT_TYPE_PROGRAM_CHANGE, timestamp, tick, channel, instrument, 0);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::didPlayNote(Studio* sender, int source, double timestamp, UInt32 tick, int channel, int pitch,
                            Float32 velocity) {
    if (_isRecording) {
        _hasChannelActivity[channel] = true;
        addEvent(source, EVENT_TYPE_NOTE_ON, timestamp, tick, channel, pitch, velocity * 127);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::didReleaseNote(Studio* sender, int source, double timestamp, UInt32 tick, int channel, int pitch,
                               Float32 velocity) {
    if (_isRecording) {
        _hasChannelActivity[channel] = true;
        addEvent(source, EVENT_TYPE_NOTE_OFF, timestamp, tick, channel, pitch, velocity * 127);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::didSetSustain(Studio* sender, int source, double timestamp, UInt32 tick, int channel, Float32 sustain) {
    if (_isRecording) {
        _hasChannelActivity[channel] = true;
        addEvent(source, EVENT_TYPE_SUSTAIN, timestamp, tick, channel, sustain * 127, -1);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::didSetPitchBend(Studio* sender, int source, double timestamp, UInt32 tick, int channel,
                                Float32 pitchBend) {
    if (_isRecording) {
        _hasChannelActivity[channel] = true;
        addEvent(source, EVENT_TYPE_PITCH_BEND, timestamp, tick, channel, (pitchBend + 2.0f) * 16383.0f / 4.0f + 0.5f,
                 2);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::didSetModulation(Studio* sender, int source, double timestamp, UInt32 tick, int channel,
                                 Float32 modulation) {
    if (_isRecording) {
        _hasChannelActivity[channel] = true;
        addEvent(source, EVENT_TYPE_MODULATION, timestamp, tick, channel, modulation * 127, -1);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::didSetControlValue(Studio* sender, int source, double timestamp, UInt32 tick, int channel,
                                   UInt32 control, UInt32 value) {
    if (_isRecording) {
        _hasChannelActivity[channel] = true;
        addEvent(source, EVENT_TYPE_CONTROL_CHANGE, timestamp, tick, channel, control, value);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::didPerformKeyAftertouch(Studio* sender, int source, double timestamp, UInt32 tick, int channel,
                                        int pitch, Float32 value) {
    if (_isRecording) {
        _hasChannelActivity[channel] = true;
        addEvent(source, EVENT_TYPE_KEY_AFTERTOUCH, timestamp, tick, channel, pitch, value * 127);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::didPerformChannelAftertouch(Studio* sender, int source, double timestamp, UInt32 tick, int channel,
                                            Float32 value) {
    if (_isRecording) {
        _hasChannelActivity[channel] = true;
        addEvent(source, EVENT_TYPE_CHANNEL_AFTERTOUCH, timestamp, tick, channel, value * 127, -1);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::didSetTempo(Studio* sender, int source, double timestamp, UInt32 tick, UInt32 mpqn) {
    if (_isRecording) {
        addEvent(source, EVENT_TYPE_META_SET_TEMPO, timestamp, tick, 0, mpqn, -1);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::didSetTimeSignature(Studio* sender, int source, double timestamp, UInt32 tick, unsigned int numerator,
                                    unsigned int denominator) {
    if (_isRecording) {
        addEvent(source, EVENT_TYPE_META_TIME_SIGNATURE, timestamp, tick, 0, numerator, denominator);
    }
}

// =====================================================================================================================
// Sequence property
//

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<Sequence> Sequencer::sequence() { return _sequence; }

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::updateCurrentSequence() {
    if (_sequence) {
        _playerEventIndices.resize(_sequence->data.tracks.size());
        _playerTicks.resize(_sequence->data.tracks.size());
        _recordLastTicks.resize(_sequence->data.tracks.size());
        _playerEOTDetected.resize(_sequence->data.tracks.size());

        _lastNoteOffTick = 0;
        for (size_t i = 0; i < _sequence->data.tracks.size(); ++i) {
            _playerTicks[i] = 0;
            _playerEventIndices[i] = 0;
            _recordLastTicks[i] = 0;
            _playerEOTDetected[i] = false;

            // Find the last note off event tick
            UInt32 tick = 0;
            auto& track = _sequence->data.tracks[i];
            for (auto& event : track.events) {
                tick += event.tickCount;
                if (event.type == EVENT_TYPE_NOTE_OFF && tick > _lastNoteOffTick) _lastNoteOffTick = tick;
            }
        }

    } else {
        _playerTicks.clear();
        _playerEventIndices.clear();
        _recordLastTicks.clear();
        _playerEOTDetected.clear();
        _lastNoteOffTick = 0;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::setSequence(std::shared_ptr<Sequence> sequence) {
    if (_sequence != sequence) {
        assert(!_isPlaying && !_isRecording);

        _sequence = sequence;
        updateCurrentSequence();

        // If the metronome is not currently running
        if (!_studio->isMetronomeRunning()) _studio->resetMetronome();

        // We set the metronome delegate
        if (_sequence) {
            using namespace std::placeholders;
            _studio->metronome()->setDidTickFn(std::bind(&Sequencer::metronomeDidTick, this, _1));
            _studio->metronome()->setDidMoveTickFn(std::bind(&Sequencer::metronomeDidMoveTick, this, _1));
            _studio->metronome()->setTimeDivision(roundf(60.0f / (sequence->data.tickPeriod * 125.0f)));
        } else {
            _studio->metronome()->setDidTickFn(nullptr);
            _studio->metronome()->setDidMoveTickFn(nullptr);
        }

        for (auto sequenceAssigned : _sequenceAssignedFns) (*sequenceAssigned)(this);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::setMuteTrackIndices(std::vector<int> muteTrackIndices) {
    if (_sequence && (_isRecording || _isPlaying)) {
        for (auto indice : muteTrackIndices) {
            assert(indice < _sequence->data.tracks.size());
            auto& track = _sequence->data.tracks[indice];
            if (track.channel == SEQUENCE_TRACK_MULTI_CHANNEL) {
                _studio->stopAllNotes(STUDIO_SOURCE_SEQUENCER);
                // All the notes have been stopped, so there is no need to continue
                break;
            } else {
                // Stop the notes for the track channel
                _studio->stopNotes(STUDIO_SOURCE_SEQUENCER, track.channel);
            }
        }
    }
    _muteTrackIndices = muteTrackIndices;
}

// ---------------------------------------------------------------------------------------------------------------------
void Sequencer::setSoloTrackIndex(int soloTrackIndex) {
    if (_sequence && (soloTrackIndex >= 0) && (_isRecording || _isPlaying)) {
        int trackIndex = 0;
        for (auto& track : _sequence->data.tracks) {
            if ((trackIndex > 0) && (trackIndex != soloTrackIndex)) {
                if (track.channel == SEQUENCE_TRACK_MULTI_CHANNEL) {
                    _studio->stopAllNotes(STUDIO_SOURCE_SEQUENCER);
                    // All the notes have been stopped, so there is no need to continue
                    break;
                } else {
                    // Stop the notes for the track channel
                    _studio->stopNotes(STUDIO_SOURCE_SEQUENCER, track.channel);
                }
            }
            ++trackIndex;
        }
    }
    _soloTrackIndex = soloTrackIndex;
}

IMPLEMENT_NOTIFICATION(PlaybackDidStart, playbackDidStart)
IMPLEMENT_NOTIFICATION(PlaybackDidFinish, playbackDidFinish)
IMPLEMENT_NOTIFICATION(PlaybackWasInterrupted, playbackWasInterrupted)
IMPLEMENT_NOTIFICATION(PlaybackDidPause, playbackDidPause)
IMPLEMENT_NOTIFICATION(PlaybackDidResume, playbackDidResume)
IMPLEMENT_NOTIFICATION(RecordDidStart, recordDidStart)
IMPLEMENT_NOTIFICATION(RecordDidPause, recordDidPause)
IMPLEMENT_NOTIFICATION(RecordDidResume, recordDidResume)
IMPLEMENT_NOTIFICATION(RecordDidFinish, recordDidFinish)
IMPLEMENT_NOTIFICATION(SequenceAssigned, sequenceAssigned)
