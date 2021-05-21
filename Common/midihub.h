//
//  MIDIHub.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2021-01-05.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#ifndef MIDIHUB_H
#define MIDIHUB_H

#include <RtMidi.h>

#include <thread>

#include "topviewcontroller.h"

class MIDIHub {
    TopViewController* _topViewController = nullptr;
    RtMidiIn* _midiIn;
    RtMidiOut* _midiOut;

    std::atomic<bool> _stopUpdateMIDIHubThread;
    std::thread _updateMIDIHubThread;

    void updateMIDIHub();

    int _nbMIDIInputPorts, _nbMIDIOutputPorts;
    std::string _defaultMIDIInputPortName, _defaultMIDIOutputPortName;
    std::string _currentMIDIInputPortName, _currentMIDIOutputPortName;

    void studioDidPlayNote(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel, int pitch,
                           Float32 velocity);
    void studioDidReleaseNote(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel,
                              int pitch, Float32 velocity);
    void studioDidSetSustain(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel,
                             Float32 sustain);
    void studioDidSetPitchBend(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel,
                               Float32 pitchBend);
    void studioDidSetModulation(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel,
                                Float32 modulation);
    void studioDidSetInstrument(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel,
                                int instrument);
    void studioDidSetMixerLevel(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel,
                                Float32 mixerLevel);
    void studioDidSetMixerBalance(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel,
                                  Float32 mixerBalance);
    void studioDidSetControlValue(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel,
                                  UInt32 control, UInt32 value);
    void studioDidPerformKeyAftertouch(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel,
                                       int pitch, Float32 value);
    void studioDidPerformChannelAftertouch(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick,
                                           int channel, Float32 value);
    void studioDidSendSystemExclusive(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel,
                                      std::vector<UInt8> data);

    void openMIDIInputPort();
    void openMIDIOutputPort();

    void midiOutSendMessage(std::vector<unsigned char>* message);

   public:
    MIDIHub();
    ~MIDIHub();

    void start(TopViewController* topViewController);

    TopViewController* topViewController() { return _topViewController; }

    std::string defaultMIDIInputPortName() { return _defaultMIDIInputPortName; }
    std::string defaultMIDIOutputPortName() { return _defaultMIDIOutputPortName; }
};

#endif  // MIDIHUB_H
