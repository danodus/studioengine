//
//  MIDIHub.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2021-01-05.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#include "midihub.h"

// ---------------------------------------------------------------------------------------------------------------------
void midiInCallback(double deltatime, std::vector<unsigned char>* message, void* userData) {
    MIDIHub* midiHub = static_cast<MIDIHub*>(userData);
    size_t nBytes = message->size();

    /*
        for ( unsigned int i=0; i<nBytes; i++ )
          std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
        if ( nBytes > 0 )
          std::cout << "stamp = " << deltatime << std::endl;
    */

    if (nBytes >= 2) {
        int type = message->at(0) & 0xF0;
        int channel = message->at(0) & 0x0F;
        if (type == 0x90) {
            // Note on
            int pitch = message->at(1);
            // If the velocity is zero, we send a note off
            if (message->at(2) == 0) {
                midiHub->topViewController()->noteOff(channel, pitch, 0.5f);
            } else {
                // Send a note on
                float velocity = (float)(message->at(2)) / 127.0f;
                midiHub->topViewController()->noteOn(channel, pitch, velocity);
            }
        } else if (type == 0x80) {
            // Note off
            int pitch = message->at(1);
            float velocity = (float)(message->at(2)) / 127.0f;
            midiHub->topViewController()->noteOff(channel, pitch, velocity);
        } else if (type == 0xA0) {
            // Key aftertouch
            int pitch = message->at(1);
            float value = (float)(message->at(2)) / 127.0f;
            midiHub->topViewController()->keyAftertouch(channel, pitch, value);
        } else if (type == 0xE0) {
            // Pitch bend
            UInt32 value = (UInt32)message->at(1) | (UInt32)message->at(2) << 7;
            midiHub->topViewController()->pitchBendChanged(channel, 4.0f * ((Float32)value - 0.5f) / 16383.0f - 2.0f);
        } else if (type == 0xC0) {
            // Program change
            int program = message->at(1);
            midiHub->topViewController()->programChange(channel, program);
        } else if (type == 0xD0) {
            // Channel aftertouch
            float value = (float)(message->at(1)) / 127.0f;
            midiHub->topViewController()->channelAftertouch(channel, value);
        } else if (type == 0xB0) {
            // Controller
            if (message->at(1) == 0x40) {
                // Damper pedal
                int sustain = message->at(2);
                midiHub->topViewController()->sustainChanged(channel, (Float32)sustain / 127.0f);
            } else if (message->at(1) == 0x01) {
                // Modulation wheel
                int modulation = message->at(2);
                midiHub->topViewController()->modulationChanged(channel, (Float32)modulation / 127.0f);
            } else if (message->at(1) == 0x07) {
                // Volume
                int volume = message->at(2);
                midiHub->topViewController()->volumeChanged(channel, (Float32)volume / 127.0f);
            } else if (message->at(1) == 0x0A) {
                // Pan
                int pan = message->at(2);
                midiHub->topViewController()->panChanged(channel, 2.0f * ((Float32)pan / 127.0f) - 1.0f);
            } else if ((message->at(1) == 0x72) || (message->at(1) == 0x73)) {
                // Start/stop button
                int value = message->at(2);
                if (value < 64) {
                    MDStudio::Platform::sharedInstance()->invoke([=]() { midiHub->topViewController()->record(); });
                }
            } else {
                // Control change
                int controller = message->at(1);
                int value = message->at(2);
                midiHub->topViewController()->controlChanged(channel, controller, value);
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
MIDIHub::MIDIHub() {
    _nbMIDIInputPorts = -1;
    _midiIn = new RtMidiIn();
    int nPorts = static_cast<int>(_midiIn->getPortCount());
    if (nPorts > 0) _defaultMIDIInputPortName = _midiIn->getPortName(nPorts - 1);

    _nbMIDIOutputPorts = -1;
    _midiOut = new RtMidiOut();

    nPorts = static_cast<int>(_midiOut->getPortCount());
    if (nPorts > 0) _defaultMIDIOutputPortName = _midiOut->getPortName(nPorts - 1);

    _stopUpdateMIDIHubThread = false;
}

// ---------------------------------------------------------------------------------------------------------------------
void MIDIHub::start(TopViewController* topViewController) {
    _topViewController = topViewController;

    // Subscribe to studio notifications
    using namespace std::placeholders;
    auto studioController = topViewController->studioController();
    studioController->studio()->setMIDIOutputDidPlayNoteFn(
        std::bind(&MIDIHub::studioDidPlayNote, this, _1, _2, _3, _4, _5, _6, _7));
    studioController->studio()->setMIDIOutputDidReleaseNoteFn(
        std::bind(&MIDIHub::studioDidReleaseNote, this, _1, _2, _3, _4, _5, _6, _7));
    studioController->studio()->setMIDIOutputDidSetSustainFn(
        std::bind(&MIDIHub::studioDidSetSustain, this, _1, _2, _3, _4, _5, _6));
    studioController->studio()->setMIDIOutputDidSetPitchBendFn(
        std::bind(&MIDIHub::studioDidSetPitchBend, this, _1, _2, _3, _4, _5, _6));
    studioController->studio()->setMIDIOutputDidSetModulationFn(
        std::bind(&MIDIHub::studioDidSetModulation, this, _1, _2, _3, _4, _5, _6));
    studioController->studio()->setMIDIOutputDidSetInstrumentFn(
        std::bind(&MIDIHub::studioDidSetInstrument, this, _1, _2, _3, _4, _5, _6));
    studioController->studio()->setMIDIOutputDidSetMixerLevelFn(
        std::bind(&MIDIHub::studioDidSetMixerLevel, this, _1, _2, _3, _4, _5, _6));
    studioController->studio()->setMIDIOutputDidSetMixerBalanceFn(
        std::bind(&MIDIHub::studioDidSetMixerBalance, this, _1, _2, _3, _4, _5, _6));
    studioController->studio()->setMIDIOutputDidSetControlValueFn(
        std::bind(&MIDIHub::studioDidSetControlValue, this, _1, _2, _3, _4, _5, _6, _7));
    studioController->studio()->setMIDIOutputDidPerformKeyAftertouchFn(
        std::bind(&MIDIHub::studioDidPerformKeyAftertouch, this, _1, _2, _3, _4, _5, _6, _7));
    studioController->studio()->setMIDIOutputDidPerformChannelAftertouchFn(
        std::bind(&MIDIHub::studioDidPerformChannelAftertouch, this, _1, _2, _3, _4, _5, _6));
    studioController->studio()->setMIDIOutputDidSendSystemExclusiveFn(
        std::bind(&MIDIHub::studioDidSendSystemExclusive, this, _1, _2, _3, _4, _5, _6));

    // Start update thread
    _updateMIDIHubThread = std::thread(&MIDIHub::updateMIDIHub, this);
}

// ---------------------------------------------------------------------------------------------------------------------
MIDIHub::~MIDIHub() {
    if (_updateMIDIHubThread.joinable()) {
        _stopUpdateMIDIHubThread = true;
        _updateMIDIHubThread.join();
    }
    
    MDStudio::Platform::sharedInstance()->cancelDelayedInvokes(this);

    if (_midiIn->isPortOpen()) _midiIn->closePort();

    if (_midiOut->isPortOpen()) _midiOut->closePort();

    delete _midiIn;
    delete _midiOut;
}

// ---------------------------------------------------------------------------------------------------------------------
void MIDIHub::openMIDIInputPort() {
    if (_topViewController->midiInputPortName() == _currentMIDIInputPortName) return;

    if (_midiIn->isPortOpen()) {
        _midiIn->cancelCallback();
        _midiIn->closePort();
    }

    _currentMIDIInputPortName = "";
    _topViewController->setIsMIDIDeviceAvailable(false);

    int nPorts = static_cast<int>(_midiIn->getPortCount());
    for (int port = 0; port < nPorts; ++port) {
        if (std::string(_midiIn->getPortName(port)) == _topViewController->midiInputPortName()) {
            _midiIn->openPort(port);
            // Set our callback function.  This should be done immediately after
            // opening the port to avoid having incoming messages written to the
            // queue.
            _midiIn->setCallback(&midiInCallback, this);
            // Don't ignore sysex, timing, or active sensing messages.
            _midiIn->ignoreTypes(false, false, false);

            std::cout << "MIDI input port " << _topViewController->midiInputPortName() << " opened" << std::endl;
            _topViewController->setIsMIDIDeviceAvailable(true);
            _currentMIDIInputPortName = _topViewController->midiInputPortName();
            break;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MIDIHub::openMIDIOutputPort() {
    if (_topViewController->midiOutputPortName() == _currentMIDIOutputPortName) return;

    if (_midiOut->isPortOpen()) {
        _midiOut->closePort();
    }

    _currentMIDIOutputPortName = "";

    int nPorts = static_cast<int>(_midiOut->getPortCount());
    for (int port = 0; port < nPorts; ++port) {
        if (std::string(_midiOut->getPortName(port)) == _topViewController->midiOutputPortName()) {
            _midiOut->openPort(port);
            std::cout << "MIDI output port " << _topViewController->midiOutputPortName() << " opened" << std::endl;
            _currentMIDIOutputPortName = _topViewController->midiOutputPortName();
            break;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MIDIHub::updateMIDIHub() {
    while (!_stopUpdateMIDIHubThread) {
        MDStudio::Platform::sharedInstance()->invoke([&]() {
            //
            // MIDI input
            //

            // Check available ports.
            int nPorts = static_cast<int>(_midiIn->getPortCount());

            if (_nbMIDIInputPorts != nPorts) {
                _nbMIDIInputPorts = nPorts;

                if (nPorts == 0) {
                    std::cout << "No MIDI input ports available!" << std::endl;
                    _topViewController->setIsMIDIDeviceAvailable(false);
                }

                std::vector<std::string> portNames;
                for (int i = 0; i < nPorts; ++i) portNames.push_back(_midiIn->getPortName(i));
                _topViewController->setMIDIInputPortNames(portNames);
            }

            // Open a new input port if necessary
            openMIDIInputPort();

            //
            // MIDI output
            //

            // Check available ports.
            nPorts = static_cast<int>(_midiOut->getPortCount());

            if (_nbMIDIOutputPorts != nPorts) {
                _nbMIDIOutputPorts = nPorts;

                if (nPorts == 0) {
                    std::cout << "No MIDI output ports available!" << std::endl;
                }

                std::vector<std::string> portNames;
                for (int i = 0; i < nPorts; ++i) portNames.push_back(_midiOut->getPortName(i));
                _topViewController->setMIDIOutputPortNames(portNames);
            }

            // Open a new output port if necessary
            openMIDIOutputPort();
        }, this);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MIDIHub::midiOutSendMessage(std::vector<unsigned char>* message) {
    /*
    printf("MIDI OUT: ");
    for (auto c : *message)
        printf("%02x ", c);
    printf("\n");
    */

    _midiOut->sendMessage(message);
}

// ---------------------------------------------------------------------------------------------------------------------
void MIDIHub::studioDidPlayNote(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel,
                                int pitch, Float32 velocity) {
    if (_midiOut->isPortOpen()) {
        std::vector<unsigned char> message;
        message.push_back(0x90 | channel);
        message.push_back(pitch);
        message.push_back(velocity * 127);
        midiOutSendMessage(&message);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MIDIHub::studioDidReleaseNote(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel,
                                   int pitch, Float32 velocity) {
    if (_midiOut->isPortOpen()) {
        std::vector<unsigned char> message;
        message.push_back(0x80 | channel);
        message.push_back(pitch);
        message.push_back(velocity * 127);
        midiOutSendMessage(&message);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MIDIHub::studioDidSetSustain(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel,
                                  Float32 sustain) {
    if (_midiOut->isPortOpen()) {
        std::vector<unsigned char> message;
        message.push_back(0xB0 | channel);
        message.push_back(0x40);
        message.push_back(sustain * 127);
        midiOutSendMessage(&message);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MIDIHub::studioDidSetPitchBend(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel,
                                    Float32 pitchBend) {
    if (_midiOut->isPortOpen()) {
        UInt32 value = (pitchBend + 2.0f) * 16383.0f / 4.0f + 0.5f;
        std::vector<unsigned char> message;
        message.push_back(0xE0 | channel);
        message.push_back(value & 0x7F);
        message.push_back(value >> 7);
        midiOutSendMessage(&message);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MIDIHub::studioDidSetModulation(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel,
                                     Float32 modulation) {
    if (_midiOut->isPortOpen()) {
        std::vector<unsigned char> message;
        message.push_back(0xB0 | channel);
        message.push_back(0x01);
        message.push_back(modulation * 127);
        midiOutSendMessage(&message);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MIDIHub::studioDidSetInstrument(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel,
                                     int instrument) {
    if (_midiOut->isPortOpen()) {
        std::vector<unsigned char> message;
        message.push_back(0xC0 | channel);
        message.push_back(instrument);
        midiOutSendMessage(&message);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MIDIHub::studioDidSetMixerLevel(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel,
                                     Float32 mixerLevel) {
    if (_midiOut->isPortOpen()) {
        std::vector<unsigned char> message;

        // Volume
        message.push_back(0xB0 | channel);
        message.push_back(0x07);
        message.push_back(mixerLevel * 127);
        midiOutSendMessage(&message);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MIDIHub::studioDidSetMixerBalance(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel,
                                       Float32 mixerBalance) {
    if (_midiOut->isPortOpen()) {
        std::vector<unsigned char> message;

        // Pan
        message.push_back(0xB0 | channel);
        message.push_back(0x0A);
        message.push_back((mixerBalance + 1.0f) / 2.0f * 127);
        midiOutSendMessage(&message);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MIDIHub::studioDidSetControlValue(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick, int channel,
                                       UInt32 control, UInt32 value) {
    if (_midiOut->isPortOpen()) {
        std::vector<unsigned char> message;

        // CC
        message.push_back(0xB0 | channel);
        message.push_back(control);
        message.push_back(value);
        midiOutSendMessage(&message);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MIDIHub::studioDidPerformKeyAftertouch(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick,
                                            int channel, int pitch, Float32 value) {
    if (_midiOut->isPortOpen()) {
        std::vector<unsigned char> message;

        // Key aftertouch
        message.push_back(0xA0 | channel);
        message.push_back(pitch);
        message.push_back(value * 127);
        midiOutSendMessage(&message);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MIDIHub::studioDidPerformChannelAftertouch(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick,
                                                int channel, Float32 value) {
    if (_midiOut->isPortOpen()) {
        std::vector<unsigned char> message;

        // Channel aftertouch
        message.push_back(0xD0 | channel);
        message.push_back(value * 127);
        midiOutSendMessage(&message);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MIDIHub::studioDidSendSystemExclusive(MDStudio::Studio* studio, int source, double timestamp, UInt32 tick,
                                           int channel, std::vector<UInt8> data) {
    if (_midiOut->isPortOpen()) {
        std::vector<unsigned char> message;

        // Channel aftertouch
        message.push_back(0xF0 | channel);
        for (auto v : data) message.push_back(v);
        midiOutSendMessage(&message);
    }
}
