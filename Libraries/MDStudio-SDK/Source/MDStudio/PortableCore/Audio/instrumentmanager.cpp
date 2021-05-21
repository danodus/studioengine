#include "instrumentmanager.h"

#include <math.h>

#include <algorithm>

#include "soundfont2.h"

#define CLAMP(_x_) (_x_ < -1.0 ? -1.0 : (_x_ > 1.0 ? 1.0 : _x_))

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
InstrumentManager::InstrumentManager(const std::string& audioPath) { _audioPath = audioPath; }

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MultiInstrument> InstrumentManager::loadSF2MultiInstrument(const std::string& name, int presetBank,
                                                                           int preset) {
    SoundFont2 sf2;

    std::shared_ptr<MultiInstrument> multiInstrument = nullptr;

    //
    // Load the instrument
    //

    std::string path = _audioPath + "/" + name + ".sf2";

    // Read the SF2 file
    if (!sf2.load(path)) return nullptr;

    multiInstrument = std::shared_ptr<MultiInstrument>(new MultiInstrument());

    auto basePos = sf2.samplesOffset();

    auto data = sf2.dataForPreset(presetBank, preset);

    // For each data element
    for (auto datum : data) {
        auto instrument = std::make_shared<Instrument>();

        instrument->setAudioFilename(name);

        instrument->setBasePitch((Float32)(datum.origKeyAndCorr >> 8) - (Float32)(datum.coarseTune) -
                                 (Float32)(datum.fineTune) / 100.0f);
        instrument->setLowestPitch((Float32)datum.minPitch);
        instrument->setHighestPitch((Float32)datum.maxPitch);
        instrument->setLowestVelocity((Float32)datum.minVelocity);
        instrument->setHighestVelocity((Float32)datum.maxVelocity);

        // If the sample is set to loop
        if ((datum.sampleModes & 0x3) != 0) {
            instrument->setLoopStart((Float64)(datum.startLoop - datum.start));
            instrument->setLoopEnd((Float64)(datum.endLoop - datum.start));
        } else {
            // The instrument is not looping
            Float64 loop = (Float64)(datum.end - datum.start);
            instrument->setLoopStart(loop);
            instrument->setLoopEnd(loop);
        }

        instrument->setSF2SampleBasePos(basePos);
        instrument->setSF2SampleStart(datum.start);
        instrument->setSF2SampleEnd(datum.end);
        instrument->setSF2SampleRate(datum.sampleRate);

        Float64 length = (Float64)(datum.end - datum.start);

        // Sanity check for the loop
        if (instrument->loopStart() > length) {
            instrument->setLoopStart(length);
        }

        if (instrument->loopEnd() > length) {
            instrument->setLoopEnd(length);
        }

        // Volume envelope
        Float32 attackTime = exp2f((Float32)datum.attackVolEnv / 1200.0f);
        Float32 holdTime = exp2f((Float32)datum.holdVolEnv / 1200.0f);
        Float32 decayTime = exp2f((Float32)datum.decayVolEnv / 1200.0f);

        if (datum.sustainVolEnv > 1000) datum.sustainVolEnv = 1000;

        Float32 sustainLevel = 1.0f - ((Float32)(datum.sustainVolEnv) / 1000.0f);

        Float32 releaseTime = exp2f((Float32)datum.releaseVolEnv / 1200.0f);

        instrument->setAttackRate(attackTime);
        instrument->setHoldTime(holdTime);
        instrument->setDecayRate(decayTime);
        instrument->setSustainLevel(sustainLevel);
        instrument->setReleaseRate(releaseTime);

        Float32 balance = CLAMP((Float32)datum.panEffectsSend / 500.0f);
        instrument->setLowestBalance(balance);
        instrument->setHighestBalance(balance);

        Float32 filterFcPitch = (Float32)(datum.initialFilterFc) / 100.0f;
        instrument->setFilterFc(exp2f((filterFcPitch - 69.0f) / 12.0f) * 440.0f);
        instrument->setFilterQ(powf(10.0f, (Float32)(datum.initialFilterQ) / 200.0f));

        // Add the instrument
        multiInstrument->addInstrument(instrument);

    }  // for each data element

    //
    // We load the samples for each instrument
    //

    // For each instrument
    std::vector<std::shared_ptr<Instrument>>::iterator it;
    for (it = multiInstrument->instrumentsBegin(); it != multiInstrument->instrumentsEnd(); it++) {
        std::shared_ptr<Instrument> instrument = *it;

        //
        // We check if the sample has already been loaded from another instrument
        //

        std::shared_ptr<Sample> foundSample = nullptr;
        // For each loaded sample

        std::vector<std::shared_ptr<Sample>>::iterator itSample;
        for (itSample = _loadedSamples.begin(); itSample != _loadedSamples.end(); itSample++) {
            std::shared_ptr<Sample> sample = *itSample;
            // If we have the same sample already loaded
            if ((sample->SF2SampleStart() == instrument->SF2SampleStart()) &&
                (sample->SF2SampleEnd() == instrument->SF2SampleEnd())) {
                // We found the sample
                foundSample = sample;
                break;
            }
        }  // for each loaded sample

        // If we found a sample
        if (foundSample) {
            // We set the sample
            instrument->setSample(foundSample);
        } else {
            // We load the sample
            std::shared_ptr<Sample> newSample = instrument->loadSF2Sample(path);
            _loadedSamples.push_back(newSample);
        }
    }  // for each instrument

    return multiInstrument;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<Preset> InstrumentManager::getSF2Presets(const std::string& name) {
    SoundFont2 sf2;

    std::vector<Preset> presetNames;

    // We load the instrument
    std::string path = _audioPath + "/" + name + ".sf2";

    // We read the SF2 file
    if (!sf2.load(path)) return presetNames;

    // We get the preset headers
    auto presetHeaders = sf2.presetHeaders();

    // For each preset header
    for (auto presetHeader : presetHeaders) {
        Preset preset;
        preset._bank = presetHeader.bank;
        preset._number = presetHeader.preset;
        preset._name = (char*)presetHeader.name;
        presetNames.push_back(preset);
    }

    // Sort the presets
    std::sort(presetNames.begin(), presetNames.end(), [](Preset a, Preset b) {
        if (a._bank < b._bank) return true;
        if (a._bank > b._bank) return false;
        if (a._number < b._number) return true;
        return false;
    });

    return presetNames;
}

// ---------------------------------------------------------------------------------------------------------------------
void InstrumentManager::unloadMultiInstrument(std::shared_ptr<MultiInstrument> multiInstrument) {
    std::vector<std::shared_ptr<Instrument>>::iterator itInstrument;

    // For each instrument
    for (itInstrument = multiInstrument->instrumentsBegin(); itInstrument != multiInstrument->instrumentsEnd();
         itInstrument++) {
        std::shared_ptr<Instrument> instrument = *itInstrument;
        std::shared_ptr<Sample> sample = instrument->sample();
        auto itSample = std::find(_loadedSamples.begin(), _loadedSamples.end(), sample);
        if (itSample != _loadedSamples.end()) _loadedSamples.erase(itSample);
    }
}
