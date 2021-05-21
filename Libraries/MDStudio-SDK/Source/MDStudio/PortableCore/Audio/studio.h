//
//  studio.h
//  MDStudio
//
//  Created by Daniel Cliche on 2010-08-17.
//  Copyright (c) 2010-2021 Daniel Cliche. All rights reserved.
//

#ifndef STUDIO_H
#define STUDIO_H

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "audioplayerunit.h"
#include "instrumentmanager.h"
#include "metronome.h"
#include "mixer.h"
#include "samplerunit.h"

#define STUDIO_MAX_CHANNELS 16

#define STUDIO_INSTRUMENT_FROM_PRESET(_bank_, _number_) ((_bank_ << 8) | (_number_))
#define STUDIO_PRESET_BANK_FROM_INSTRUMENT(_instrument_) (_instrument_ >> 8)
#define STUDIO_PRESET_NUMBER_FROM_INSTRUMENT(_instrument_) (_instrument_ & 0xff)

#define STUDIO_INSTRUMENT_NONE -1

#define STUDIO_INSTRUMENT_GM_PIANO STUDIO_INSTRUMENT_FROM_PRESET(0, 0)
#define STUDIO_INSTRUMENT_GM_BASS STUDIO_INSTRUMENT_FROM_PRESET(0, 32)
#define STUDIO_INSTRUMENT_GM_STRINGS STUDIO_INSTRUMENT_FROM_PRESET(0, 48)
#define STUDIO_INSTRUMENT_GM_HARPSICHORD STUDIO_INSTRUMENT_FROM_PRESET(0, 6)

#define STUDIO_INSTRUMENT_GM_STANDARD_DRUM_KIT STUDIO_INSTRUMENT_FROM_PRESET(128, 0)

#define STUDIO_CONTROL_DATA_ENTRY_MSB 6   // Coarse
#define STUDIO_CONTROL_DATA_ENTRY_LSB 38  // Fine
#define STUDIO_CONTROL_EXPRESSION_CTRL 11
#define STUDIO_CONTROL_REVERB 91
#define STUDIO_CONTROL_CHORUS 93
#define STUDIO_CONTROL_NON_REG_PARAM_NUM_LSB 98  // Fine
#define STUDIO_CONTROL_NON_REG_PARAM_NUM_MSB 99  // Coarse
#define STUDIO_CONTROL_REG_PARAM_NUM_LSB 100     // Fine
#define STUDIO_CONTROL_REG_PARAM_NUM_MSB 101     // Coarse
#define STUDIO_CONTROL_ALL_SOUND_OFF 120
#define STUDIO_CONTROL_ALL_NOTES_OFF 123

#define STUDIO_SOURCE_USER 0
#define STUDIO_SOURCE_SEQUENCER 1

namespace MDStudio {

class Studio {
   public:
    typedef std::function<void(Studio* sender, int source, double timestamp, UInt32 tick, int channel, int instrument)>
        didSetInstrumentFnType;
    typedef std::function<void(Studio* sender, int source, double timestamp, UInt32 tick, int channel,
                               Float32 mixerLevel)>
        didSetMixerLevelFnType;
    typedef std::function<void(Studio* sender, int source, double timestamp, UInt32 tick, int channel,
                               Float32 mixerBalance)>
        didSetMixerBalanceFnType;
    typedef std::function<void(Studio* sender, int source, double timestamp, UInt32 tick, int channel, int pitch,
                               Float32 velocity)>
        didPlayNoteFnType;
    typedef std::function<void(Studio* sender, int source, double timestamp, UInt32 tick, int channel, int pitch,
                               Float32 velocity)>
        didReleaseNoteFnType;
    typedef std::function<void(Studio* sender, int source, double timestamp, UInt32 tick, UInt32 mpqn)>
        didSetTempoFnType;
    typedef std::function<void(Studio* sender, int source, double timestamp, UInt32 tick, unsigned int numerator,
                               unsigned int denominator)>
        didSetTimeSignatureFnType;
    typedef std::function<void(Studio* sender, int source, double timestamp, UInt32 tick, int channel, Float32 sustain)>
        didSetSustainFnType;
    typedef std::function<void(Studio* sender, int source, double timestamp, UInt32 tick, int channel,
                               Float32 pitchBend)>
        didSetPitchBendFnType;
    typedef std::function<void(Studio* sender, int source, double timestamp, UInt32 tick, int channel,
                               Float32 modulation)>
        didSetModulationFnType;
    typedef std::function<void(Studio* sender, int source, double timestamp, UInt32 tick, int channel, UInt32 control,
                               UInt32 value)>
        didSetControlValueFnType;
    typedef std::function<void(Studio* sender, int source, double timestamp, UInt32 tick, int channel, int pitch,
                               Float32 value)>
        didPerformKeyAftertouchFnType;
    typedef std::function<void(Studio* sender, int source, double timestamp, UInt32 tick, int channel, Float32 value)>
        didPerformChannelAftertouchFnType;
    typedef std::function<void(Studio* sender, int source, double timestamp, UInt32 tick, int channel,
                               std::vector<UInt8> data)>
        didSendSystemExclusiveFnType;
    typedef std::function<void(Studio* sender, bool state)> didSetStudioLoadingStateFnType;

