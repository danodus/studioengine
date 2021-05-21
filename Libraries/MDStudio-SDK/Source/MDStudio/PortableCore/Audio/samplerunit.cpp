//
//  samplerunit.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-21.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

#include <assert.h>
#include <math.h>

#include <chrono>
#include <thread>
#include <vector>

#include "filter_k12z4.h"
#include "mixer.h"
#include "samplerunit.h"

#if (TARGET_OS_IPHONE || TARGET_OS_MAC)
#include <Accelerate/Accelerate.h>
#endif

// Convert [0 .. 1] to [-100 .. 0]
#define TO_ATTENUATION_DB_RANGE(_x_, _range_) ((1.0f - _x_) * _range_)

#define GRAPH_SAMPLE_RATE 44100
#define SAMPLER_CROSS_FADE_RATE (1000.0 / GRAPH_SAMPLE_RATE)

// Returns the rate for a given pitch
#define SAMPLER_RATE_FOR_PITCH(_base_pitch_, _pitch_) exp2f((float)(_pitch_ - _base_pitch_) / 12.0f)

#define SAMPLER_PLAY_NOTE_CMD 0
#define SAMPLER_RELEASE_NOTE_CMD 1
#define SAMPLER_STOP_NOTE_CMD 2
#define SAMPLER_RELEASE_ALL_NOTES_CMD 3
#define SAMPLER_STOP_ALL_NOTES_PITCH_CMD 4
#define SAMPLER_STOP_ALL_NOTES_CMD 5
#define SAMPLER_SET_SUSTAIN_STATE_CMD 6
#define SAMPLER_SET_PITCH_BEND_CMD 7
#define SAMPLER_SET_MODULATION_CMD 8
#define SAMPLER_IS_NOTE_PLAYING_CMD 9
#define SAMPLER_IS_NOTE_PLAYING_PITCH_CMD 10
#define SAMPLER_CLEAR_VOICES_CMD 11
#define SAMPLER_CLEAR_ALL_VOICES_CMD 12
#define SAMPLER_IS_MULTI_INSTRUMENT_IN_USE_CMD 13
#define SAMPLER_SET_LEVEL_CMD 14
#define SAMPLER_SET_BALANCE_CMD 15
#define SAMPLER_SET_REVERB_CMD 16
#define SAMPLER_SET_CHORUS_CMD 17
#define SAMPLER_SET_EXPRESSION_CMD 18
#define SAMPLER_SET_PITCH_BEND_FACTOR_CMD 19

using namespace MDStudio;

static float attenuationTable[1000];
static Float32 volumeForVelocities[100];

// ---------------------------------------------------------------------------------------------------------------------
// Get the attenuation factor based on a factor in dB
// 0 dB -> 1.0
// -100 dB -> 0.0
static inline Float32 getAttenuation(Float32 dB) {
    int dBInt = (int)(dB * 10.0f);
    if (dBInt <= -1000) {
        return 0.0f;
    } else if (dBInt >= 0) {
        return 1.0f;
    }
    return attenuationTable[-dBInt];
}

