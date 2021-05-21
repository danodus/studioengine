//
//  sequencer.h
//  MDStudio
//
//  Created by Daniel Cliche on 2010-08-12.
//  Copyright 2010-2020 Daniel Cliche. All rights reserved.
//

#ifndef SEQUENCER_H
#define SEQUENCER_H

#include <memory>

#include "event.h"
#include "sequence.h"
#include "studio.h"

namespace MDStudio {

class Sequencer {
   public:
    typedef std::function<void(Sequencer* sender)> playbackDidStartFnType;
    typedef std::function<void(Sequencer* sender)> playbackDidFinishFnType;
    typedef std::function<void(Sequencer* sender)> playbackWasInterruptedFnType;
    typedef std::function<void(Sequencer* sender)> playbackDidPauseFnType;
    typedef std::function<void(Sequencer* sender)> playbackDidResumeFnType;
    typedef std::function<void(Sequencer* sender)> recordDidStartFnType;
    typedef std::function<void(Sequencer* sender)> recordDidPauseFnType;
    typedef std::function<void(Sequencer* sender)> recordDidResumeFnType;
    typedef std::function<void(Sequencer* sender)> recordDidFinishFnType;
    typedef std::function<void(Sequencer* sender)> sequenceAssignedFnType;

   private:
    void* _owner;

    std::shared_ptr<MDStudio::Sequence> _sequence;
    Studio* _studio;

    std::vector<UInt32> _recordLastTicks;
    double _recordStartTimestamp;

    bool _isPlaying, _isRecording, _isPaused;

    bool _hasChannelActivity[STUDIO_MAX_CHANNELS];

    std::vector<int> _armedTrackIndices;
    std::vector<int> _muteTrackIndices;
    int _soloTrackIndex;

    // Player
    std::vector<UInt32> _playerEventIndices;
    std::vector<UInt32> _playerTicks;
    std::vector<bool> _playerEOTDetected;

    std::atomic<bool> _isLastNoteOffDetected;
    UInt32 _lastNoteOffTick;

    // Published notifications
    std::vector<std::shared_ptr<playbackDidStartFnType>> _playbackDidStartFns;
    std::vector<std::shared_ptr<playbackDidFinishFnType>> _playbackDidFinishFns;
    std::vector<std::shared_ptr<playbackWasInterruptedFnType>> _playbackWasInterruptedFns;
    std::vector<std::shared_ptr<playbackDidPauseFnType>> _playbackDidPauseFns;
    std::vector<std::shared_ptr<playbackDidResumeFnType>> _playbackDidResumeFns;
    std::vector<std::shared_ptr<recordDidStartFnType>> _recordDidStartFns;
    std::vector<std::shared_ptr<recordDidPauseFnType>> _recordDidPauseFns;
    std::vector<std::shared_ptr<recordDidResumeFnType>> _recordDidResumeFns;
    std::vector<std::shared_ptr<recordDidFinishFnType>> _recordDidFinishFns;
    std::vector<std::shared_ptr<sequenceAssignedFnType>> _sequenceAssignedFns;

    // Subscribed notifications
    std::shared_ptr<Studio::didSetMixerLevelFnType> _didSetMixerLevelFn;
    std::shared_ptr<Studio::didSetMixerBalanceFnType> _didSetMixerBalanceFn;
    std::shared_ptr<Studio::didSetInstrumentFnType> _didSetInstrumentFn;
    std::shared_ptr<Studio::didPlayNoteFnType> _didPlayNoteFn;
    std::shared_ptr<Studio::didReleaseNoteFnType> _didReleaseNoteFn;
    std::shared_ptr<Studio::didSetSustainFnType> _didSetSustainFn;
    std::shared_ptr<Studio::didSetPitchBendFnType> _didSetPitchBendFn;
    std::shared_ptr<Studio::didSetModulationFnType> _didSetModulationFn;
    std::shared_ptr<Studio::didSetControlValueFnType> _didSetControlValueFn;
    std::shared_ptr<Studio::didPerformKeyAftertouchFnType> _didPerformKeyAftertouchFn;
    std::shared_ptr<Studio::didPerformChannelAftertouchFnType> _didPerformChannelAftertouchFn;
    std::shared_ptr<Studio::didSetTempoFnType> _didSetTempoFn;
    std::shared_ptr<Studio::didSetTimeSignatureFnType> _didSetTimeSignatureFn;

    void didSetInstrument(Studio* sender, int source, double timestamp, UInt32 tick, int channel, int instrument);
    void didSetMixerLevel(Studio* sender, int source, double timestamp, UInt32 tick, int channel, Float32 mixerLevel);
    void didSetMixerBalance(Studio* sender, int source, double timestamp, UInt32 tick, int channel,
                            Float32 mixerBalance);
    void didPlayNote(Studio* sender, int source, double timestamp, UInt32 tick, int channel, int pitch,
                     Float32 velocity);
    void didReleaseNote(Studio* sender, int source, double timestamp, UInt32 tick, int channel, int pitch,
                        Float32 velocity);
    void didSetTempo(Studio* sender, int source, double timestamp, UInt32 tick, UInt32 mpqn);
    void didSetTimeSignature(Studio* sender, int source, double timestamp, UInt32 tick, unsigned int numerator,
                             unsigned int denominator);
    void didSetSustain(Studio* sender, int source, double timestamp, UInt32 tick, int channel, Float32 sustain);
    void didSetPitchBend(Studio* sender, int source, double timestamp, UInt32 tick, int channel, Float32 pitchBend);
    void didSetModulation(Studio* sender, int source, double timestamp, UInt32 tick, int channel, Float32 modulation);
    void didSetControlValue(Studio* sender, int source, double timestamp, UInt32 tick, int channel, UInt32 control,
                            UInt32 value);
    void didPerformKeyAftertouch(Studio* sender, int source, double timestamp, UInt32 tick, int channel, int pitch,
                                 Float32 value);
    void didPerformChannelAftertouch(Studio* sender, int source, double timestamp, UInt32 tick, int channel,
                                     Float32 value);