   private:
    int _instruments[STUDIO_MAX_CHANNELS];
    Float32 _mixerLevelValues[STUDIO_MAX_CHANNELS];
    Float32 _mixerBalanceValues[STUDIO_MAX_CHANNELS];
    Float32 _sustainValues[STUDIO_MAX_CHANNELS];
    Float32 _pitchBendValues[STUDIO_MAX_CHANNELS];
    Float32 _modulationValues[STUDIO_MAX_CHANNELS];
    UInt32 _controlValues[STUDIO_MAX_CHANNELS][128];
    Float32 _pitchBendFactorValues[STUDIO_MAX_CHANNELS];

    Mixer* _mixer;
    InstrumentManager* _instrumentManager;
    std::shared_ptr<SamplerUnit> _sampler;
    std::shared_ptr<SamplerUnit> _metronomeSampler;
    std::shared_ptr<AudioPlayerUnit> _audioPlayer;
    Metronome* _metronome;
    std::shared_ptr<MultiInstrument> _multiInstruments[STUDIO_MAX_CHANNELS];
    std::shared_ptr<MultiInstrument> _gmInstruments[256][256];
    std::shared_ptr<MultiInstrument> _metronomeInstrument;
    bool _playingNotes[STUDIO_MAX_CHANNELS][128];
    std::vector<Preset> _presets;

    std::vector<std::shared_ptr<didSetInstrumentFnType>> _didSetInstrumentFns;
    std::vector<std::shared_ptr<didSetMixerLevelFnType>> _didSetMixerLevelFns;
    std::vector<std::shared_ptr<didSetMixerBalanceFnType>> _didSetMixerBalanceFns;
    std::vector<std::shared_ptr<didPlayNoteFnType>> _didPlayNoteFns;
    std::vector<std::shared_ptr<didReleaseNoteFnType>> _didReleaseNoteFns;
    std::vector<std::shared_ptr<didSetTempoFnType>> _didSetTempoFns;
    std::vector<std::shared_ptr<didSetTimeSignatureFnType>> _didSetTimeSignatureFns;
    std::vector<std::shared_ptr<didSetSustainFnType>> _didSetSustainFns;
    std::vector<std::shared_ptr<didSetPitchBendFnType>> _didSetPitchBendFns;
    std::vector<std::shared_ptr<didSetModulationFnType>> _didSetModulationFns;
    std::vector<std::shared_ptr<didSetControlValueFnType>> _didSetControlValueFns;
    std::vector<std::shared_ptr<didPerformKeyAftertouchFnType>> _didPerformKeyAftertouchFns;
    std::vector<std::shared_ptr<didPerformChannelAftertouchFnType>> _didPerformChannelAftertouchFns;
    std::vector<std::shared_ptr<didSetStudioLoadingStateFnType>> _didSetStudioLoadingStateFns;

