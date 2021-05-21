//
//  event.h
//  MDStudio
//
//  Created by Daniel Cliche on 10-08-12.
//  Copyright 2010-2020 Daniel Cliche. All rights reserved.
//

#ifndef MDSTUDIO_EVENT_H
#define MDSTUDIO_EVENT_H

#include <vector>

#include "../types.h"

#define EVENT_TYPE_NOP 0
#define EVENT_TYPE_NOTE_ON 1               // 0x90
#define EVENT_TYPE_NOTE_OFF 2              // 0x80
#define EVENT_TYPE_PROGRAM_CHANGE 3        // 0xC0
#define EVENT_TYPE_MIXER_LEVEL_CHANGE 4    // 0xB0, CC: 0x07
#define EVENT_TYPE_MIXER_BALANCE_CHANGE 5  // 0xB0, CC: 0x0A
#define EVENT_TYPE_SUSTAIN 6               // 0xB0, CC: 0x40
#define EVENT_TYPE_PITCH_BEND 7            // 0xE0
#define EVENT_TYPE_MODULATION 8            // 0xB0, CC: 0x01
#define EVENT_TYPE_KEY_AFTERTOUCH 9        // 0xA0
#define EVENT_TYPE_CHANNEL_AFTERTOUCH 10   // 0xD0
#define EVENT_TYPE_CONTROL_CHANGE 11       // 0xB0
#define EVENT_TYPE_SYSTEM_EXCLUSIVE 12     // 0xF0

// META events

#define EVENT_TYPE_META_SET_TEMPO 0xF0       // 0xFF, Meta: 0x51
#define EVENT_TYPE_META_TIME_SIGNATURE 0xF1  // 0xFF, Meta: 0x58
#define EVENT_TYPE_META_END_OF_TRACK 0xF2    // 0xFF, Meta: 0x2F
#define EVENT_TYPE_META_GENERIC 0xFF         // Generic Meta event not included in cases above

namespace MDStudio {

struct Event {
    UInt8 type;
    UInt8 channel;
    UInt32 tickCount;
    SInt32 param1;
    SInt32 param2;
    std::vector<UInt8> data;
};

Event makeEvent(UInt8 type, UInt8 channel, UInt32 tickCount, SInt32 param1, SInt32 param2,
                std::vector<UInt8> data = {});

}  // namespace MDStudio

#endif  // MDSTUDIO_EVENT_H
