//
//  event.cpp
//  MelobaseCore
//
//  Created by Daniel Cliche on 2016-08-01.
//  Copyright © 2016-2017 Daniel Cliche. All rights reserved.
//

#include "melobasecore_event.h"
#include <platform.h>
#include <assert.h>

using namespace MelobaseCore;

// ---------------------------------------------------------------------------------------------------------------------
Event::Event(UInt8 classType) : _classType(classType)
{
    assert(classType == 0);
}

// ---------------------------------------------------------------------------------------------------------------------
ChannelEvent::ChannelEvent(UInt8 type, UInt8 channel, UInt32 tickCount, UInt32 length, SInt32 param1, SInt32 param2, SInt32 param3, std::vector<UInt8> data) : Event(0), _type(type), _channel(channel), _tickCount(tickCount), _length(length), _param1(param1), _param2(param2), _param3(param3), _data(data)
{
    if (isVariableLength())
        _length = (UInt32)data.size();
    
    validateEvent();
}

// ---------------------------------------------------------------------------------------------------------------------
ChannelEvent::ChannelEvent() : Event(0), _type(0), _channel(0), _tickCount(0), _length(0), _param1(0), _param2(0), _param3(0)
{
}

// ---------------------------------------------------------------------------------------------------------------------
bool ChannelEvent::isVariableLength()
{
    return ((_type == CHANNEL_EVENT_TYPE_SYSTEM_EXCLUSIVE) || (_type == CHANNEL_EVENT_TYPE_META_GENERIC));
}

// ---------------------------------------------------------------------------------------------------------------------
void ChannelEvent::validateEvent()
{
    if ((_type == CHANNEL_EVENT_TYPE_NOTE) || (_type == CHANNEL_EVENT_TYPE_NOTE_OFF)) {
        if (_param1 > 127) {
            _param1 = 127;
        }
        else if (_param1 < 0) {
            _param1 = 0;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ChannelEvent::setType(UInt8 type)
{
    _type = type;
}

// ---------------------------------------------------------------------------------------------------------------------
void ChannelEvent::setChannel(UInt8 channel)
{
    _channel = channel;
}

// ---------------------------------------------------------------------------------------------------------------------
// Note: For variable length events, the length parameter is used to encode the data length
std::vector<char> ChannelEvent::encode(bool isVLE)
{
    if (isVariableLength() && !isVLE)
        return {};
    
    std::vector<char> data;
    data.push_back(_type);
    data.push_back(_channel);
    data.insert(data.end(), (char *)(&_tickCount), (char *)(&_tickCount) + sizeof(_tickCount));
    data.insert(data.end(), (char *)(&_length), (char *)(&_length) + sizeof(_length));
    data.insert(data.end(), (char *)(&_param1), (char *)(&_param1) + sizeof(_param1));
    data.insert(data.end(), (char *)(&_param2), (char *)(&_param2) + sizeof(_param2));
    data.insert(data.end(), (char *)(&_param3), (char *)(&_param3) + sizeof(_param3));
    
    if (isVariableLength()) {
        for (auto c : _data)
            data.push_back(c);
    }
    
    return data;
}

// ---------------------------------------------------------------------------------------------------------------------
size_t ChannelEvent::decode(const char *data, size_t eventSize)
{
    _type = *data; data += sizeof(_type);
    _channel = *data; data += sizeof(_channel);
    _tickCount = *((UInt32 *)data); data += sizeof(_tickCount);
    _length = *((UInt32 *)data); data += sizeof(_length);
    _param1 = *((SInt32 *)data); data += sizeof(_param1);
    _param2 = *((SInt32 *)data); data += sizeof(_param2);
    _param3 = *((SInt32 *)data); data += sizeof(_param3);
    
    size_t dataSize = 0;
    
    if (isVariableLength() && eventSize > 0) {
        // The data size is based on the number of remaining bytes
        dataSize = eventSize - (sizeof(_type) + sizeof(_channel) + sizeof(_tickCount) + sizeof(_length) + sizeof(_param1) + sizeof(_param2) + sizeof(_param3));
        for (UInt8 i = dataSize; i; --i) {
            _data.push_back(*((UInt8 *)data)); data += sizeof(UInt8);
        }
    }

    validateEvent();

    return sizeof(_type) + sizeof(_channel) + sizeof(_tickCount) + sizeof(_length) + sizeof(_param1) + sizeof(_param2) + sizeof(_param3) + dataSize;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<char> MelobaseCore::encodeEvent(std::shared_ptr<Event> event, bool isVLE)
{
    std::vector<char> data;
    
    if (event->classType() == 0) {
        data.push_back(0);
        auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);
        auto channelEventData = channelEvent->encode(isVLE);
        // If the data is empty, the event cannot be encoded
        if (channelEventData.empty())
            return {};
        data.insert(data.end(), channelEventData.begin(), channelEventData.end());
    }
    
    // If VLE, add the size at the beginning
    if (isVLE) {
        
        size_t size = data.size();
        auto lengthData = MDStudio::vlEncode((UInt32)size);
        data.insert(data.begin(), lengthData.begin(), lengthData.end());
    }
    
    return data;
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<Event> MelobaseCore::decodeEvent(const char *data, size_t *size, bool isVLE)
{
    *size = 0;
    
    size_t vleDecodedSize = 0;
    size_t vleSize = 0;
    
    if (isVLE) {
        
        UInt8 n;
        do {
            vleDecodedSize <<= 7;
            n = *data; ++data;
            vleDecodedSize |= (n & 0x7f);
            ++vleSize;
        } while (n & 0x80);
    }
    
    UInt8 classType = *data;
    data += sizeof(classType);
    
    if (classType == 0) {
        auto channelEvent = std::make_shared<ChannelEvent>();
        auto channelEventSize = channelEvent->decode(data, isVLE ? (vleDecodedSize - sizeof(classType)) : 0);
        if (!isVLE) {
            *size = sizeof(classType) + channelEventSize;
        } else {
            *size = vleSize + vleDecodedSize;
        }
        return channelEvent;
    }
    return nullptr;
}

