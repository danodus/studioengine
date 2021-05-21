//
//  midi.h
//  MDStudio
//
//  Created by Daniel Cliche on 2019-08-23.
//  Copyright (c) 2019-2020 Daniel Cliche. All rights reserved.
//

#ifndef MIDI_H
#define MIDI_H

#include <RtMidi.h>

#include <functional>
#include <string>
#include <vector>

namespace MDStudio {

class MIDIIn {
   public:
    typedef std::function<void(MIDIIn* sender, double deltatime, const std::vector<unsigned char>& message)>
        DidReceiveMessageFnType;

   private:
    RtMidiIn* _rtMidiIn;

   public:
    MIDIIn();
    ~MIDIIn();

    DidReceiveMessageFnType _didReceiveMessageFn = nullptr;

    int getPortCount();
    std::string getPortName(int port);
    void openPort(int port);
    void closePort();

    void setDidReceiveMessageFn(DidReceiveMessageFnType didReceiveMessageFn) {
        _didReceiveMessageFn = didReceiveMessageFn;
    };
    DidReceiveMessageFnType didReceiveMessageFn() { return _didReceiveMessageFn; }
};

}  // namespace MDStudio

#endif  // MIDI_H
