//
//  event.h
//  MelobaseCore
//
//  Created by Daniel Cliche on 2016-08-01.
//  Copyright (c) 2016-2021 Daniel Cliche. All rights reserved.
//

#ifndef MELOBASECORE_EVENT_H
#define MELOBASECORE_EVENT_H

#include <types.h>

#include <memory>
#include <vector>

#define CHANNEL_EVENT_TYPE_NOP 0
#define CHANNEL_EVENT_TYPE_NOTE 1                  // 0x90, 0x80
#define CHANNEL_EVENT_TYPE_NOTE_OFF 2              // 0x80, reserved for conversion to studio sequence
#define CHANNEL_EVENT_TYPE_PROGRAM_CHANGE 3        // 0xC0
#define CHANNEL_EVENT_TYPE_MIXER_LEVEL_CHANGE 4    // 0xB0, CC: 0x07
#define CHANNEL_EVENT_TYPE_MIXER_BALANCE_CHANGE 5  // 0xB0, CC: 0x0A
#define CHANNEL_EVENT_TYPE_SUSTAIN 6               // 0xB0, CC: 0x40
#define CHANNEL_EVENT_TYPE_PITCH_BEND 7            // 0xE0
#define CHANNEL_EVENT_TYPE_MODULATION 8            // 0xB0, CC: 0x01
#define CHANNEL_EVENT_TYPE_KEY_AFTERTOUCH 9        // 0xA0
#define CHANNEL_EVENT_TYPE_CHANNEL_AFTERTOUCH 10   // 0xD0
#define CHANNEL_EVENT_TYPE_CONTROL_CHANGE 11       // 0xB0
#define CHANNEL_EVENT_TYPE_SYSTEM_EXCLUSIVE 12     // 0xF0

// META events

#define CHANNEL_EVENT_TYPE_META_SET_TEMPO 0xF0       // 0xFF, Meta: 0x51
#define CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE 0xF1  // 0xFF, Meta: 0x58
#define CHANNEL_EVENT_TYPE_META_END_OF_TRACK 0xF2    // 0xFF, Meta: 0x2F
#define CHANNEL_EVENT_TYPE_META_GENERIC 0xFF         // Generic Meta event not included in cases above

namespace MelobaseCore {

class Event {
   protected:
    UInt8 _classType;

   public:
    Event(UInt8 classType);

    UInt8 classType() { return _classType; }

    virtual std::vector<char> encode(bool isVLE) = 0;
};

class ChannelEvent : public Event {
    UInt8 _type;
    UInt8 _channel;
    UInt32 _tickCount;
    UInt32 _length;
    SInt32 _param1;
    SInt32 _param2;
    SInt32 _param3;

    std::vector<UInt8> _data;

    void validateEvent();

   public:
    ChannelEvent(UInt8 type, UInt8 channel, UInt32 tickCount, UInt32 length, SInt32 param1 = 0, SInt32 param2 = 0,
                 SInt32 param3 = 0, std::vector<UInt8> data = {});
    ChannelEvent();

    bool isVariableLength();

    UInt8 type() { return _type; }
    void setType(UInt8 type);

    UInt8 channel() { return _channel; }
    void setChannel(UInt8 channel);

    UInt32 tickCount() { return _tickCount; }
    void setTickCount(UInt32 tickCount) { _tickCount = tickCount; }

    UInt32 length() { return _length; }
    void setLength(UInt32 length) { _length = length; }

    SInt32 param1() { return _param1; }
    void setParam1(SInt32 param1) {
        _param1 = param1;
        validateEvent();
    }

    SInt32 param2() { return _param2; }
    void setParam2(SInt32 param2) {
        _param2 = param2;
        validateEvent();
    }

    SInt32 param3() { return _param3; }
    void setParam3(SInt32 param3) {
        _param3 = param3;
        validateEvent();
    }

    std::vector<UInt8>& data() { return _data; }
    void setData(std::vector<UInt8> data) {
        _data = data;
        validateEvent();
    }

    std::vector<char> encode(bool isVLE) override;
    size_t decode(const char* data, size_t eventSize);
};

std::vector<char> encodeEvent(std::shared_ptr<Event> event, bool isVLE);
std::shared_ptr<Event> decodeEvent(const char* data, size_t* size, bool isVLE);
}  // namespace MelobaseCore

#endif  // MELOBASECORE_EVENT_H
