//
//  midi.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2019-08-23.
//  Copyright (c) 2019 Daniel Cliche. All rights reserved.
//

#include "midi.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
MIDIIn::MIDIIn() { _rtMidiIn = new RtMidiIn(); }

// ---------------------------------------------------------------------------------------------------------------------
MIDIIn::~MIDIIn() {
    if (_rtMidiIn->isPortOpen()) {
        _rtMidiIn->cancelCallback();
        _rtMidiIn->closePort();
    }
    delete _rtMidiIn;
}

// ---------------------------------------------------------------------------------------------------------------------
int MIDIIn::getPortCount() { return _rtMidiIn->getPortCount(); }

// ---------------------------------------------------------------------------------------------------------------------
std::string MIDIIn::getPortName(int port) { return _rtMidiIn->getPortName(port); }

// ---------------------------------------------------------------------------------------------------------------------
void MIDIIn::openPort(int port) {
    _rtMidiIn->setCallback(
        [](double deltatime, std::vector<unsigned char>* message, void* userData) {
            auto midiIn = static_cast<MIDIIn*>(userData);
            auto didReceiveMessageFn = midiIn->didReceiveMessageFn();
            if (didReceiveMessageFn) {
                didReceiveMessageFn(midiIn, deltatime, *message);
            }
        },
        this);
    _rtMidiIn->openPort(port);
}

// ---------------------------------------------------------------------------------------------------------------------
void MIDIIn::closePort() { _rtMidiIn->closePort(); }