    void reportPlaybackDidFinish();
    void reportPlaybackWasInterrupted();

    bool processMetaEvent(const MDStudio::Event& event);
    void processEvent(const MDStudio::Event& event, bool ignoreNotes, bool isPreloadingOnly = false,
                      bool* isLoadingInstrumentInBackground = nullptr);
    bool metronomeDidTick(Metronome* metronome);
    void metronomeDidMoveTick(Metronome* metronome);

    void addEvent(int source, UInt8 type, Float64 timestamp, UInt32 tick, UInt8 channel, SInt32 param1, SInt32 param2);

    void preloadInstruments(bool* isLoadingInstrumentInBackground);
    void programMetronome();
    void resumePlayback();

    void removeEventsWithoutActivity(int trackIndex);

    void updateCurrentSequence();

   public:
    Sequencer(void* owner, Studio* studio);
    ~Sequencer();

    void* owner() { return _owner; }

    Studio* studio() { return _studio; }

    void resetPlayer();

    void play();
    void playFromCurrentLocation();
    void pause();
    void stop();
    void record();

    void playAudioExport();

    bool isRecording() { return _isRecording; }
    bool isPlaying() { return _isPlaying; }
    bool isPaused() { return _isPaused; }

    std::shared_ptr<MDStudio::Sequence> sequence();
    void setSequence(std::shared_ptr<MDStudio::Sequence> sequence);

    void setArmedTrackIndices(std::vector<int> armedTrackIndices) { _armedTrackIndices = armedTrackIndices; }
    std::vector<int>& armedTrackIndices() { return _armedTrackIndices; }

    void setMuteTrackIndices(std::vector<int> muteTrackIndices);
    std::vector<int>& muteTrackIndices() { return _muteTrackIndices; }

    void setSoloTrackIndex(int soloTrackIndex);
    int soloTrackIndex() { return _soloTrackIndex; }

    void setIsLastNoteOffDetected(bool isLastNoteOffDetected) { _isLastNoteOffDetected = isLastNoteOffDetected; }

    void addPlaybackDidStartFn(std::shared_ptr<playbackDidStartFnType> playbackDidStartFn);
    void removePlaybackDidStartFn(std::shared_ptr<playbackDidStartFnType> playbackDidStartFn);
    void addPlaybackDidFinishFn(std::shared_ptr<playbackDidFinishFnType> playbackDidFinishFn);
    void removePlaybackDidFinishFn(std::shared_ptr<playbackDidFinishFnType> playbackDidFinishFn);
    void addPlaybackWasInterruptedFn(std::shared_ptr<playbackWasInterruptedFnType> playbackWasInterruptedFn);
    void removePlaybackWasInterruptedFn(std::shared_ptr<playbackWasInterruptedFnType> playbackWasInterruptedFn);
    void addPlaybackDidPauseFn(std::shared_ptr<playbackDidPauseFnType> playbackDidPauseFn);
    void removePlaybackDidPauseFn(std::shared_ptr<playbackDidPauseFnType> playbackDidPauseFn);
    void addPlaybackDidResumeFn(std::shared_ptr<playbackDidResumeFnType> playbackDidResumeFn);
    void removePlaybackDidResumeFn(std::shared_ptr<playbackDidResumeFnType> playbackDidResumeFn);
    void addRecordDidStartFn(std::shared_ptr<recordDidStartFnType> recordDidStartFn);
    void removeRecordDidStartFn(std::shared_ptr<recordDidStartFnType> recordDidStartFn);
    void addRecordDidPauseFn(std::shared_ptr<recordDidPauseFnType> recordDidPauseFn);
    void removeRecordDidPauseFn(std::shared_ptr<recordDidPauseFnType> recordDidPauseFn);
    void addRecordDidResumeFn(std::shared_ptr<recordDidResumeFnType> recordDidResumeFn);
    void removeRecordDidResumeFn(std::shared_ptr<recordDidResumeFnType> recordDidResumeFn);
    void addRecordDidFinishFn(std::shared_ptr<recordDidFinishFnType> recordDidFinishFn);
    void removeRecordDidFinishFn(std::shared_ptr<recordDidFinishFnType> recordDidFinishFn);
    void addSequenceAssignedFn(std::shared_ptr<sequenceAssignedFnType> sequenceAssignedFn);
    void removeSequenceAssignedFn(std::shared_ptr<sequenceAssignedFnType> sequenceAssignedFn);
};

}  // namespace MDStudio

#endif  // SEQUENCER_H