    // MIDI output callbacks
    didSetInstrumentFnType _midiOutputDidSetInstrumentFn;
    didSetMixerLevelFnType _midiOutputDidSetMixerLevelFn;
    didSetMixerBalanceFnType _midiOutputDidSetMixerBalanceFn;
    didPlayNoteFnType _midiOutputDidPlayNoteFn;
    didReleaseNoteFnType _midiOutputDidReleaseNoteFn;
    didSetSustainFnType _midiOutputDidSetSustainFn;
    didSetPitchBendFnType _midiOutputDidSetPitchBendFn;
    didSetModulationFnType _midiOutputDidSetModulationFn;
    didSetControlValueFnType _midiOutputDidSetControlValueFn;
    didPerformKeyAftertouchFnType _midiOutputDidPerformKeyAftertouchFn;
    didPerformChannelAftertouchFnType _midiOutputDidPerformChannelAftertouchFn;
    didSendSystemExclusiveFnType _midiOutputDidSendSystemExclusiveFn;

    int _nbChannels;
    std::mutex _playMutex, _loadInstrumentMutex;

    bool _isNonRegisteredDataEntry;

    double getTimestamp();

    void sendMixerLevel(int source, int channel);
    void sendMixerBalance(int source, int channel);
    void sendInstrument(int source, int channel);
    void sendTempo(int source);
    void sendTimeSignature(int source);
    void sendSustain(int source, int channel);
    void sendPitchBend(int source, int channel);
    void sendModulation(int source, int channel);
    void sendControlValue(int source, UInt32 control, UInt32 value, int channel);
    void sendKeyAftertouch(int source, int pitch, Float32 value, int channel);
    void sendChannelAftertouch(int source, Float32 value, int channel);

    void metronomePlayMajorTick(Metronome* metronome);
    void metronomePlayMinorTick(Metronome* metronome);

    void setIsLoading(bool isLoading);

    bool _areStateNotificationsEnabled;

    unsigned _loadingCounter;

    bool _areInstrumentsLoadedInBackground;

    bool _isInternalSynthEnabled;
    bool _isMIDIOutputEnabled;

   public:
    Studio(int nbChannels, std::string sf2Path, UInt32 nbVoices = SAMPLER_MAX_VOICES);
    ~Studio();

    void setNbVoices(UInt32 nbVoices);

    void setAreInstrumentsLoadedInBackground(bool areInstrumentsLoadedInBackground) {
        _areInstrumentsLoadedInBackground = areInstrumentsLoadedInBackground;
    }
    bool areInstrumentsLoadedInBackground() { return _areInstrumentsLoadedInBackground; }

    Mixer* mixer() { return _mixer; }
    Metronome* metronome() { return _metronome; }

    void addDidSetInstrumentFn(std::shared_ptr<didSetInstrumentFnType> didSetInstrumentFn);
    void removeDidSetInstrumentFn(std::shared_ptr<didSetInstrumentFnType> didSetInstrumentFn);
    void addDidSetMixerLevelFn(std::shared_ptr<didSetMixerLevelFnType> didSetMixerLevelFn);
    void removeDidSetMixerLevelFn(std::shared_ptr<didSetMixerLevelFnType> didSetMixerLevelFn);
    void addDidSetMixerBalanceFn(std::shared_ptr<didSetMixerBalanceFnType> didSetMixerBalanceFn);
    void removeDidSetMixerBalanceFn(std::shared_ptr<didSetMixerBalanceFnType> didSetMixerBalanceFn);

    void addDidPlayNoteFn(std::shared_ptr<didPlayNoteFnType> didPlayNoteFn);
    void removeDidPlayNoteFn(std::shared_ptr<didPlayNoteFnType> didPlayNoteFn);
    void addDidReleaseNoteFn(std::shared_ptr<didReleaseNoteFnType> didReleaseNoteFn);
    void removeDidReleaseNoteFn(std::shared_ptr<didReleaseNoteFnType> didReleaseNoteFn);

