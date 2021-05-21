//
//  studio.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2010-08-17.
//  Copyright (c) 2010-2021 Daniel Cliche. All rights reserved.
//

#include "studio.h"

#include <algorithm>
#include <chrono>
#include <string>
#include <thread>

#include "platform.h"

// Channel of the metronome
#define STUDIO_METRONOME_CHANNEL 0

// GM bank file name
#define STUDIO_GM_SF2_FILENAME "GM"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
Studio::Studio(int nbChannels, std::string sf2Path, UInt32 nbVoices) {
    using namespace std::placeholders;

    // Initialize the MIDI output callbacks
    _midiOutputDidSetInstrumentFn = nullptr;
    _midiOutputDidSetMixerLevelFn = nullptr;
    _midiOutputDidSetMixerBalanceFn = nullptr;
    _midiOutputDidPlayNoteFn = nullptr;
    _midiOutputDidReleaseNoteFn = nullptr;
    _midiOutputDidSetSustainFn = nullptr;
    _midiOutputDidSetPitchBendFn = nullptr;
    _midiOutputDidSetModulationFn = nullptr;
    _midiOutputDidSetControlValueFn = nullptr;
    _midiOutputDidPerformKeyAftertouchFn = nullptr;
    _midiOutputDidPerformChannelAftertouchFn = nullptr;
    _midiOutputDidSendSystemExclusiveFn = nullptr;

    if (nbChannels > STUDIO_MAX_CHANNELS) nbChannels = STUDIO_MAX_CHANNELS;

    _nbChannels = nbChannels;

    _areInstrumentsLoadedInBackground = false;

    _isNonRegisteredDataEntry = false;

    // We initialize the playing notes
    memset(_playingNotes, 0, sizeof(_playingNotes));

    memset(_multiInstruments, 0, sizeof(_multiInstruments));
    memset(_gmInstruments, 0, sizeof(_gmInstruments));

    // Create the instrument manager
    _instrumentManager = new InstrumentManager(sf2Path);

    // We create the metronome instrument
    _metronomeInstrument = _instrumentManager->loadSF2MultiInstrument(STUDIO_GM_SF2_FILENAME, 128, 0);

    // Create the sampler (a single synthesizer is used for both channels)
    _sampler = std::make_shared<SamplerUnit>(nbVoices);

    // Create the audio player
    _audioPlayer = std::make_shared<AudioPlayerUnit>();

    // Create the metronome sampler
    _metronomeSampler = std::make_shared<SamplerUnit>(1);
    _metronomeSampler->setReverb(0.0f, STUDIO_METRONOME_CHANNEL);

    // Create our mixer with graph
    _mixer = new Mixer();
    _mixer->addUnit(_sampler);
    _mixer->addUnit(_audioPlayer);
    _mixer->addUnit(_metronomeSampler);

    // Create a metronome
    _metronome = new Metronome();
    _metronome->setPlayMajorTickFn(std::bind(&Studio::metronomePlayMajorTick, this, _1));
    _metronome->setPlayMinorTickFn(std::bind(&Studio::metronomePlayMinorTick, this, _1));
    _metronome->setBPMDidChangeFn([this](Metronome* sender, unsigned int bpm) { sendTempo(STUDIO_SOURCE_USER); });
    _metronome->setTimeSignatureDidChangeFn([this](Metronome* sender, std::pair<UInt32, UInt32> timeSignature) { sendTimeSignature(STUDIO_SOURCE_USER); });

    // Get the presets
    _presets = _instrumentManager->getSF2Presets(STUDIO_GM_SF2_FILENAME);

    // Initialize the synth states
    for (int i = 0; i < STUDIO_MAX_CHANNELS; ++i) {
        _instruments[i] = STUDIO_INSTRUMENT_NONE;
        _mixerBalanceValues[i] = _sampler->balance(i);
        _mixerLevelValues[i] = _sampler->level(i);
        _sustainValues[i] = _sampler->sustain(i);
        _pitchBendValues[i] = _sampler->pitchBend(i);
        _modulationValues[i] = _sampler->modulation(i);
        for (int j = 0; j < 128; ++j) {
            if (j == STUDIO_CONTROL_REVERB) {
                _controlValues[i][j] = _sampler->reverb(i) * 127.0f;
            } else if (j == STUDIO_CONTROL_CHORUS) {
                _controlValues[i][j] = _sampler->chorus(i) * 127.0f;
            } else if (j == STUDIO_CONTROL_EXPRESSION_CTRL) {
                _controlValues[i][j] = _sampler->expression(i) * 127.0f;
            } else {
                _controlValues[i][j] = 0;
            }
        }
        _pitchBendFactorValues[i] = _sampler->pitchBendFactor(i);
    }

    _areStateNotificationsEnabled = true;

    _loadingCounter = 0;

    _isInternalSynthEnabled = true;
    _isMIDIOutputEnabled = false;
}