#if _LINUX
// ---------------------------------------------------------------------------------------------------------------------
// Ref.: http://stackoverflow.com/questions/11436551/dot-product-w-neon-intrinsics
static inline Float32 dotProduct4(const Float32* a, const Float32* b, int n) {
    float net1D = 0.0f;
    assert(n % 4 == 0);  // required floats 'a' & 'b' to be multiple of 4
#if defined(__ARM_NEON__)
    asm volatile(
        "vmov.f32 q8, #0.0          \n\t"  // zero out q8 register
        "1:                         \n\t"
        "subs %3, %3, #4            \n\t"  // we load 4 floats into q0, and q2 register
        "vld1.f32 {d0,d1}, [%1]!    \n\t"  // loads q0, update pointer *a
        "vld1.f32 {d4,d5}, [%2]!    \n\t"  // loads q2, update pointer *b
        "vmla.f32 q8, q0, q2        \n\t"  // store four partial sums in q8
        "bgt 1b                     \n\t"  // loops to label 1 until n==0
        "vpadd.f32 d0, d16, d17     \n\t"  // pairwise add 4 partial sums in q8, store in d0
        "vadd.f32 %0, s0, s1        \n\t"  // add 2 partial sums in d0, store result in return variable net1D
        : "=w"(net1D)                      // output
        : "r"(a), "r"(b), "r"(n)           // input
        : "q0", "q2", "q8");               // clobber list
#else
    for (int k = 0; k < n; k++) {
        net1D += a[k] * b[k];
    }
#endif
    return net1D;
}
#endif  // _LINUX

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::renderVoice(struct Voice* voice, UInt64 rate, GraphSampleType* out) {
    if (voice->isPlaying && voice->data != NULL) {
        GraphSampleType s;
        UInt64 pos = voice->pos;

        //
        // We render the mono samples
        //

        if (rate != (UInt64)1 << VOICE_FRACTION_BITS) {
            //
            // Band-limited interpolation
            //

            SInt32 posInt = pos >> VOICE_FRACTION_BITS;

            UInt64 sincTableOffset = (pos & (((UInt64)(1) << VOICE_FRACTION_BITS) - 1)) >>
                                     (VOICE_FRACTION_BITS - RESAMPLE_WINDOW_SAMPLES_PER_ZERO_CROSSING_BITS);

            SampleType* pCoefficients = resamplerCoefficients[sincTableOffset];

            SampleType* pIn = voice->data + (posInt - RESAMPLE_WINDOW_NB_ZERO_CROSSINGS - 1);
#if _LINUX

            s = dotProduct4(pIn, pCoefficients, 2 * RESAMPLE_WINDOW_NB_ZERO_CROSSINGS);

#else  // _LINUX

#if (TARGET_OS_IPHONE || TARGET_OS_MAC)

            GraphSampleType tempS;

            vDSP_dotpr(pIn, 1, pCoefficients, 1, &tempS, 2 * RESAMPLE_WINDOW_NB_ZERO_CROSSINGS + 1);

            s = (GraphSampleType)tempS;

#else
            s = 0;

            for (int i = 0; i < 2 * RESAMPLE_WINDOW_NB_ZERO_CROSSINGS + 1; i++) {
                s += *pIn * *pCoefficients;
                pCoefficients++;
                pIn++;
            }

#endif

#endif  // _LINUX

        } else {
            //
            // No interpolation
            //

            SampleType* pIn = voice->data + (pos >> VOICE_FRACTION_BITS);
            s = *pIn;
        }

        Float32 volEnvFactor = voice->volEnvFactor;
        UInt32 volEnvHoldCounter = voice->volEnvHoldCounter;

        // We apply the voice balance and volume envelope factors
        *out = s * getAttenuation(TO_ATTENUATION_DB_RANGE(volEnvFactor, -100));

        // We progress in the wave
        pos += rate;

        // We loop
        if (voice->loopStart != 0 || voice->loopEnd != 0) {
            if (pos >= voice->loopEnd) {
                if (voice->loopStart != voice->loopEnd) {
                    pos = (pos - voice->loopEnd) % (voice->loopEnd - voice->loopStart) + voice->loopStart;
                } else {
                    pos = voice->loopStart;
                }
            }
        }

        if (!voice->isNoteOn) {
            // The note is no longer played

            // We update the release phase envelope
            if (!voice->isNoteSustained) {
                volEnvFactor -= voice->releaseRatePerSample;
            }

            if (volEnvFactor < 0.0f) {
                voice->isPlaying = false;
                voice->data = nullptr;
            }

            if (voice->pos >= voice->length) {
                voice->isPlaying = false;
                voice->data = nullptr;
                voice->instrument = nullptr;
            }
        }

        // If the note is on or if the note is sustained
        if (voice->isNoteOn || voice->isNoteSustained) {
            if (volEnvHoldCounter > 0) {
                // We are in the hold phase
                volEnvHoldCounter--;
            } else {
                // We are in the decay-sustain phase envelope

                // If we are still above the sustain level, we decrease the factor
                if (volEnvFactor > voice->sustainLevel) {
                    volEnvFactor -= voice->decayRatePerSample;
                }
            }
        }

        voice->volEnvHoldCounter = volEnvHoldCounter;
        voice->volEnvFactor = volEnvFactor;
        voice->pos = pos;

    } else {
        // The data is null or the voice is not playing therefore we force silence.
        *out = 0;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
int SamplerUnit::renderInput(UInt32 inNumberFrames, GraphSampleType* ioData[2], UInt32 stride) {
    // Command interpreter
    SamplerCmd cmd;
    while (_cmdQueue.try_dequeue(cmd)) {
        switch (cmd.op) {
            case SAMPLER_PLAY_NOTE_CMD:
                playNoteCmd(cmd.p1, cmd.p2, cmd.multiInstrument, cmd.channel);
                break;
            case SAMPLER_RELEASE_NOTE_CMD:
                releaseNoteCmd(cmd.p1, cmd.channel);
                break;
            case SAMPLER_STOP_NOTE_CMD:
                stopNoteCmd(cmd.p1, cmd.channel);
                break;
            case SAMPLER_RELEASE_ALL_NOTES_CMD:
                releaseAllNotesCmd(cmd.p1, cmd.channel);
                break;
            case SAMPLER_STOP_ALL_NOTES_PITCH_CMD:
                stopAllNotesPitchCmd(cmd.p1, cmd.channel);
                break;
            case SAMPLER_STOP_ALL_NOTES_CMD:
                stopAllNotesCmd(cmd.channel);
                break;
            case SAMPLER_SET_SUSTAIN_STATE_CMD:
                setSustainStateCmd(cmd.p1 == 1.0f, cmd.channel);
                break;
            case SAMPLER_SET_PITCH_BEND_CMD:
                setPitchBendCmd(cmd.p1, cmd.channel);
                break;
            case SAMPLER_SET_MODULATION_CMD:
                setModulationCmd(cmd.p1, cmd.channel);
                break;
            case SAMPLER_IS_NOTE_PLAYING_CMD:
                isNotePlayingCmd(cmd.channel);
                break;
            case SAMPLER_IS_NOTE_PLAYING_PITCH_CMD:
                isNotePlayingPitchCmd(cmd.p1, cmd.channel);
                break;
            case SAMPLER_CLEAR_VOICES_CMD:
                clearVoicesCmd(cmd.channel);
                break;
            case SAMPLER_CLEAR_ALL_VOICES_CMD:
                clearAllVoicesCmd();
                break;
            case SAMPLER_IS_MULTI_INSTRUMENT_IN_USE_CMD:
                isMultiInstrumentInUseCmd(cmd.multiInstrument);
                break;
            case SAMPLER_SET_LEVEL_CMD:
                setLevelCmd(cmd.p1, cmd.channel);
                break;
            case SAMPLER_SET_BALANCE_CMD:
                setBalanceCmd(cmd.p1, cmd.channel);
                break;
            case SAMPLER_SET_REVERB_CMD:
                setReverbCmd(cmd.p1, cmd.channel);
                break;
            case SAMPLER_SET_CHORUS_CMD:
                setChorusCmd(cmd.p1, cmd.channel);
                break;
            case SAMPLER_SET_EXPRESSION_CMD:
                setExpressionCmd(cmd.p1, cmd.channel);
                break;
            case SAMPLER_SET_PITCH_BEND_FACTOR_CMD:
                setPitchBendFactorCmd(cmd.p1, cmd.channel);
                break;
        }
    }

    struct SamplerContext* context = &_context;

    // LFO

    context->lfoPhase += context->lfoFrequency * (Float32)(inNumberFrames) / GRAPH_SAMPLE_RATE;
    if (context->lfoPhase > 1.0f) context->lfoPhase = 0.0f;
    Float32 lfoPitch =
        ((context->lfoPhase < 0.5f) ? (context->lfoPhase * 2.0f) : (1.0f - (context->lfoPhase - 0.5f) * 2.0f));

    GraphSampleType reverbBufA[4096];
    GraphSampleType reverbBufB[4096];

    GraphSampleType chorusBufA[4096];
    GraphSampleType chorusBufB[4096];

    for (UInt32 i = 0; i < inNumberFrames; ++i) {
        reverbBufA[i] = 0;
        reverbBufB[i] = 0;
        chorusBufA[i] = 0;
        chorusBufB[i] = 0;
    }

    GraphSampleType* outA = (GraphSampleType*)ioData[0];
    GraphSampleType* outB = (GraphSampleType*)ioData[1];

    // For each channel
    for (UInt32 channelIndex = 0; channelIndex < SAMPLER_MAX_CHANNELS; ++channelIndex) {
        GraphSampleType channelBufA[4096];
        GraphSampleType channelBufB[4096];

        for (UInt32 i = 0; i < inNumberFrames; ++i) {
            channelBufA[i] = 0;
            channelBufB[i] = 0;
        }

        // For each voice
        for (UInt32 voiceIndex = 0; voiceIndex < context->nbVoices; ++voiceIndex) {
            struct Voice* voice = &context->voices[voiceIndex];

            // If the voice is not for the current channel, continue
            if (voice->channel != channelIndex) continue;

            assert(inNumberFrames <= 4096);

            GraphSampleType bufA[4096];
            GraphSampleType bufB[4096];

            if (voice->isPlaying) {
                UInt64 rate =
                    SAMPLER_RATE_FOR_PITCH(
                        voice->instrumentBasePitch,
                        voice->pitch +
                            context->pitchBendValues[voice->channel] * context->pitchBendFactors[voice->channel] +
                            context->modulationValues[voice->channel] * 0.5f * (lfoPitch - 0.5f)) *
                    (voice->instrumentSampleRate / GRAPH_SAMPLE_RATE) * ((UInt64)1 << VOICE_FRACTION_BITS);

                // We calculate the balance
                Float32 mixerBalance = context->balance[voice->channel];  // between -1.0 and 1.0
                Float32 volumeA, volumeB;
                if (mixerBalance < 0.0f) {
                    volumeA = 1.0f;
                    volumeB = mixerBalance + 1.0f;
                } else {
                    volumeA = 1.0f - mixerBalance;
                    volumeB = 1.0f;
                }

                float level = context->level[voice->channel] * context->expressionValues[voice->channel];

                volumeA *= level;
                volumeB *= level;

                Float32 crossFadeFactor = context->crossFadeFactors[voiceIndex];

                struct Voice* oldVoice = &context->oldVoices[voiceIndex];

                for (UInt32 i = 0; i < inNumberFrames; ++i) {
                    GraphSampleType sA, sB;

                    // We render the current voice
                    renderVoice(voice, rate, &sA);
                    sB = sA;
                    sA *= voice->volumeA;
                    sB *= voice->volumeB;

                    // Render the old voice if still active
                    if (crossFadeFactor < 1.0f) {
                        UInt64 oldRate =
                            SAMPLER_RATE_FOR_PITCH(
                                oldVoice->instrumentBasePitch,
                                oldVoice->pitch + context->pitchBendValues[oldVoice->channel] +
                                    context->modulationValues[oldVoice->channel] * 0.5f * (lfoPitch - 0.5f)) *
                            (voice->instrumentSampleRate / GRAPH_SAMPLE_RATE) * ((UInt64)1 << VOICE_FRACTION_BITS);
                        GraphSampleType sOldA, sOldB;
                        renderVoice(oldVoice, oldRate, &sOldA);
                        sOldB = sOldA;
                        sOldA *= oldVoice->volumeA;
                        sOldB *= oldVoice->volumeB;

                        // We mix the old voice with the new one
                        sA = crossFadeFactor * sA + (1.0f - crossFadeFactor) * sOldA;
                        sB = crossFadeFactor * sB + (1.0f - crossFadeFactor) * sOldB;

                        crossFadeFactor += SAMPLER_CROSS_FADE_RATE;
                    }

                    sA *= volumeA;
                    sB *= volumeB;

                    bufA[i] = sA;
                    bufB[i] = sB;

                }  // for each sample

                context->crossFadeFactors[voiceIndex] = crossFadeFactor;

            } else {
                for (UInt32 i = 0; i < inNumberFrames; ++i) {
                    bufA[i] = 0;
                    bufB[i] = 0;
                }
            }

            //
            // Apply a low-pass filter
            //

            Float32 filterFc = voice->filterFc;

            Dsp::Params params;
            params[0] = GRAPH_SAMPLE_RATE;  // Sample rate
            params[1] = filterFc;           // Cut-off frequency
            params[2] = voice->filterQ;     // Q

            context->lowPassFilters[voiceIndex]->setParams(params);
            float* o[2] = {bufA, bufB};
            context->lowPassFilters[voiceIndex]->process(inNumberFrames, o);

            //
            // Calculate the maximum output and perform the mix
            //

            GraphSampleType maxOutput = 0;

            for (UInt32 i = 0; i < inNumberFrames; ++i) {
                GraphSampleType sA, sB;
                sA = bufA[i];
                sB = bufB[i];

                GraphSampleType sum = fabs(sA) + fabs(sB);

                if (sum > maxOutput) {
                    maxOutput = sum;
                }

                channelBufA[i] += sA;
                channelBufB[i] += sB;

            }  // for each sample

            voice->maxOutput = 0.9f * voice->maxOutput + 0.1f * maxOutput;

        }  // for each voice

        //
        // Mix the channel buffer with the output
        //

        UInt32 outIndex = 0;
        for (UInt32 i = 0; i < inNumberFrames; ++i) {
            reverbBufA[i] += channelBufA[i] * context->reverbValues[channelIndex];
            reverbBufB[i] += channelBufB[i] * context->reverbValues[channelIndex];

            chorusBufA[i] += channelBufA[i] * context->chorusValues[channelIndex];
            chorusBufB[i] += channelBufB[i] * context->chorusValues[channelIndex];

            outA[outIndex] += channelBufA[i];
            outB[outIndex] += channelBufB[i];
            outIndex += stride;
        }

    }  // for each channel

    //
    // Apply the reverberation
    //

    GraphSampleType reverb2BufA[4096];
    GraphSampleType reverb2BufB[4096];

    for (UInt32 i = 0; i < inNumberFrames; ++i) {
        reverb2BufA[i] = 0;
        reverb2BufB[i] = 0;
    }

    float* o[2] = {chorusBufA, chorusBufB};
    context->chorusModel.renderInput(inNumberFrames, o, 1);

    context->reverbModel.processMix(reverbBufA, reverbBufB, reverb2BufA, reverb2BufB, inNumberFrames, 1);

    // Combine
    UInt32 outIndex = 0;
    for (UInt32 i = 0; i < inNumberFrames; ++i) {
        outA[outIndex] += reverb2BufA[i] + chorusBufA[i];
        outB[outIndex] += reverb2BufB[i] + chorusBufB[i];
        outIndex += stride;
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
SamplerUnit::SamplerUnit(UInt32 nbVoices) : _cmdQueue(1024), _cmdOutQueue(1024) {
    // Initialize the filters
    for (int i = 0; i < SAMPLER_MAX_VOICES; ++i) _context.lowPassFilters[i] = nullptr;

    _nbVoices = 0;

    // Set the number of voices
    setNbVoices(nbVoices);

    // We initialize the states per channel
    for (int channelIndex = 0; channelIndex < SAMPLER_MAX_CHANNELS; channelIndex++) {
        _sustainValues[channelIndex] = 0.0f;
        _balanceValues[channelIndex] = 0.0f;
        _levelValues[channelIndex] = 0.6f;
        _pitchBendValues[channelIndex] = 0.0f;
        _modulationValues[channelIndex] = 0.0f;
        _reverbValues[channelIndex] = 0.3f;
        _chorusValues[channelIndex] = 0.0f;
        _expressionValues[channelIndex] = 1.0f;
        _pitchBendFactors[channelIndex] = 1.0f;

        _context.balance[channelIndex] = 0.0f;
        _context.level[channelIndex] = 0.6f;
        _context.pitchBendValues[channelIndex] = 0.0f;
        _context.modulationValues[channelIndex] = 0.0f;
        _context.expressionValues[channelIndex] = 1.0f;

        _context.reverbValues[channelIndex] = 0.3f;
        _context.chorusValues[channelIndex] = 0.0f;
        _context.pitchBendFactors[channelIndex] = 1.0f;
    }

    _context.reverbModel.setDry(0.0f);
    _context.reverbModel.setWet(1.0f);
    _context.reverbModel.setDamp(0.5f);
    _context.reverbModel.update();

    _context.chorusModel.setWet(1.0f);

    // We initialize the global tables if necessary

    calculateResamplerCoefficients();

    // We calculate the attenuation factors
    for (int i = 0; i < 1000; i++) {
        attenuationTable[i] = powf(10.0f, (float)-i / 100.0f);
    }

    // We calculate the volume for velocities
    Float32 d = expf(1) - 1.0f;
    for (int i = 0; i < 100; i++) {
        volumeForVelocities[i] = (expf((float)i / 100.0f) - 1.0f) / d;
    }

    _currentNotesGroupID = 0;
}

// ---------------------------------------------------------------------------------------------------------------------
SamplerUnit::~SamplerUnit() {
    for (int i = 0; i < _nbVoices; ++i) {
        delete _context.lowPassFilters[i];
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::setNbVoices(UInt32 nbVoices) {
    if (nbVoices > SAMPLER_MAX_VOICES) nbVoices = SAMPLER_MAX_VOICES;

    // Free the filters if already allocated
    for (int voiceIndex = 0; voiceIndex < _nbVoices; ++voiceIndex)
        if (_context.lowPassFilters[voiceIndex]) delete _context.lowPassFilters[voiceIndex];

    _nbVoices = nbVoices;
    _context.nbVoices = nbVoices;
    _context.lfoPhase = 0.0f;
    _context.lfoFrequency = 4.0f;

    // We initialize all the voices
    for (int voiceIndex = 0; voiceIndex < _nbVoices; voiceIndex++) {
        _context.lowPassFilters[voiceIndex] = new Dsp::FilterDesign<Dsp::RBJ::Design::LowPass, 2>();
        _context.voices[voiceIndex].isPlaying = NO;
        _context.voices[voiceIndex].data = NULL;
        _context.voices[voiceIndex].filterFc = 8000.0f;
        _context.voices[voiceIndex].filterQ = 1.0f;
        _context.oldVoices[voiceIndex].isPlaying = NO;
        _context.oldVoices[voiceIndex].data = NULL;
        _context.oldVoices[voiceIndex].filterFc = 8000.0f;
        _context.oldVoices[voiceIndex].filterQ = 1.0f;
        _context.crossFadeFactors[voiceIndex] = 1.0f;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::playNoteCmd(Float32 pitch, Float32 velocity, std::shared_ptr<MultiInstrument> multiInstrument,
                              UInt32 channel) {
    // If we have no current instrument, we return right away
    if (!multiInstrument) return;

    // We get an instruments based on the desired pitch and velocity
    std::vector<std::shared_ptr<Instrument>> instruments = multiInstrument->instruments(pitch, velocity);

    // All voices can initially be stolen
    for (int i = 0; i < _nbVoices; i++) {
        struct Voice* voice = &_context.voices[i];
        voice->isLocked = NO;
    }

    int nbFoundVoices = 0;
    std::vector<std::shared_ptr<Instrument>>::iterator it;
    for (it = instruments.begin(); it != instruments.end(); it++) {
        std::shared_ptr<Instrument> instrument = (*it);

        // We check if the instrument is available
        if (!instrument || (instrument->sample() && instrument->sample()->data() == NULL)) continue;

        // Clamp the velocity
        if (velocity < 0.0f) {
            velocity = 0.0f;
        } else if (velocity > 1.0f) {
            velocity = 1.0f;
        }

        // We find a voice
        bool voiceFound = false;

        GraphSampleType minOutput = 2.0f;

        int minOutputVoiceIndex = -1;

        struct Voice* voice;

        for (int i = 0; i < _nbVoices; i++) {
            voice = &_context.voices[i];
            if (!voice->isLocked && !voice->isPlaying) {
                voiceFound = true;
                break;
            }
            // If the note is not on, we have a candidate
            if (!voice->isLocked && voice->isPlaying && (!voice->isNoteOn || voice->channel == channel) &&
                (voice->maxOutput <= minOutput)) {
                minOutput = voice->maxOutput;
                minOutputVoiceIndex = i;
            }
        }  // for each voice

        // If all voices are playing, we steal one
        if (!voiceFound) {
            // If a candidate voice to be stolen has been found
            if (minOutputVoiceIndex >= 0) {
                voice = &_context.voices[minOutputVoiceIndex];
                voiceFound = true;

                // Steal all the notes in the group of the voice with the minimal output

                // For each voice
                for (int i = 0; i < _nbVoices; i++) {
                    struct Voice* v = &_context.voices[i];
                    // If the voice is to be stolen
                    if (!v->isLocked && (v->notesGroupID == voice->notesGroupID)) {
                        // We set the old voice in order to perform the cross-over
                        struct Voice* oldVoice = &_context.oldVoices[i];

                        oldVoice->channel = v->channel;

                        oldVoice->instrumentBasePitch = v->instrumentBasePitch;
                        oldVoice->instrumentSampleRate = v->instrumentSampleRate;

                        oldVoice->instrument = v->instrument;
                        oldVoice->data = v->data;
                        oldVoice->length = v->length;
                        oldVoice->loopStart = v->loopStart;
                        oldVoice->loopEnd = v->loopEnd;

                        oldVoice->pos = v->pos;
                        oldVoice->isNoteOn = v->isNoteOn;
                        oldVoice->isPlaying = v->isPlaying;
                        oldVoice->volEnvFactor = v->volEnvFactor;
                        oldVoice->volEnvHoldCounter = v->volEnvHoldCounter;
                        oldVoice->pitch = v->pitch;
                        oldVoice->velocity = v->velocity;
                        oldVoice->isNoteSustained = v->isNoteSustained;
                        oldVoice->balance = v->balance;
                        oldVoice->volumeA = v->volumeA;
                        oldVoice->volumeB = v->volumeB;
                        oldVoice->decayRatePerSample = v->decayRatePerSample;
                        oldVoice->sustainLevel = v->sustainLevel;
                        oldVoice->releaseRatePerSample = v->releaseRatePerSample;

                        oldVoice->filterFc = v->filterFc;
                        oldVoice->filterQ = v->filterQ;

                        oldVoice->notesGroupID = v->notesGroupID;

                        // Fade to silence
                        v->isPlaying = true;
                        v->data = nullptr;

                        _context.crossFadeFactors[i] = 0.0f;  // We start with the old voice

                        _context.lowPassFilters[i]->reset();

                    }  // if the voice is to be stolen
                }      // for each voice
            }          // if a candidate voice to be stolen has been found
        }              // if all voices are playing

        // If a voice is available
        if (voiceFound) {
            // We lock this voice in order to ensure that it will not be stolen again
            voice->isLocked = true;

            voice->channel = channel;

            voice->instrumentBasePitch = instrument->basePitch();
            voice->instrumentSampleRate = instrument->sample()->sampleRate();

            voice->instrument = instrument;
            voice->data = instrument->sample()->data();
            voice->length = instrument->sample()->length() * ((UInt64)1 << VOICE_FRACTION_BITS);
            voice->loopStart = instrument->loopStart() * ((UInt64)1 << VOICE_FRACTION_BITS);
            voice->loopEnd = instrument->loopEnd() * ((UInt64)1 << VOICE_FRACTION_BITS);

            voice->pos = 0;
            voice->isNoteOn = true;
            voice->isPlaying = true;
            voice->volEnvFactor = 1.0f;  // Note: The attack phase is not yet implemented
            voice->volEnvHoldCounter = instrument->holdTime() * GRAPH_SAMPLE_RATE;
            voice->pitch = pitch;
            voice->velocity = exp2f(velocity) - 1.0f;
            voice->isNoteSustained = _sustainValues[channel] >= 0.5f;
            voice->maxOutput = 2.0f;

            // We calculate the balance of the voice
            if (instrument->highestPitch() == instrument->lowestPitch()) {
                // No interpolation possible, so we pick only the lowest value
                voice->balance = instrument->lowestBalance();
            } else {
                // We interpolate the balance
                voice->balance =
                    instrument->lowestBalance() + (instrument->highestBalance() - instrument->lowestBalance()) *
                                                      (pitch - instrument->lowestPitch()) /
                                                      (instrument->highestPitch() - instrument->lowestPitch());
            }

            //
            // We pre-calculate the voice volume
            //

            Float32 volumeA, volumeB;
            if (voice->balance < 0.0f) {
                volumeA = 1.0f;
                volumeB = voice->balance + 1.0f;
            } else {
                volumeA = 1.0f - voice->balance;
                volumeB = 1.0f;
            }

            // We modulate the volume with the note velocity
            Float32 volumeForVelocity = volumeForVelocities[(int)(100.0f * voice->velocity) - 1];
            volumeA *= volumeForVelocity;
            volumeB *= volumeForVelocity;

            voice->volumeA = volumeA;
            voice->volumeB = volumeB;

            // Pre-calculate the decay rate per sample
            voice->decayRatePerSample = 1.0f / (instrument->decayRate() * GRAPH_SAMPLE_RATE);

            // Sustain level
            voice->sustainLevel = instrument->sustainLevel();

            // Pre-calculate the release rate per sample
            voice->releaseRatePerSample = 1.0f / (instrument->releaseRate() * GRAPH_SAMPLE_RATE);

            // Filter
            voice->filterFc = instrument->filterFc() <= 20000.0f ? instrument->filterFc() : 20000.0f;
            voice->filterQ = instrument->filterQ();

            // We set the group ID
            voice->notesGroupID = _currentNotesGroupID;

            ++nbFoundVoices;
        }  // if a voice is available
    }      // for each instrument

    // If we did not reach our target, we cancel
    if (nbFoundVoices != instruments.size()) {
        for (int i = 0; i < _nbVoices; i++) {
            Voice* voice = &_context.voices[i];
            if (voice->notesGroupID == _currentNotesGroupID) {
                voice->isPlaying = NO;
                voice->data = nullptr;
            }
        }
    }
    _currentNotesGroupID++;
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::playNote(Float32 pitch, Float32 velocity, std::shared_ptr<MultiInstrument> multiInstrument,
                           UInt32 channel) {
    SamplerCmd cmd = {SAMPLER_PLAY_NOTE_CMD, pitch, velocity, channel, multiInstrument};
    _cmdQueue.enqueue(cmd);
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::releaseNoteCmd(Float32 pitch, UInt32 channel) {
    for (int i = 0; i < _nbVoices; i++) {
        if (_context.voices[i].channel == channel && _context.voices[i].pitch == pitch && _context.voices[i].isNoteOn) {
            _context.voices[i].isNoteOn = NO;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::releaseNote(Float32 pitch, UInt32 channel) {
    SamplerCmd cmd = {SAMPLER_RELEASE_NOTE_CMD, pitch, 0, channel, nullptr};
    _cmdQueue.enqueue(cmd);
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::stopNoteCmd(Float32 pitch, UInt32 channel) {
    for (int i = 0; i < _nbVoices; i++) {
        if (_context.voices[i].channel == channel && _context.voices[i].pitch == pitch && _context.voices[i].isNoteOn) {
            _context.voices[i].isNoteOn = NO;
            _context.voices[i].isNoteSustained = NO;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::stopNote(Float32 pitch, UInt32 channel) {
    SamplerCmd cmd = {SAMPLER_STOP_NOTE_CMD, pitch, 0, channel, nullptr};
    _cmdQueue.enqueue(cmd);
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::releaseAllNotesCmd(Float32 pitch, UInt32 channel) {
    for (int i = 0; i < _nbVoices; i++) {
        if (_context.voices[i].channel == channel && _context.voices[i].pitch == pitch) {
            _context.voices[i].isNoteOn = NO;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::releaseAllNotes(Float32 pitch, UInt32 channel) {
    SamplerCmd cmd = {SAMPLER_RELEASE_ALL_NOTES_CMD, pitch, 0, channel, nullptr};
    _cmdQueue.enqueue(cmd);
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::stopAllNotesPitchCmd(Float32 pitch, UInt32 channel) {
    for (int i = 0; i < _nbVoices; i++) {
        if (_context.voices[i].channel == channel && _context.voices[i].pitch == pitch) {
            _context.voices[i].isNoteOn = NO;
            _context.voices[i].isNoteSustained = NO;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::stopAllNotes(Float32 pitch, UInt32 channel) {
    SamplerCmd cmd = {SAMPLER_STOP_ALL_NOTES_PITCH_CMD, pitch, 0, channel, nullptr};
    _cmdQueue.enqueue(cmd);
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::stopAllNotesCmd(UInt32 channel) {
    for (int i = 0; i < _nbVoices; i++) {
        if (_context.voices[i].channel == channel) {
            _context.voices[i].isNoteOn = NO;
            _context.voices[i].isNoteSustained = NO;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::stopAllNotes(UInt32 channel) {
    SamplerCmd cmd = {SAMPLER_STOP_ALL_NOTES_CMD, 0, 0, channel, nullptr};
    _cmdQueue.enqueue(cmd);
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::setSustainStateCmd(bool state, UInt32 channel) {
    for (int i = 0; i < _nbVoices; i++) {
        if (_context.voices[i].channel == channel) {
            _context.voices[i].isNoteSustained = state;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::setSustain(Float32 sustain, UInt32 channel) {
    _sustainValues[channel] = sustain;

    SamplerCmd cmd = {SAMPLER_SET_SUSTAIN_STATE_CMD, sustain >= 0.5f ? 1.0f : 0.0f, 0, channel, nullptr};
    _cmdQueue.enqueue(cmd);
}

// ---------------------------------------------------------------------------------------------------------------------
Float32 SamplerUnit::sustain(UInt32 channel) { return _sustainValues[channel]; }

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::setPitchBendCmd(Float32 pitchBend, UInt32 channel) { _context.pitchBendValues[channel] = pitchBend; }

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::setPitchBend(Float32 pitchBend, UInt32 channel) {
    _pitchBendValues[channel] = pitchBend;

    SamplerCmd cmd = {SAMPLER_SET_PITCH_BEND_CMD, pitchBend, 0, channel, nullptr};
    _cmdQueue.enqueue(cmd);
}

// ---------------------------------------------------------------------------------------------------------------------
Float32 SamplerUnit::pitchBend(UInt32 channel) { return _pitchBendValues[channel]; }

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::setModulationCmd(Float32 modulation, UInt32 channel) {
    _context.modulationValues[channel] = modulation;
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::setModulation(Float32 modulation, UInt32 channel) {
    _modulationValues[channel] = modulation;

    SamplerCmd cmd = {SAMPLER_SET_MODULATION_CMD, modulation, 0, channel, nullptr};
    _cmdQueue.enqueue(cmd);
}

// ---------------------------------------------------------------------------------------------------------------------
Float32 SamplerUnit::modulation(UInt32 channel) { return _modulationValues[channel]; }

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::isNotePlayingCmd(UInt32 channel) {
    bool isPlaying = false;

    for (int i = 0; i < _nbVoices; i++) {
        if (_context.voices[i].channel == channel && _context.voices[i].isNoteOn) {
            isPlaying = true;
        }
    }

    SamplerCmd cmd = {SAMPLER_IS_NOTE_PLAYING_CMD, isPlaying ? 1.0f : 0.0f, 0, channel, nullptr};
    _cmdOutQueue.enqueue(cmd);
}

// ---------------------------------------------------------------------------------------------------------------------
bool SamplerUnit::isNotePlaying(UInt32 channel) {
    SamplerCmd cmd = {SAMPLER_IS_NOTE_PLAYING_CMD, 0.0f, 0.0f, channel, nullptr};
    _cmdQueue.enqueue(cmd);

    while (!_cmdOutQueue.try_dequeue(cmd) && isRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    };

    assert(cmd.op == SAMPLER_IS_NOTE_PLAYING_CMD);
    return (cmd.p1 == 1.0f);
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::isNotePlayingPitchCmd(Float32 pitch, UInt32 channel) {
    bool isPlaying = false;

    for (int i = 0; i < _nbVoices; i++) {
        if (_context.voices[i].channel == channel && _context.voices[i].isNoteOn && _context.voices[i].pitch == pitch) {
            isPlaying = true;
        }
    }

    SamplerCmd cmd = {SAMPLER_IS_NOTE_PLAYING_PITCH_CMD, isPlaying ? 1.0f : 0.0f, 0, channel, nullptr};
    _cmdOutQueue.enqueue(cmd);
}

// ---------------------------------------------------------------------------------------------------------------------
bool SamplerUnit::isNotePlaying(Float32 pitch, UInt32 channel) {
    SamplerCmd cmd = {SAMPLER_IS_NOTE_PLAYING_PITCH_CMD, pitch, 0.0f, channel, nullptr};
    _cmdQueue.enqueue(cmd);

    while (!_cmdOutQueue.try_dequeue(cmd) && isRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    };

    assert(cmd.op == SAMPLER_IS_NOTE_PLAYING_PITCH_CMD);
    return (cmd.p1 == 1.0f);
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::clearVoicesCmd(UInt32 channel) {
    for (int i = 0; i < _nbVoices; i++) {
        if (_context.voices[i].channel == channel) {
            _context.voices[i].isNoteOn = false;
            _context.voices[i].data = NULL;
            _context.oldVoices[i].isNoteOn = false;
            _context.oldVoices[i].data = NULL;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::clearVoices(UInt32 channel) {
    SamplerCmd cmd = {SAMPLER_CLEAR_VOICES_CMD, 0.0f, 0.0f, channel, nullptr};
    _cmdQueue.enqueue(cmd);
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::clearAllVoicesCmd() {
    for (int i = 0; i < _nbVoices; i++) {
        _context.voices[i].isNoteOn = false;
        _context.voices[i].data = NULL;
        _context.oldVoices[i].isNoteOn = false;
        _context.oldVoices[i].data = NULL;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::clearAllVoices() {
    SamplerCmd cmd = {SAMPLER_CLEAR_ALL_VOICES_CMD, 0.0f, 0.0f, 0, nullptr};
    _cmdQueue.enqueue(cmd);
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::isMultiInstrumentInUseCmd(std::shared_ptr<MultiInstrument> multiInstrument) {
    bool inUse = false;

    std::vector<std::shared_ptr<Instrument>>::iterator it;
    for (it = multiInstrument->instrumentsBegin(); it != multiInstrument->instrumentsEnd(); it++) {
        std::shared_ptr<Instrument> instrument = (*it);
        for (int voiceIndex = 0; voiceIndex < _nbVoices; voiceIndex++) {
            if (instrument == _context.voices[voiceIndex].instrument ||
                instrument == _context.oldVoices[voiceIndex].instrument) {
                inUse = true;
                break;
            }
        }
        if (inUse) break;
    }

    SamplerCmd cmd = {SAMPLER_IS_MULTI_INSTRUMENT_IN_USE_CMD, inUse ? 1.0f : 0.0f, 0, 0, multiInstrument};
    _cmdOutQueue.enqueue(cmd);
}

// ---------------------------------------------------------------------------------------------------------------------
bool SamplerUnit::isMultiInstrumentInUse(std::shared_ptr<MultiInstrument> multiInstrument) {
    SamplerCmd cmd = {SAMPLER_IS_MULTI_INSTRUMENT_IN_USE_CMD, 0.0f, 0.0f, 0, multiInstrument};
    _cmdQueue.enqueue(cmd);

    while (!_cmdOutQueue.try_dequeue(cmd) && isRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    };

    assert(cmd.op == SAMPLER_IS_MULTI_INSTRUMENT_IN_USE_CMD);
    return (cmd.p1 == 1.0f);
}

// Balance and Levels

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::setLevelCmd(Float32 levelValue, UInt32 channel) { _context.level[channel] = levelValue; }

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::setLevel(Float32 levelValue, UInt32 channel) {
    _levelValues[channel] = levelValue;

    SamplerCmd cmd = {SAMPLER_SET_LEVEL_CMD, powf(levelValue, 2.0f), 0.0f, channel, nullptr};
    _cmdQueue.enqueue(cmd);
}

// ---------------------------------------------------------------------------------------------------------------------
Float32 SamplerUnit::level(UInt32 channel) { return _levelValues[channel]; }

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::setBalanceCmd(Float32 balanceValue, UInt32 channel) { _context.balance[channel] = balanceValue; }

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::setBalance(Float32 balanceValue, UInt32 channel) {
    _balanceValues[channel] = balanceValue;

    SamplerCmd cmd = {SAMPLER_SET_BALANCE_CMD, balanceValue, 0.0f, channel, nullptr};
    _cmdQueue.enqueue(cmd);
}

// ---------------------------------------------------------------------------------------------------------------------
Float32 SamplerUnit::balance(UInt32 channel) { return _balanceValues[channel]; }

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::setReverbCmd(Float32 reverbValue, UInt32 channel) { _context.reverbValues[channel] = reverbValue; }

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::setReverb(Float32 reverbValue, UInt32 channel) {
    _reverbValues[channel] = reverbValue;

    SamplerCmd cmd = {SAMPLER_SET_REVERB_CMD, reverbValue, 0.0f, channel, nullptr};
    _cmdQueue.enqueue(cmd);
}

// ---------------------------------------------------------------------------------------------------------------------
Float32 SamplerUnit::reverb(UInt32 channel) { return _reverbValues[channel]; }

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::setChorusCmd(Float32 chorusValue, UInt32 channel) { _context.chorusValues[channel] = chorusValue; }

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::setChorus(Float32 chorusValue, UInt32 channel) {
    _chorusValues[channel] = chorusValue;

    SamplerCmd cmd = {SAMPLER_SET_CHORUS_CMD, chorusValue, 0.0f, channel, nullptr};
    _cmdQueue.enqueue(cmd);
}

// ---------------------------------------------------------------------------------------------------------------------
Float32 SamplerUnit::chorus(UInt32 channel) { return _chorusValues[channel]; }

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::setExpressionCmd(Float32 expressionValue, UInt32 channel) {
    _context.expressionValues[channel] = expressionValue;
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::setPitchBendFactorCmd(Float32 pitchBendFactor, UInt32 channel) {
    _context.pitchBendFactors[channel] = pitchBendFactor;
}

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::setExpression(Float32 expressionValue, UInt32 channel) {
    _expressionValues[channel] = expressionValue;

    SamplerCmd cmd = {SAMPLER_SET_EXPRESSION_CMD, powf(expressionValue, 2.0f), 0.0f, channel, nullptr};
    _cmdQueue.enqueue(cmd);
}

// ---------------------------------------------------------------------------------------------------------------------
Float32 SamplerUnit::expression(UInt32 channel) { return _expressionValues[channel]; }

// ---------------------------------------------------------------------------------------------------------------------
void SamplerUnit::setPitchBendFactor(Float32 pitchBendFactor, UInt32 channel) {
    _pitchBendFactors[channel] = pitchBendFactor;

    SamplerCmd cmd = {SAMPLER_SET_PITCH_BEND_FACTOR_CMD, pitchBendFactor, 0.0f, channel, nullptr};
    _cmdQueue.enqueue(cmd);
}

// ---------------------------------------------------------------------------------------------------------------------
Float32 SamplerUnit::pitchBendFactor(UInt32 channel) { return _pitchBendFactors[channel]; }