    void addDidSetTempoFn(std::shared_ptr<didSetTempoFnType> didSetTempoFn);
    void removeDidSetTempoFn(std::shared_ptr<didSetTempoFnType> didSetTempoFn);
    void addDidSetTimeSignatureFn(std::shared_ptr<didSetTimeSignatureFnType> didSetTimeSignatureFn);
    void removeDidSetTimeSignatureFn(std::shared_ptr<didSetTimeSignatureFnType> didSetTimeSignatureFn);
    void addDidSetSustainFn(std::shared_ptr<didSetSustainFnType> didSetSustainFn);
    void removeDidSetSustainFn(std::shared_ptr<didSetSustainFnType> didSetSustainFn);
    void addDidSetPitchBendFn(std::shared_ptr<didSetPitchBendFnType> didSetPitchBendFn);
    void removeDidSetPitchBendFn(std::shared_ptr<didSetPitchBendFnType> didSetPitchBendFn);
    void addDidSetModulationFn(std::shared_ptr<didSetModulationFnType> didSetModulationFn);
    void removeDidSetModulationFn(std::shared_ptr<didSetModulationFnType> didSetModulationFn);
    void addDidSetControlValueFn(std::shared_ptr<didSetControlValueFnType> didSetControlValueFn);
    void removeDidSetControlValueFn(std::shared_ptr<didSetControlValueFnType> didSetControlValueFn);
    void addDidPerformKeyAftertouchFn(std::shared_ptr<didPerformKeyAftertouchFnType> didPerformKeyAftertouchFn);
    void removeDidPerformKeyAftertouchFn(std::shared_ptr<didPerformKeyAftertouchFnType> didPerformKeyAftertouchFn);
    void addDidPerformChannelAftertouchFn(
        std::shared_ptr<didPerformChannelAftertouchFnType> didPerformChannelAftertouchFn);
    void removeDidPerformChannelAftertouchFn(
        std::shared_ptr<didPerformChannelAftertouchFnType> didPerformChannelAftertouchFn);

    void addDidSetStudioLoadingStateFn(std::shared_ptr<didSetStudioLoadingStateFnType> didSetStudioLoadingStateFn);
    void removeDidSetStudioLoadingStateFn(std::shared_ptr<didSetStudioLoadingStateFnType> didSetStudioLoadingStateFn);

    // MIDI output callbacks
    // Warning: not called from the main thread
    void setMIDIOutputDidSetInstrumentFn(didSetInstrumentFnType midiOutputDidSetInstrumentFn) {
        _midiOutputDidSetInstrumentFn = midiOutputDidSetInstrumentFn;
    }
    void setMIDIOutputDidSetMixerLevelFn(didSetMixerLevelFnType midiOutputDidSetMixerLevelFn) {
        _midiOutputDidSetMixerLevelFn = midiOutputDidSetMixerLevelFn;
    }
    void setMIDIOutputDidSetMixerBalanceFn(didSetMixerBalanceFnType midiOutputDidSetMixerBalanceFn) {
        _midiOutputDidSetMixerBalanceFn = midiOutputDidSetMixerBalanceFn;
    }
    void setMIDIOutputDidPlayNoteFn(didPlayNoteFnType midiOutputDidPlayNoteFn) {
        _midiOutputDidPlayNoteFn = midiOutputDidPlayNoteFn;
    }
    void setMIDIOutputDidReleaseNoteFn(didReleaseNoteFnType midiOutputDidReleaseNoteFn) {
        _midiOutputDidReleaseNoteFn = midiOutputDidReleaseNoteFn;
    }
    void setMIDIOutputDidSetSustainFn(didSetSustainFnType midiOutputDidSetSustainFn) {
        _midiOutputDidSetSustainFn = midiOutputDidSetSustainFn;
    }
    void setMIDIOutputDidSetPitchBendFn(didSetPitchBendFnType midiOutputDidSetPitchBendFn) {
        _midiOutputDidSetPitchBendFn = midiOutputDidSetPitchBendFn;
    }
    void setMIDIOutputDidSetModulationFn(didSetModulationFnType midiOutputDidSetModulationFn) {
        _midiOutputDidSetModulationFn = midiOutputDidSetModulationFn;
    }
    void setMIDIOutputDidSetControlValueFn(didSetControlValueFnType midiOutputDidSetControlValueFn) {
        _midiOutputDidSetControlValueFn = midiOutputDidSetControlValueFn;
    }
    void setMIDIOutputDidPerformKeyAftertouchFn(didPerformKeyAftertouchFnType midiOutputDidPerformKeyAftertouchFn) {
        _midiOutputDidPerformKeyAftertouchFn = midiOutputDidPerformKeyAftertouchFn;
    }
    void setMIDIOutputDidPerformChannelAftertouchFn(
        didPerformChannelAftertouchFnType midiOutputDidPerformChannelAftertouchFn) {
        _midiOutputDidPerformChannelAftertouchFn = midiOutputDidPerformChannelAftertouchFn;
    }
    void setMIDIOutputDidSendSystemExclusiveFn(didSendSystemExclusiveFnType midiOutputDidSendSystemExclusiveFn) {
        _midiOutputDidSendSystemExclusiveFn = midiOutputDidSendSystemExclusiveFn;
    }