// ---------------------------------------------------------------------------------------------------------------------
Studio::~Studio() {
    stopMetronome();

    _mixer->stop();

    delete _metronome;
    delete _mixer;
    delete _instrumentManager;
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::setNbVoices(UInt32 nbVoices) {
    _playMutex.lock();
    auto wasRunning = _mixer->isRunning();
    _mixer->stop();
    _sampler->setNbVoices(nbVoices);
    if (wasRunning) _mixer->start();
    _playMutex.unlock();
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::setIsLoading(bool isLoading) {
    if (isLoading) {
        if (_loadingCounter == 0) {
            for (auto f : _didSetStudioLoadingStateFns) {
                (*f)(this, true);
            }
        }
        _loadingCounter++;
    } else {
        _loadingCounter--;
        if (_loadingCounter == 0) {
            for (auto f : _didSetStudioLoadingStateFns) {
                (*f)(this, false);
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::setInstrument(int source, int instrument, int channel, bool isPreloadingOnly,
                           bool* isLoadingInBackground) {
    if (channel >= _nbChannels) return;

    _playMutex.lock();

    if (_isInternalSynthEnabled) {
        std::shared_ptr<MultiInstrument> multiInstrument = nullptr;

        int bank = STUDIO_PRESET_BANK_FROM_INSTRUMENT(instrument);
        int number = STUDIO_PRESET_NUMBER_FROM_INSTRUMENT(instrument);

        multiInstrument = _gmInstruments[bank][number];

        if (multiInstrument == nullptr) {
            if (_areInstrumentsLoadedInBackground) {
                if (isLoadingInBackground) *isLoadingInBackground = true;
                std::thread([=] {
                    Platform::sharedInstance()->invoke([=]() { setIsLoading(true); });
                    _loadInstrumentMutex.lock();
                    std::shared_ptr<MultiInstrument> mi =
                        _instrumentManager->loadSF2MultiInstrument(STUDIO_GM_SF2_FILENAME, bank, number);
                    _loadInstrumentMutex.unlock();
                    _playMutex.lock();
                    _gmInstruments[bank][number] = mi;
                    if (!isPreloadingOnly) _multiInstruments[channel] = mi;
                    _playMutex.unlock();
                    Platform::sharedInstance()->invoke([=]() { setIsLoading(false); });
                }).detach();
            } else {
                multiInstrument = _gmInstruments[bank][number] =
                    _instrumentManager->loadSF2MultiInstrument(STUDIO_GM_SF2_FILENAME, bank, number);
                if (!isPreloadingOnly) {
                    // We load the instrument
                    _multiInstruments[channel] = multiInstrument;
                }
            }
        } else {
            if (!isPreloadingOnly) _multiInstruments[channel] = multiInstrument;
        }
    }  // if internal synth is enabled

    if (!isPreloadingOnly) _instruments[channel] = instrument;

    _playMutex.unlock();

    if (!isPreloadingOnly) {
        // Send the instrument
        sendInstrument(source, channel);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
int Studio::instrument(int channel) {
    if (channel >= _nbChannels) return STUDIO_INSTRUMENT_NONE;

    int ins;

    _playMutex.lock();

    ins = _instruments[channel];

    _playMutex.unlock();

    return ins;
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::clearInstrumentCache() {
    _playMutex.lock();

    for (int bank = 0; bank < 256; ++bank)
        for (int instrument = 0; instrument < 256; ++instrument) {
            auto multiInstrument = _gmInstruments[bank][instrument];
            if (multiInstrument != nullptr) {
                // Check if the instrument is currently in use
                bool isFound = false;
                for (int i = 0; i < STUDIO_MAX_CHANNELS; ++i)
                    if (_multiInstruments[i] == multiInstrument) {
                        isFound = true;
                        break;
                    }

                // If the instrument is not currently in use, we remove it from the cache
                if (!isFound) {
                    _instrumentManager->unloadMultiInstrument(_gmInstruments[bank][instrument]);
                    _gmInstruments[bank][instrument] = nullptr;
                }
            }
        }

    _playMutex.unlock();
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::playNote(int source, int pitch, Float32 velocity, int channel) {
    if (channel >= _nbChannels) return;

    _playMutex.lock();
    if (_isInternalSynthEnabled)
        _sampler->playNote(static_cast<Float32>(pitch), velocity, _multiInstruments[channel], channel);
    _playingNotes[channel][pitch] = true;
    _playMutex.unlock();

    UInt32 tick = _metronome->tick();

    Platform::sharedInstance()->invoke([=]() {
        for (auto didPlayNote : _didPlayNoteFns) {
            (*didPlayNote)(this, source, getTimestamp(), tick, channel, pitch, velocity);
        }
    });

    if (_isMIDIOutputEnabled && _midiOutputDidPlayNoteFn)
        _midiOutputDidPlayNoteFn(this, source, getTimestamp(), tick, channel, pitch, velocity);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::releaseNote(int source, int pitch, Float32 velocity, int channel) {
    if (channel >= _nbChannels) return;

    _playMutex.lock();
    if (_isInternalSynthEnabled) _sampler->releaseNote(static_cast<Float32>(pitch), channel);
    _playingNotes[channel][pitch] = false;
    _playMutex.unlock();

    UInt32 tick = _metronome->tick();

    Platform::sharedInstance()->invoke([=]() {
        for (auto didReleaseNote : _didReleaseNoteFns) {
            (*didReleaseNote)(this, source, getTimestamp(), tick, channel, pitch, velocity);
        }
    });

    if (_isMIDIOutputEnabled && _midiOutputDidReleaseNoteFn)
        _midiOutputDidReleaseNoteFn(this, source, getTimestamp(), tick, channel, pitch, velocity);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::keyAftertouch(int source, int pitch, Float32 value, int channel) {
    if (channel > _nbChannels) return;

    _playMutex.lock();
    sendKeyAftertouch(source, pitch, value, channel);
    _playMutex.unlock();
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::channelAftertouch(int source, Float32 value, int channel) {
    if (channel > _nbChannels) return;

    _playMutex.lock();
    sendChannelAftertouch(source, value, channel);
    _playMutex.unlock();
}

// ---------------------------------------------------------------------------------------------------------------------
// The notifications are sent immediately if the call is performed from the main thread.
void Studio::stopNotes(int source, int channel) {
    std::vector<std::function<void()>> invokeFns;

    _playMutex.lock();

    // For each pitch
    for (int pitch = 0; pitch < 128; pitch++) {
        if (_playingNotes[channel][pitch]) {
            _playingNotes[channel][pitch] = false;

            UInt32 tick = _metronome->tick();

            // We send the notification
            invokeFns.push_back([=]() {
                for (auto didReleaseNote : _didReleaseNoteFns)
                    (*didReleaseNote)(this, source, getTimestamp(), tick, channel, pitch, 1.0f);
            });

            if (_isMIDIOutputEnabled && _midiOutputDidReleaseNoteFn)
                _midiOutputDidReleaseNoteFn(this, source, getTimestamp(), tick, channel, pitch, 1.0f);
        }
    }

    // We stop all the notes on the synthesizer for the given channel
    if (_isInternalSynthEnabled) _sampler->stopAllNotes(channel);

    _playMutex.unlock();

    for (auto invokeFn : invokeFns) Platform::sharedInstance()->invoke(invokeFn);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::clearVoices(int channel) {
    _playMutex.lock();
    if (_isInternalSynthEnabled) _sampler->clearVoices(channel);
    _playMutex.unlock();
}

// ---------------------------------------------------------------------------------------------------------------------
// Stop all the notes.
void Studio::stopAllNotes(int source, bool areVoicesCleared) {
    // For each channel
    for (int channel = 0; channel < _nbChannels; channel++) {
        stopNotes(source, channel);
        if (areVoicesCleared) clearVoices(channel);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool Studio::isNotePlaying(int channel) {
    for (int pitch = 0; pitch < 128; pitch++)
        if (_playingNotes[channel][pitch]) return true;
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::setSustain(int source, Float32 sustain, int channel) {
    if (channel >= _nbChannels) return;

    _playMutex.lock();
    _sustainValues[channel] = sustain;
    if (_isInternalSynthEnabled) _sampler->setSustain(sustain, channel);
    sendSustain(source, channel);
    _playMutex.unlock();
}

// ---------------------------------------------------------------------------------------------------------------------
Float32 Studio::sustain(int channel) {
    if (channel >= _nbChannels) return false;

    Float32 sustain;

    _playMutex.lock();
    sustain = _sustainValues[channel];
    _playMutex.unlock();

    return sustain;
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::setPitchBend(int source, Float32 pitchBend, int channel) {
    if (channel >= _nbChannels) return;

    _playMutex.lock();
    _pitchBendValues[channel] = pitchBend;
    if (_isInternalSynthEnabled) _sampler->setPitchBend(pitchBend, channel);
    sendPitchBend(source, channel);
    _playMutex.unlock();
}

// ---------------------------------------------------------------------------------------------------------------------
Float32 Studio::pitchBend(int channel) {
    if (channel >= _nbChannels) return 0.0f;

    Float32 pitchBend;
    _playMutex.lock();
    pitchBend = _pitchBendValues[channel];
    _playMutex.unlock();
    return pitchBend;
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::setModulation(int source, Float32 modulation, int channel) {
    if (channel >= _nbChannels) return;

    _playMutex.lock();
    _modulationValues[channel] = modulation;
    if (_isInternalSynthEnabled) _sampler->setModulation(modulation, channel);
    sendModulation(source, channel);
    _playMutex.unlock();
}

// ---------------------------------------------------------------------------------------------------------------------
Float32 Studio::modulation(int channel) {
    if (channel >= _nbChannels) return 0.0f;

    Float32 modulation;
    _playMutex.lock();
    modulation = _modulationValues[channel];
    _playMutex.unlock();
    return modulation;
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::setControlValue(int source, UInt32 control, UInt32 value, int channel) {
    if (channel >= _nbChannels) return;

    _playMutex.lock();
    _controlValues[channel][control] = value;
    switch (control) {
        case STUDIO_CONTROL_REVERB:
            // Reverb
            if (_isInternalSynthEnabled) _sampler->setReverb((Float32)(value / 127.0f), channel);
            break;
        case STUDIO_CONTROL_CHORUS:
            // Chorus
            if (_isInternalSynthEnabled) _sampler->setChorus((Float32)(value / 127.0f), channel);
            break;
        case STUDIO_CONTROL_EXPRESSION_CTRL:
            // Expression control
            if (_isInternalSynthEnabled) _sampler->setExpression((Float32)(value / 127.0f), channel);
            break;

        case STUDIO_CONTROL_REG_PARAM_NUM_MSB:
        case STUDIO_CONTROL_REG_PARAM_NUM_LSB:
            _isNonRegisteredDataEntry = false;
            break;

        case STUDIO_CONTROL_NON_REG_PARAM_NUM_MSB:
        case STUDIO_CONTROL_NON_REG_PARAM_NUM_LSB:
            _isNonRegisteredDataEntry = true;
            break;

        case STUDIO_CONTROL_DATA_ENTRY_MSB:
            if (!_isNonRegisteredDataEntry && _controlValues[channel][STUDIO_CONTROL_REG_PARAM_NUM_MSB] == 0 &&
                _controlValues[channel][STUDIO_CONTROL_REG_PARAM_NUM_LSB] == 0) {
                // Pitch bend range sensitivity MSB
                auto fine = fmodf(_pitchBendFactorValues[channel], 1.0f);
                auto factor = value / 2.0f + fine;
                if (_isInternalSynthEnabled) _sampler->setPitchBendFactor(factor, channel);
                _pitchBendFactorValues[channel] = factor;
            }
            break;
        case STUDIO_CONTROL_DATA_ENTRY_LSB:
            if (!_isNonRegisteredDataEntry && _controlValues[channel][STUDIO_CONTROL_REG_PARAM_NUM_MSB] == 0 &&
                _controlValues[channel][STUDIO_CONTROL_REG_PARAM_NUM_LSB] == 0) {
                // Pitch bend range sensitivity LSB
                auto factor =
                    (floorf(_pitchBendFactorValues[channel] * 2.0f) + static_cast<float>(value) / 100.0f) / 2.0f;
                if (_isInternalSynthEnabled) _sampler->setPitchBendFactor(factor, channel);
                _pitchBendFactorValues[channel] = factor;
            }
            break;
        case STUDIO_CONTROL_ALL_SOUND_OFF:
            _playMutex.unlock();
            stopNotes(source, channel);
            clearVoices(channel);
            _playMutex.lock();
            break;
        case STUDIO_CONTROL_ALL_NOTES_OFF:
            _playMutex.unlock();
            stopNotes(source, channel);
            _playMutex.lock();
            break;
    }

    sendControlValue(source, control, value, channel);
    _playMutex.unlock();
}

// ---------------------------------------------------------------------------------------------------------------------
UInt32 Studio::controlValue(int channel, UInt32 control) {
    if (channel >= _nbChannels) return 0;

    UInt32 value;
    _playMutex.lock();
    value = _controlValues[channel][control];
    _playMutex.unlock();
    return value;
}

// ---------------------------------------------------------------------------------------------------------------------
Float32 Studio::pitchBendFactor(int channel) {
    if (channel >= _nbChannels) return 0.0f;

    Float32 pitchBendFactor;
    _playMutex.lock();
    pitchBendFactor = _pitchBendFactorValues[channel];
    _playMutex.unlock();
    return pitchBendFactor;
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::sendSystemExclusive(int source, std::vector<UInt8> data, int channel) {
    if (channel >= _nbChannels) return;

    _playMutex.lock();

    UInt32 tick = _metronome->tick();

    if (_areStateNotificationsEnabled && _isMIDIOutputEnabled && _midiOutputDidSetControlValueFn)
        _midiOutputDidSendSystemExclusiveFn(this, source, getTimestamp(), tick, channel, data);

    _playMutex.unlock();
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::setMixerLevel(int source, Float32 level, int channel) {
    if (channel >= _nbChannels) return;

    _playMutex.lock();

    if (_isInternalSynthEnabled) _sampler->setLevel(level, channel);

    _mixerLevelValues[channel] = level;

    _playMutex.unlock();

    sendMixerLevel(source, channel);
}

// ---------------------------------------------------------------------------------------------------------------------
Float32 Studio::mixerLevel(int channel) {
    if (channel >= _nbChannels) return 0.0f;

    Float32 level;

    _playMutex.lock();
    level = _mixerLevelValues[channel];
    _playMutex.unlock();

    return level;
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::setMixerBalance(int source, Float32 balance, int channel) {
    if (channel >= _nbChannels) return;

    _playMutex.lock();

    if (_isInternalSynthEnabled) _sampler->setBalance(balance, channel);

    _mixerBalanceValues[channel] = balance;

    _playMutex.unlock();

    sendMixerBalance(source, channel);
}

// ---------------------------------------------------------------------------------------------------------------------
Float32 Studio::mixerBalance(int channel) {
    if (channel >= _nbChannels) return 0.0f;

    Float32 balance;

    _playMutex.lock();
    balance = _mixerBalanceValues[channel];
    _playMutex.unlock();

    return balance;
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::setMasterMixerLevel(int source, Float32 level) { _mixer->setLevel(level); }

// ---------------------------------------------------------------------------------------------------------------------
Float32 Studio::masterMixerLevel() { return _mixer->level(); }

// ---------------------------------------------------------------------------------------------------------------------
void Studio::addDidSetInstrumentFn(std::shared_ptr<didSetInstrumentFnType> didSetInstrumentFn) {
    _didSetInstrumentFns.push_back(didSetInstrumentFn);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::removeDidSetInstrumentFn(std::shared_ptr<didSetInstrumentFnType> didSetInstrumentFn) {
    _didSetInstrumentFns.erase(std::find(_didSetInstrumentFns.begin(), _didSetInstrumentFns.end(), didSetInstrumentFn));
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::addDidSetMixerLevelFn(std::shared_ptr<didSetMixerLevelFnType> didSetMixerLevelFn) {
    _didSetMixerLevelFns.push_back(didSetMixerLevelFn);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::removeDidSetMixerLevelFn(std::shared_ptr<didSetMixerLevelFnType> didSetMixerLevelFn) {
    _didSetMixerLevelFns.erase(std::find(_didSetMixerLevelFns.begin(), _didSetMixerLevelFns.end(), didSetMixerLevelFn));
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::addDidSetMixerBalanceFn(std::shared_ptr<didSetMixerBalanceFnType> didSetMixerBalanceFn) {
    _didSetMixerBalanceFns.push_back(didSetMixerBalanceFn);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::removeDidSetMixerBalanceFn(std::shared_ptr<didSetMixerBalanceFnType> didSetMixerBalanceFn) {
    _didSetMixerBalanceFns.erase(
        std::find(_didSetMixerBalanceFns.begin(), _didSetMixerBalanceFns.end(), didSetMixerBalanceFn));
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::addDidPlayNoteFn(std::shared_ptr<didPlayNoteFnType> didPlayNoteFn) {
    _didPlayNoteFns.push_back(didPlayNoteFn);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::removeDidPlayNoteFn(std::shared_ptr<didPlayNoteFnType> didPlayNoteFn) {
    _didPlayNoteFns.erase(std::find(_didPlayNoteFns.begin(), _didPlayNoteFns.end(), didPlayNoteFn));
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::addDidReleaseNoteFn(std::shared_ptr<didReleaseNoteFnType> didReleaseNoteFn) {
    _didReleaseNoteFns.push_back(didReleaseNoteFn);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::removeDidReleaseNoteFn(std::shared_ptr<didReleaseNoteFnType> didReleaseNoteFn) {
    _didReleaseNoteFns.erase(std::find(_didReleaseNoteFns.begin(), _didReleaseNoteFns.end(), didReleaseNoteFn));
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::addDidSetTempoFn(std::shared_ptr<didSetTempoFnType> didSetTempoFn) {
    _didSetTempoFns.push_back(didSetTempoFn);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::removeDidSetTempoFn(std::shared_ptr<didSetTempoFnType> didSetTempoFn) {
    _didSetTempoFns.erase(std::find(_didSetTempoFns.begin(), _didSetTempoFns.end(), didSetTempoFn));
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::addDidSetTimeSignatureFn(std::shared_ptr<didSetTimeSignatureFnType> didSetTimeSignatureFn) {
    _didSetTimeSignatureFns.push_back(didSetTimeSignatureFn);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::removeDidSetTimeSignatureFn(std::shared_ptr<didSetTimeSignatureFnType> didSetTimeSignatureFn) {
    _didSetTimeSignatureFns.erase(
        std::find(_didSetTimeSignatureFns.begin(), _didSetTimeSignatureFns.end(), didSetTimeSignatureFn));
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::addDidSetSustainFn(std::shared_ptr<didSetSustainFnType> didSetSustainFn) {
    _didSetSustainFns.push_back(didSetSustainFn);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::removeDidSetSustainFn(std::shared_ptr<didSetSustainFnType> didSetSustainFn) {
    _didSetSustainFns.erase(std::find(_didSetSustainFns.begin(), _didSetSustainFns.end(), didSetSustainFn));
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::addDidSetPitchBendFn(std::shared_ptr<didSetPitchBendFnType> didSetPitchBendFn) {
    _didSetPitchBendFns.push_back(didSetPitchBendFn);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::removeDidSetPitchBendFn(std::shared_ptr<didSetPitchBendFnType> didSetPitchBendFn) {
    _didSetPitchBendFns.erase(std::find(_didSetPitchBendFns.begin(), _didSetPitchBendFns.end(), didSetPitchBendFn));
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::addDidSetModulationFn(std::shared_ptr<didSetModulationFnType> didSetModulationFn) {
    _didSetModulationFns.push_back(didSetModulationFn);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::removeDidSetModulationFn(std::shared_ptr<didSetPitchBendFnType> didSetModulationFn) {
    _didSetModulationFns.erase(std::find(_didSetModulationFns.begin(), _didSetModulationFns.end(), didSetModulationFn));
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::addDidSetControlValueFn(std::shared_ptr<didSetControlValueFnType> didSetControlValueFn) {
    _didSetControlValueFns.push_back(didSetControlValueFn);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::removeDidSetControlValueFn(std::shared_ptr<didSetControlValueFnType> didSetControlValueFn) {
    _didSetControlValueFns.erase(
        std::find(_didSetControlValueFns.begin(), _didSetControlValueFns.end(), didSetControlValueFn));
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::addDidPerformKeyAftertouchFn(std::shared_ptr<didPerformKeyAftertouchFnType> didPerformKeyAftertouchFn) {
    _didPerformKeyAftertouchFns.push_back(didPerformKeyAftertouchFn);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::removeDidPerformKeyAftertouchFn(std::shared_ptr<didPerformKeyAftertouchFnType> didPerformKeyAftertouchFn) {
    _didPerformKeyAftertouchFns.erase(
        std::find(_didPerformKeyAftertouchFns.begin(), _didPerformKeyAftertouchFns.end(), didPerformKeyAftertouchFn));
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::addDidPerformChannelAftertouchFn(
    std::shared_ptr<didPerformChannelAftertouchFnType> didPerformChannelAftertouchFn) {
    _didPerformChannelAftertouchFns.push_back(didPerformChannelAftertouchFn);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::removeDidPerformChannelAftertouchFn(
    std::shared_ptr<didPerformChannelAftertouchFnType> didPerformChannelAftertouchFn) {
    _didPerformChannelAftertouchFns.erase(std::find(
        _didPerformChannelAftertouchFns.begin(), _didPerformChannelAftertouchFns.end(), didPerformChannelAftertouchFn));
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::addDidSetStudioLoadingStateFn(std::shared_ptr<didSetStudioLoadingStateFnType> didSetStudioLoadingStateFn) {
    _didSetStudioLoadingStateFns.push_back(didSetStudioLoadingStateFn);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::removeDidSetStudioLoadingStateFn(
    std::shared_ptr<didSetStudioLoadingStateFnType> didSetStudioLoadingStateFn) {
    _didSetStudioLoadingStateFns.erase(std::find(_didSetStudioLoadingStateFns.begin(),
                                                 _didSetStudioLoadingStateFns.end(), didSetStudioLoadingStateFn));
}

// ---------------------------------------------------------------------------------------------------------------------
const Preset* Studio::presetForInstrument(int instrument) {
    if (instrument == STUDIO_INSTRUMENT_NONE) return nullptr;

    int bank = STUDIO_PRESET_BANK_FROM_INSTRUMENT(instrument);
    int number = STUDIO_PRESET_NUMBER_FROM_INSTRUMENT(instrument);

    std::vector<Preset>::iterator it;
    for (it = _presets.begin(); it != _presets.end(); ++it) {
        if ((it->_bank == bank) && (it->_number == number)) return &(*it);
    }

    return nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------
double Studio::getTimestamp() {
    typedef std::chrono::duration<double> doublePrecisionDurationType;
    typedef std::chrono::time_point<std::chrono::high_resolution_clock, doublePrecisionDurationType> timePointType;

    timePointType timestampTP = std::chrono::high_resolution_clock::now();
    doublePrecisionDurationType duration = timestampTP.time_since_epoch();
    double timestamp = std::chrono::duration_cast<std::chrono::seconds>(duration).count();

    return timestamp;
}

// =====================================================================================================================
// Notifications sent from the studio
//

// ---------------------------------------------------------------------------------------------------------------------
void Studio::dumpStates(int source) {
    // Send CC events
    for (int i = 0; i < _nbChannels; ++i) {
        // Pitch bend range
        sendControlValue(source, STUDIO_CONTROL_REG_PARAM_NUM_MSB, 0, i);
        sendControlValue(source, STUDIO_CONTROL_REG_PARAM_NUM_LSB, 0, i);
        auto pitchBendFactor = _pitchBendFactorValues[i] * 2.0f;
        sendControlValue(source, STUDIO_CONTROL_DATA_ENTRY_MSB, (UInt32)(pitchBendFactor), i);
        sendControlValue(source, STUDIO_CONTROL_DATA_ENTRY_LSB,
                         (UInt32)((pitchBendFactor - floorf(pitchBendFactor)) * 100.0f), i);
        // Reverb & chorus
        sendControlValue(source, STUDIO_CONTROL_REVERB, _controlValues[i][STUDIO_CONTROL_REVERB], i);
        sendControlValue(source, STUDIO_CONTROL_CHORUS, _controlValues[i][STUDIO_CONTROL_CHORUS], i);
    }
    for (int i = 0; i < _nbChannels; ++i) {
        sendMixerLevel(source, i);
        sendMixerBalance(source, i);
        sendInstrument(source, i);
    }
    sendTempo(source);
    sendTimeSignature(source);
    for (int i = 0; i < _nbChannels; ++i) {
        sendSustain(source, i);
        sendPitchBend(source, i);
        sendModulation(source, i);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::sendMixerLevel(int source, int channel) {
    if (!_areStateNotificationsEnabled) return;

    // Send the mixer level
    if (channel >= _nbChannels) return;

    Float32 mixerLevel = _mixerLevelValues[channel];

    UInt32 tick = _metronome->tick();

    Platform::sharedInstance()->invoke([=]() {
        for (auto didSetMixerLevelFn : _didSetMixerLevelFns) {
            (*didSetMixerLevelFn)(this, source, getTimestamp(), tick, channel, mixerLevel);
        }
    });

    if (_isMIDIOutputEnabled && _midiOutputDidSetMixerLevelFn)
        _midiOutputDidSetMixerLevelFn(this, source, getTimestamp(), tick, channel, mixerLevel);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::sendMixerBalance(int source, int channel) {
    if (!_areStateNotificationsEnabled) return;

    // Send the mixer balance
    if (channel >= _nbChannels) return;

    Float32 mixerBalance = _mixerBalanceValues[channel];

    UInt32 tick = _metronome->tick();

    Platform::sharedInstance()->invoke([=]() {
        for (auto didSetMixerBalanceFn : _didSetMixerBalanceFns) {
            (*didSetMixerBalanceFn)(this, source, getTimestamp(), tick, channel, mixerBalance);
        }
    });

    if (_isMIDIOutputEnabled && _midiOutputDidSetMixerBalanceFn)
        _midiOutputDidSetMixerBalanceFn(this, source, getTimestamp(), tick, channel, mixerBalance);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::sendInstrument(int source, int channel) {
    if (!_areStateNotificationsEnabled) return;

    // Send the instrument
    if (channel >= _nbChannels) return;

    int instrument = _instruments[channel];

    UInt32 tick = _metronome->tick();

    Platform::sharedInstance()->invoke([=]() {
        for (auto didSetInstrumentFn : _didSetInstrumentFns) {
            (*didSetInstrumentFn)(this, source, getTimestamp(), tick, channel, instrument);
        }
    });

    if (_isMIDIOutputEnabled && _midiOutputDidSetInstrumentFn)
        _midiOutputDidSetInstrumentFn(this, source, getTimestamp(), tick, channel, instrument);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::sendTempo(int source) {
    if (!_areStateNotificationsEnabled || _metronome->bpms().empty()) return;

    // Send the tempo

    UInt32 tick = _metronome->tick();
    auto bpm = _metronome->bpmForTick(tick);

    UInt32 mpqn = 60000000 / bpm;

    Platform::sharedInstance()->invoke([=]() {
        for (auto didSetTempo : _didSetTempoFns) {
            (*didSetTempo)(this, source, getTimestamp(), tick, mpqn);
        }
    });
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::sendTimeSignature(int source) {
    if (!_areStateNotificationsEnabled || _metronome->timeSignatures().empty()) return;

    // Send the time signature

    UInt32 tick = _metronome->tick();
    auto timeSignature = _metronome->timeSignatureForTick(tick);

    Platform::sharedInstance()->invoke([=]() {
        for (auto didSetTimeSignature : _didSetTimeSignatureFns) {
            (*didSetTimeSignature)(this, source, getTimestamp(), tick, timeSignature.first, timeSignature.second);
        }
    });
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::sendSustain(int source, int channel) {
    if (!_areStateNotificationsEnabled) return;

    // Send the sustain state
    if (channel >= _nbChannels) return;

    Float32 sustain = _sustainValues[channel];

    UInt32 tick = _metronome->tick();

    Platform::sharedInstance()->invoke([=]() {
        for (auto didSetSustain : _didSetSustainFns) {
            (*didSetSustain)(this, source, getTimestamp(), tick, channel, sustain);
        }
    });

    if (_isMIDIOutputEnabled && _midiOutputDidSetSustainFn)
        _midiOutputDidSetSustainFn(this, source, getTimestamp(), tick, channel, sustain);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::sendPitchBend(int source, int channel) {
    if (!_areStateNotificationsEnabled) return;

    // Send the pitch bend value
    if (channel >= _nbChannels) return;

    Float32 pitchBend = _pitchBendValues[channel];

    UInt32 tick = _metronome->tick();

    Platform::sharedInstance()->invoke([=]() {
        for (auto didSetPitchBend : _didSetPitchBendFns) {
            (*didSetPitchBend)(this, source, getTimestamp(), tick, channel, pitchBend);
        }
    });

    if (_isMIDIOutputEnabled && _midiOutputDidSetPitchBendFn)
        _midiOutputDidSetPitchBendFn(this, source, getTimestamp(), tick, channel, pitchBend);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::sendModulation(int source, int channel) {
    if (!_areStateNotificationsEnabled) return;

    // Send the pitch bend value
    if (channel >= _nbChannels) return;

    Float32 modulation = _modulationValues[channel];

    UInt32 tick = _metronome->tick();

    Platform::sharedInstance()->invoke([=]() {
        for (auto didSetModulation : _didSetModulationFns) {
            (*didSetModulation)(this, source, getTimestamp(), tick, channel, modulation);
        }
    });

    if (_isMIDIOutputEnabled && _midiOutputDidSetModulationFn)
        _midiOutputDidSetModulationFn(this, source, getTimestamp(), tick, channel, modulation);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::sendControlValue(int source, UInt32 control, UInt32 value, int channel) {
    if (!_areStateNotificationsEnabled) return;

    // Send the pitch bend value
    if (channel >= _nbChannels) return;

    UInt32 tick = _metronome->tick();

    Platform::sharedInstance()->invoke([=]() {
        for (auto didSetControlValue : _didSetControlValueFns) {
            (*didSetControlValue)(this, source, getTimestamp(), tick, channel, control, value);
        }
    });

    if (_isMIDIOutputEnabled && _midiOutputDidSetControlValueFn)
        _midiOutputDidSetControlValueFn(this, source, getTimestamp(), tick, channel, control, value);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::sendKeyAftertouch(int source, int pitch, Float32 value, int channel) {
    if (!_areStateNotificationsEnabled) return;

    // Send the pitch bend value
    if (channel >= _nbChannels) return;

    UInt32 tick = _metronome->tick();

    Platform::sharedInstance()->invoke([=]() {
        for (auto didPerformKeyAftertouch : _didPerformKeyAftertouchFns) {
            (*didPerformKeyAftertouch)(this, source, getTimestamp(), tick, channel, pitch, value);
        }
    });

    if (_isMIDIOutputEnabled && _midiOutputDidPerformKeyAftertouchFn)
        _midiOutputDidPerformKeyAftertouchFn(this, source, getTimestamp(), tick, channel, pitch, value);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::sendChannelAftertouch(int source, Float32 value, int channel) {
    if (!_areStateNotificationsEnabled) return;

    // Send the pitch bend value
    if (channel >= _nbChannels) return;

    UInt32 tick = _metronome->tick();

    Platform::sharedInstance()->invoke([=]() {
        for (auto didPerformChannelAftertouch : _didPerformChannelAftertouchFns) {
            (*didPerformChannelAftertouch)(this, source, getTimestamp(), tick, channel, value);
        }
    });

    if (_isMIDIOutputEnabled && _midiOutputDidPerformChannelAftertouchFn)
        _midiOutputDidPerformChannelAftertouchFn(this, source, getTimestamp(), tick, channel, value);
}

// =====================================================================================================================
// Metronome control
//

// ---------------------------------------------------------------------------------------------------------------------
bool Studio::isMetronomeRunning() { return _metronome->isRunning(); }

// ---------------------------------------------------------------------------------------------------------------------
void Studio::setMetronomeTimeDivision(unsigned int timeDivision) { _metronome->setTimeDivision(timeDivision); }

// ---------------------------------------------------------------------------------------------------------------------
void Studio::setMetronomeBPMs(int source, std::vector<std::pair<UInt32, unsigned int>> bpms) {
    _metronome->setBPMs(bpms);
    sendTempo(source);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::setMetronomeTimeSignatures(int source,
                                        std::vector<std::pair<UInt32, std::pair<UInt32, UInt32>>> timeSignatures) {
    _metronome->setTimeSignatures(timeSignatures);
    sendTimeSignature(source);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::startMetronome() { _metronome->start(); }

// ---------------------------------------------------------------------------------------------------------------------
void Studio::stopMetronome() { _metronome->stop(); }

// ---------------------------------------------------------------------------------------------------------------------
void Studio::resetMetronome() {
    if (_metronome->isRunning()) _metronome->stop();
    _metronome->setTick(0);
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::moveMetronomeToTick(UInt32 tick) { _metronome->moveToTick(tick); }

// =====================================================================================================================
// Metronome player delegate (thread safe)
//

// ---------------------------------------------------------------------------------------------------------------------
void Studio::metronomePlayMajorTick(Metronome* metronome) {
    _playMutex.lock();

    // We start the mixer if necessary
    startMixer();

    // We play the major tick
    _metronomeSampler->stopAllNotes(STUDIO_METRONOME_CHANNEL);
    _metronomeSampler->playNote(126, 0.8f, _metronomeInstrument, STUDIO_METRONOME_CHANNEL);

    _playMutex.unlock();
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::metronomePlayMinorTick(Metronome* metronome) {
    _playMutex.lock();

    // We start the mixer if necessary
    startMixer();

    // We play the minor tick
    _metronomeSampler->stopAllNotes(STUDIO_METRONOME_CHANNEL);
    _metronomeSampler->playNote(127, 0.8f, _metronomeInstrument, STUDIO_METRONOME_CHANNEL);

    _playMutex.unlock();
}

// =====================================================================================================================
// Mixer control
//

// ---------------------------------------------------------------------------------------------------------------------
void Studio::setAudioOutputDevice(const std::string& audioOutputDeviceName, double latency) {
    bool mixerWasRunning = _mixer->isRunning();
    std::string previousOutputDeviceName = _mixer->outputDeviceName();

    // Make sure the mixer is stopped
    _mixer->stop();

    _mixer->setOutputDeviceName(audioOutputDeviceName);
    _mixer->setOutputLatency(latency);

    // Re-start the mixer if necessary
    if (mixerWasRunning) _mixer->start();
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::startMixer() {
    if (!_mixer->isRunning()) {
        // We clear the voices
        _sampler->clearAllVoices();

        // We start the mixer
        _mixer->start();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::stopMixer() {
    _mixer->stop();
    _sampler->clearAllVoices();
}

// ---------------------------------------------------------------------------------------------------------------------
void Studio::playAudioFile(std::string path, Float32 level) {
    _audioPlayer->setLevel(level);
    _audioPlayer->playAudioFile(path);
}