    // Instrument management (thread safe)

    void setInstrument(int source, int instrument, int channel, bool isPreloadingOnly = false,
                       bool* isLoadingInBackground = nullptr);
    int instrument(int channel);
    void clearInstrumentCache();

    // Play (thread safe)

    void playNote(int source, int pitch, Float32 velocity, int channel);
    void releaseNote(int source, int pitch, Float32 velocity, int channel);
    void keyAftertouch(int source, int pitch, Float32 velocity, int channel);
    void channelAftertouch(int source, Float32 velocity, int channel);
    void setSustain(int source, Float32 sustain, int channel);
    Float32 sustain(int channel);
    void setPitchBend(int source, Float32 pitchBend, int channel);
    Float32 pitchBend(int channel);
    void setModulation(int source, Float32 modulation, int channel);
    Float32 modulation(int channel);

    void setMixerLevel(int source, Float32 level, int channel);
    Float32 mixerLevel(int channel);
    void setMixerBalance(int source, Float32 balance, int channel);
    Float32 mixerBalance(int channel);

    void setMasterMixerLevel(int source, Float32 level);
    Float32 masterMixerLevel();

    void setControlValue(int source, UInt32 control, UInt32 value, int channel);
    UInt32 controlValue(int channel, UInt32 control);

    Float32 pitchBendFactor(int channel);

    void sendSystemExclusive(int source, std::vector<UInt8> data, int channel);

    void stopNotes(int source, int channel);
    void clearVoices(int channel);
    void stopAllNotes(int source, bool areVoicesCleared = false);

    bool isNotePlaying(int channel);

    // Mixer
    std::vector<std::pair<std::string, double>> audioOutputDevices() { return _mixer->outputDevices(); }
    std::pair<std::string, double> audioOutputDevice() {
        return std::make_pair(_mixer->outputDeviceName(), _mixer->outputLatency());
    }
    void setAudioOutputDevice(const std::string& audioOutputDeviceName, double latency);
    void startMixer();
    void stopMixer();

    const Preset* presetForInstrument(int instrument);

    std::vector<Preset> presets() { return _presets; }

    void setMetronomeTimeDivision(unsigned int timeDivision);
    void setMetronomeBPMs(int source, std::vector<std::pair<UInt32, unsigned int>> bpms);
    void setMetronomeTimeSignatures(int source,
                                    std::vector<std::pair<UInt32, std::pair<UInt32, UInt32>>> timeSignatures);

    bool isMetronomeRunning();
    void startMetronome();
    void stopMetronome();
    void resetMetronome();
    void moveMetronomeToTick(UInt32 tick);

    void enableStateNotifications() { _areStateNotificationsEnabled = true; }
    void disableStateNotifications() { _areStateNotificationsEnabled = false; }
    void dumpStates(int source);

    void playAudioFile(std::string path, Float32 level);

    void setIsInternalSynthEnabled(bool isInternalSynthEnabled) {
        if (_isInternalSynthEnabled && !isInternalSynthEnabled) _sampler->clearAllVoices();
        _isInternalSynthEnabled = isInternalSynthEnabled;
    }
    bool isInternalSynthEnabled() { return _isInternalSynthEnabled; }

    void setIsMIDIOutputEnabled(bool isMIDIOutputEnabled) { _isMIDIOutputEnabled = isMIDIOutputEnabled; }
    bool isMIDIOutputEnabled() { return _isMIDIOutputEnabled; }
};

}  // namespace MDStudio

#endif  // STUDIO_H
