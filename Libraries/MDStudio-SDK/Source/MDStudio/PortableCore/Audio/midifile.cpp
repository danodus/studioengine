//
//  midifile.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 10-10-22.
//  Copyright 2010-2020 Daniel Cliche. All rights reserved.
//

#include "midifile.h"

#include <math.h>
#include <string.h>

#include <fstream>
#include <iostream>

#include "platform.h"
#include "studio.h"
#include "ustring.h"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace MDStudio;

struct InvalidMIDIFileException : public std::exception {};

// ---------------------------------------------------------------------------------------------------------------------
std::vector<UInt8> midiTrackDataFromSequence(std::shared_ptr<Sequence> sequence, int trackIndex) {
    std::vector<UInt8> data;
    std::vector<UInt8> trackHeaderData;
    std::vector<UInt8> trackData;

    //
    // Track data
    //

    UInt32 deltaTime = 0;
    UInt8 bytes[16];
    bool isEndOfTrackEventAdded = false;

    //
    // Add the MIDI channel prefix if set
    //

    if (sequence->data.tracks[trackIndex].channel != SEQUENCE_TRACK_MULTI_CHANNEL) {
        // Delta time of zero (VLE)
        trackData.push_back(0);

        // Meta event
        trackData.push_back(0xFF);

        // MIDI channel prefix
        trackData.push_back(0x20);

        // Length (VLE)
        auto vlLength = vlEncode(1);
        trackData.insert(trackData.end(), vlLength.begin(), vlLength.end());

        // Channel
        trackData.push_back(sequence->data.tracks[trackIndex].channel);
    }

    //
    // Add the name of the track if set
    //

    auto trackName = UString(sequence->data.tracks[trackIndex].name);
    trackName.toAscii();
    if (!trackName.isEmpty()) {
        // Delta time of zero (VLE)
        trackData.push_back(0);

        // Meta event
        trackData.push_back(0xFF);

        // Track name
        trackData.push_back(0x03);

        // Length (VLE)
        auto vlLength = vlEncode((UInt32)(trackName.length()));
        trackData.insert(trackData.end(), vlLength.begin(), vlLength.end());

        // String
        auto name = trackName.string();
        trackData.insert(trackData.end(), name.begin(), name.end());
    }

    for (auto event : sequence->data.tracks[trackIndex].events) {
        //
        // Delta time
        //

        deltaTime += event.tickCount;

        // We check if the event is handled
        if (event.type != EVENT_TYPE_NOTE_ON && event.type != EVENT_TYPE_NOTE_OFF && event.type != EVENT_TYPE_SUSTAIN &&
            event.type != EVENT_TYPE_PITCH_BEND && event.type != EVENT_TYPE_PROGRAM_CHANGE &&
            event.type != EVENT_TYPE_KEY_AFTERTOUCH && event.type != EVENT_TYPE_CHANNEL_AFTERTOUCH &&
            event.type != EVENT_TYPE_SYSTEM_EXCLUSIVE && event.type != EVENT_TYPE_MIXER_LEVEL_CHANGE &&
            event.type != EVENT_TYPE_MIXER_BALANCE_CHANGE && event.type != EVENT_TYPE_MODULATION &&
            event.type != EVENT_TYPE_CONTROL_CHANGE && event.type != EVENT_TYPE_META_SET_TEMPO &&
            event.type != EVENT_TYPE_META_TIME_SIGNATURE && event.type != EVENT_TYPE_META_GENERIC &&
            event.type != EVENT_TYPE_META_END_OF_TRACK)
            continue;

        // Add delta time (VLE)
        auto vlDeltaTime = vlEncode(deltaTime);
        trackData.insert(trackData.end(), vlDeltaTime.begin(), vlDeltaTime.end());

        //
        // Event
        //

        switch (event.type) {
            case EVENT_TYPE_NOTE_ON:
                bytes[0] = 0x90 | event.channel;
                bytes[1] = event.param1;
                bytes[2] = (event.param2 >= 0) ? event.param2 : 100;
                for (std::size_t i = 0; i < 3; i++) trackData.push_back(bytes[i]);
                break;

            case EVENT_TYPE_NOTE_OFF:
                bytes[0] = 0x80 | event.channel;
                bytes[1] = event.param1;
                bytes[2] = (event.param2 >= 0) ? event.param2 : 127;
                for (std::size_t i = 0; i < 3; i++) trackData.push_back(bytes[i]);
                break;

            case EVENT_TYPE_KEY_AFTERTOUCH:
                bytes[0] = 0xA0 | event.channel;
                bytes[1] = event.param1;
                bytes[2] = event.param2;
                for (std::size_t i = 0; i < 3; i++) trackData.push_back(bytes[i]);
                break;

            case EVENT_TYPE_SUSTAIN:
                bytes[0] = 0xB0 | event.channel;
                bytes[1] = 0x40;  // Damper pedal
                bytes[2] = event.param1;
                for (std::size_t i = 0; i < 3; i++) trackData.push_back(bytes[i]);
                break;

            case EVENT_TYPE_PITCH_BEND:
                bytes[0] = 0xE0 | event.channel;
                bytes[1] = event.param1 & 0x7f;
                bytes[2] = event.param1 >> 7;
                for (std::size_t i = 0; i < 3; i++) trackData.push_back(bytes[i]);
                break;

            case EVENT_TYPE_MIXER_LEVEL_CHANGE:
                bytes[0] = 0xB0 | event.channel;  // Controller event
                bytes[1] = 0x07;                  // Main volume
                bytes[2] = (event.param2 == 0) ? event.param1 : (event.param1 * 127 / 100);
                for (std::size_t i = 0; i < 3; i++) trackData.push_back(bytes[i]);
                break;

            case EVENT_TYPE_MIXER_BALANCE_CHANGE:
                bytes[0] = 0xB0 | event.channel;  // Controller event
                bytes[1] = 0x0A;                  // Pan
                if (event.param2 == 0) {
                    bytes[2] = event.param1;
                } else {
                    bytes[2] = 64 + (event.param1 * ((event.param1 < 0) ? 64 : 63) / 100);
                }
                for (std::size_t i = 0; i < 3; i++) trackData.push_back(bytes[i]);
                break;

            case EVENT_TYPE_MODULATION:
                bytes[0] = 0xB0 | event.channel;  // Controller event
                bytes[1] = 0x01;                  // Modulation wheel
                bytes[2] = event.param1;
                for (std::size_t i = 0; i < 3; i++) trackData.push_back(bytes[i]);
                break;

            case EVENT_TYPE_CONTROL_CHANGE:  // Controller event
                bytes[0] = 0xB0 | event.channel;
                bytes[1] = event.param1;
                bytes[2] = event.param2;
                for (std::size_t i = 0; i < 3; i++) trackData.push_back(bytes[i]);
                break;

            case EVENT_TYPE_PROGRAM_CHANGE:
                bytes[0] = 0xC0 | event.channel;
                bytes[1] = STUDIO_PRESET_NUMBER_FROM_INSTRUMENT(event.param1);
                for (std::size_t i = 0; i < 2; i++) trackData.push_back(bytes[i]);
                break;

            case EVENT_TYPE_CHANNEL_AFTERTOUCH:
                bytes[0] = 0xD0 | event.channel;
                bytes[1] = event.param1;
                for (std::size_t i = 0; i < 2; i++) trackData.push_back(bytes[i]);
                break;

            case EVENT_TYPE_SYSTEM_EXCLUSIVE:
                bytes[0] = 0xF0 | event.channel;
                trackData.push_back(bytes[0]);
                {
                    auto vlLength = vlEncode((UInt32)(event.data.size()));
                    trackData.insert(trackData.end(), vlLength.begin(), vlLength.end());
                }
                trackData.insert(trackData.end(), event.data.begin(), event.data.end());
                break;

            case EVENT_TYPE_META_SET_TEMPO:
                bytes[0] = 0xFF;
                bytes[1] = 0x51;  // Set tempo
                bytes[2] = 3;     // length
                bytes[3] = event.param1 >> 16;
                bytes[4] = (event.param1 >> 8) & 0xFF;
                bytes[5] = event.param1 & 0xFF;
                for (std::size_t i = 0; i < 6; i++) trackData.push_back(bytes[i]);
                break;

            case EVENT_TYPE_META_TIME_SIGNATURE:
                bytes[0] = 0xFF;
                bytes[1] = 0x58;  // Time signature
                bytes[2] = 4;     // length
                bytes[3] = event.param1;
                bytes[4] = log2(event.param2);  // denominator
                bytes[5] = 24;
                bytes[6] = 8;
                for (std::size_t i = 0; i < 7; i++) trackData.push_back(bytes[i]);
                break;

            case EVENT_TYPE_META_END_OF_TRACK:
                bytes[0] = 0xFF;
                bytes[1] = 0x2F;  // End Of Track
                bytes[2] = 0;
                for (std::size_t i = 0; i < 3; i++) trackData.push_back(bytes[i]);
                isEndOfTrackEventAdded = true;
                break;

            case EVENT_TYPE_META_GENERIC:
                bytes[0] = 0xFF;
                bytes[1] = event.param1;
                for (std::size_t i = 0; i < 2; i++) trackData.push_back(bytes[i]);
                {
                    auto vlLength = vlEncode((UInt32)(event.data.size()));
                    trackData.insert(trackData.end(), vlLength.begin(), vlLength.end());
                }
                trackData.insert(trackData.end(), event.data.begin(), event.data.end());
                break;

        }  // switch event type

        // We reset the delta time
        deltaTime = 0;

    }  // for each event in the sequence

    // If we did not yet add an end of track event, we do so in order to comply with the standard
    if (!isEndOfTrackEventAdded) {
        bytes[0] = 0;
        bytes[1] = 0xFF;
        bytes[2] = 0x2F;  // End Of Track
        bytes[3] = 0;
        for (std::size_t i = 0; i < 4; i++) trackData.push_back(bytes[i]);
    }

    //
    // Track Header
    //

    // ID
    const char* trackHeaderID = "MTrk";
    for (std::size_t i = 0; i < 4; i++) trackHeaderData.push_back(trackHeaderID[i]);

    // size
    UInt32 trackDataSize = swapUInt32HostToBig(static_cast<UInt32>(trackData.size()));
    UInt8* p = reinterpret_cast<UInt8*>(&trackDataSize);
    for (std::size_t i = 0; i < 4; i++) trackHeaderData.push_back(p[i]);

    // We append the track header and track
    data.insert(data.end(), trackHeaderData.begin(), trackHeaderData.end());
    data.insert(data.end(), trackData.begin(), trackData.end());

    return data;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<UInt8> MDStudio::midiFileDataFromSequence(std::shared_ptr<Sequence> sequence) {
    std::vector<UInt8> data;
    UInt8* p;

    //
    // Header
    //

    // ID
    const char* headerID = "MThd";
    for (std::size_t i = 0; i < 4; i++) data.push_back(headerID[i]);

    // size
    UInt32 headerSize = swapUInt32HostToBig(6);
    p = reinterpret_cast<UInt8*>(&headerSize);
    data.push_back(*p);
    p++;
    data.push_back(*p);
    p++;
    data.push_back(*p);
    p++;
    data.push_back(*p);

    // format
    UInt16 headerFormat = swapUInt16HostToBig((sequence->data.format == SEQUENCE_DATA_FORMAT_SINGLE_TRACK) ? 0 : 1);
    p = reinterpret_cast<UInt8*>(&headerFormat);
    data.push_back(*p);
    p++;
    data.push_back(*p);

    // number of tracks
    UInt16 headerNbTracks = swapUInt16HostToBig(sequence->data.tracks.size());
    p = reinterpret_cast<UInt8*>(&headerNbTracks);
    data.push_back(*p);
    p++;
    data.push_back(*p);

    // time division in ticks per beat
    UInt16 headerTimeDivision = swapUInt16HostToBig((UInt16)(60.0f / (sequence->data.tickPeriod * 125.0f)));
    p = reinterpret_cast<UInt8*>(&headerTimeDivision);
    data.push_back(*p);
    p++;
    data.push_back(*p);

    std::vector<UInt8> trackData;

    for (int trackIndex = 0; trackIndex < sequence->data.tracks.size(); ++trackIndex) {
        trackData = midiTrackDataFromSequence(sequence, trackIndex);
        data.insert(data.end(), trackData.begin(), trackData.end());
    }

    return data;
}

// ---------------------------------------------------------------------------------------------------------------------
bool MDStudio::writeMIDIFile(std::string path, std::shared_ptr<Sequence> sequence) {
    std::vector<UInt8> data = midiFileDataFromSequence(sequence);

#ifdef _WIN32

    std::wstring path16;
    utf8::unchecked::utf8to16(path.begin(), path.end(), back_inserter(path16));

    HANDLE hFile = CreateFileW(path16.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) return false;

    DWORD nbBytesWritten = 0;
    if (WriteFile(hFile, &data[0], data.size(), &nbBytesWritten, NULL) != TRUE) {
        CloseHandle(hFile);
        return false;
    }

    CloseHandle(hFile);

    return true;

#else  // _WIN32

    std::ofstream ofs(path, std::ofstream::out | std::ofstream::binary);
    if (!ofs.is_open()) return false;

    for (UInt8 c : data) ofs << c;
    ofs.close();

    return true;

#endif  // _WIN32
}

// ---------------------------------------------------------------------------------------------------------------------
static void copy(void* dest, size_t destSize, std::vector<UInt8>& data, size_t startIndex, size_t size) {
    if (size > destSize) throw InvalidMIDIFileException();

    if (startIndex >= data.size()) throw InvalidMIDIFileException();

    if (startIndex + size > data.size()) throw InvalidMIDIFileException();

    for (size_t i = 0; i < size; ++i) ((UInt8*)dest)[i] = data[startIndex + i];
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<Sequence> MDStudio::readMIDIFile(const std::string& path) {
    std::streampos fileSize;

    std::ifstream ifs(path, std::ifstream::in | std::ifstream::binary);
    if (!ifs.is_open()) return nullptr;

    ifs.seekg(0, std::ios::end);
    fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::vector<UInt8> data(fileSize);
    ifs.read((char*)&data[0], fileSize);
    ifs.close();

    std::shared_ptr<Sequence> sequence = std::shared_ptr<Sequence>(new Sequence);

    size_t loc = 0;

    try {
        //
        // Header
        //

        // ID
        char headerID[4];

        copy(headerID, sizeof(headerID), data, loc, sizeof(headerID));
        loc += 4;

        if (strncmp(headerID, "MThd", 4) != 0) {
            // Invalid header
            return nullptr;
        }

        // size
        UInt32 headerSize;

        copy(&headerSize, sizeof(headerSize), data, loc, sizeof(headerSize));
        loc += sizeof(headerSize);
        headerSize = swapUInt32HostToBig(headerSize);
        if (headerSize != 6) {
            // Invalid header size
            return nullptr;
        }

        // format
        UInt16 headerFormat;
        copy(&headerFormat, sizeof(headerFormat), data, loc, sizeof(headerFormat));
        loc += sizeof(headerFormat);
        headerFormat = swapUInt16HostToBig(headerFormat);
        if (headerFormat == 0) {
            sequence->data.format = SEQUENCE_DATA_FORMAT_SINGLE_TRACK;
        } else if (headerFormat == 1) {
            sequence->data.format = SEQUENCE_DATA_FORMAT_MULTI_TRACK;
        } else {
            // Unhandled format
            return nullptr;
        }

        // number of tracks
        UInt16 headerNbTracks;
        copy(&headerNbTracks, sizeof(headerNbTracks), data, loc, sizeof(headerNbTracks));
        loc += sizeof(headerNbTracks);
        headerNbTracks = swapUInt16HostToBig(headerNbTracks);

        // time division in ticks per beat
        UInt16 headerTimeDivision;
        copy(&headerTimeDivision, sizeof(headerTimeDivision), data, loc, sizeof(headerTimeDivision));
        loc += sizeof(headerTimeDivision);
        headerTimeDivision = swapUInt16HostToBig(headerTimeDivision);

        sequence->data.tickPeriod = 60.0f / ((float)headerTimeDivision * 125.0f);

        std::vector<Event> events;
        std::vector<Event> mergedEvents;

        sequence->data.tracks.resize(headerNbTracks);

        // For each track
        for (UInt16 track = 0; track < headerNbTracks; ++track) {
            //
            // Track Header
            //

            // ID
            char trackHeaderID[4];
            copy(&trackHeaderID, sizeof(trackHeaderID), data, loc, 4);
            loc += 4;
            if (strncmp(trackHeaderID, "MTrk", 4) != 0) {
                // Invalid track header
                return nullptr;
            }

            // size
            UInt32 trackSize;
            copy(&trackSize, sizeof(trackSize), data, loc, sizeof(trackSize));
            loc += sizeof(trackSize);
            trackSize = swapUInt32HostToBig(trackSize);

            UInt8 lastEventCode = 0;

            // UInt32 tickCount = 0;

            while (trackSize) {
                // Handle cases of corrupted files
                if (loc >= fileSize) return nullptr;

                //
                // Delta time
                //

                UInt32 deltaTime = 0;
                UInt8 n;
                do {
                    deltaTime <<= 7;
                    copy(&n, sizeof(n), data, loc, sizeof(n));
                    loc += sizeof(n);
                    trackSize -= sizeof(n);
                    deltaTime |= (n & 0x7f);
                } while (n & 0x80);

                //
                // Event
                //

                Event event;

                UInt8 eventCode;
                copy(&eventCode, sizeof(eventCode), data, loc, sizeof(eventCode));
                loc += sizeof(eventCode);
                trackSize -= sizeof(eventCode);

                if (eventCode == 0xFF) {
                    // Meta event

                    UInt8 type;
                    copy(&type, sizeof(type), data, loc, sizeof(type));
                    loc += sizeof(type);
                    trackSize -= sizeof(type);

                    UInt32 length = 0;
                    UInt8 n;
                    do {
                        length <<= 7;
                        copy(&n, sizeof(n), data, loc, sizeof(n));
                        loc += sizeof(n);
                        trackSize -= sizeof(n);
                        length |= (n & 0x7f);
                    } while (n & 0x80);

                    switch (type) {
                        case 0x20: {
                            // Track channel
                            sequence->data.tracks[track].channel = data[loc];
                            ++loc;
                            --trackSize;
                        } break;

                        case 0x03: {
                            // Track name
                            for (UInt32 i = 0; i < length; ++i) {
                                sequence->data.tracks[track].name += data[loc];
                                ++loc;
                                --trackSize;
                            }
                        } break;
                        case 0x51: {
                            // Tempo
                            UInt32 mpqn;  // Microseconds per quarter note
                            UInt8 mpqn_n[3];
                            copy(mpqn_n, sizeof(mpqn_n), data, loc, 3);
                            loc += 3;
                            trackSize -= 3;
                            mpqn = (mpqn_n[0] << 16) | (mpqn_n[1] << 8) | mpqn_n[2];
                            event = makeEvent(EVENT_TYPE_META_SET_TEMPO, 0, deltaTime, mpqn, -1);
                            events.push_back(event);
                        } break;

                        case 0x58: {
                            // Time signature
                            UInt8 timeSignature[4];
                            copy(timeSignature, sizeof(timeSignature), data, loc, 4);
                            loc += 4;
                            trackSize -= 4;
                            event = makeEvent(EVENT_TYPE_META_TIME_SIGNATURE, 0, deltaTime, timeSignature[0],
                                              exp2(timeSignature[1]));
                            events.push_back(event);
                        } break;

                        case 0x2F: {
                            // End of track
                            event = makeEvent(EVENT_TYPE_META_END_OF_TRACK, 0, deltaTime, -1, -1);
                            events.push_back(event);
                        } break;

                        default: {
                            // Generic
                            std::vector<UInt8> metaData;
                            for (size_t l = length; l; --l) {
                                UInt8 v;
                                copy(&v, sizeof(v), data, loc, sizeof(v));
                                loc += sizeof(v);
                                trackSize -= sizeof(v);
                                metaData.push_back(v);
                            }

                            events.push_back(makeEvent(EVENT_TYPE_META_GENERIC, 0, deltaTime, type, -1, metaData));
                        } break;
                    }
                } else {
                    UInt8 eventParams[2];
                    bool repeat;
                    do {
                        // Handle cases of corrupted files
                        if (loc >= fileSize) return nullptr;

                        UInt8 channel = eventCode & 0xF;
                        repeat = false;
                        switch (eventCode & 0xF0) {
                            case 0x80:  // Note Off
                                copy(eventParams, sizeof(eventParams), data, loc, 2);
                                loc += 2;
                                trackSize -= 2;
                                event =
                                    makeEvent(EVENT_TYPE_NOTE_OFF, channel, deltaTime, eventParams[0], eventParams[1]);
                                events.push_back(event);
                                break;

                            case 0x90:  // Note On
                                copy(eventParams, sizeof(eventParams), data, loc, 2);
                                loc += 2;
                                trackSize -= 2;
                                if (eventParams[1] == 0) {
                                    event = makeEvent(EVENT_TYPE_NOTE_OFF, channel, deltaTime, eventParams[0], 64);
                                } else {
                                    event = makeEvent(EVENT_TYPE_NOTE_ON, channel, deltaTime, eventParams[0],
                                                      eventParams[1]);
                                }
                                events.push_back(event);
                                break;

                            case 0xA0:  // Key Aftertouch
                                copy(eventParams, sizeof(eventParams), data, loc, 2);
                                loc += 2;
                                trackSize -= 2;
                                event = makeEvent(EVENT_TYPE_KEY_AFTERTOUCH, channel, deltaTime, eventParams[0],
                                                  eventParams[1]);
                                events.push_back(event);
                                break;

                            case 0xB0:  // Controller
                                copy(eventParams, sizeof(eventParams), data, loc, 2);
                                loc += 2;
                                trackSize -= 2;
                                // If the controller event is the damper pedal
                                if (eventParams[0] == 0x40) {
                                    // We add the sustain event
                                    event = makeEvent(EVENT_TYPE_SUSTAIN, channel, deltaTime, eventParams[1], -1);
                                    events.push_back(event);
                                } else if (eventParams[0] == 0x07) {
                                    // Main volume
                                    event =
                                        makeEvent(EVENT_TYPE_MIXER_LEVEL_CHANGE, channel, deltaTime, eventParams[1], 0);
                                    events.push_back(event);
                                } else if (eventParams[0] == 0x0A) {
                                    // Pan
                                    event = makeEvent(EVENT_TYPE_MIXER_BALANCE_CHANGE, channel, deltaTime,
                                                      eventParams[1], 0);
                                    events.push_back(event);
                                } else if (eventParams[0] == 0x01) {
                                    // Modulation wheel
                                    event = makeEvent(EVENT_TYPE_MODULATION, channel, deltaTime, eventParams[1], -1);
                                    events.push_back(event);
                                } else {
                                    // Any other control change
                                    event = makeEvent(EVENT_TYPE_CONTROL_CHANGE, channel, deltaTime, eventParams[0],
                                                      eventParams[1]);
                                    events.push_back(event);
                                }
                                break;

                            case 0xC0:  // Program Change
                                copy(eventParams, sizeof(eventParams), data, loc, 1);
                                loc += 1;
                                trackSize -= 1;
                                event = makeEvent(EVENT_TYPE_PROGRAM_CHANGE, channel, deltaTime,
                                                  STUDIO_INSTRUMENT_FROM_PRESET(0, eventParams[0]), 0);
                                events.push_back(event);
                                break;

                            case 0xD0:  // Channel Aftertouch
                                copy(eventParams, sizeof(eventParams), data, loc, 1);
                                loc += 1;
                                trackSize -= 1;
                                events.push_back(
                                    makeEvent(EVENT_TYPE_CHANNEL_AFTERTOUCH, channel, deltaTime, eventParams[0], -1));
                                break;

                            case 0xE0:  // Pitch bend
                                copy(eventParams, sizeof(eventParams), data, loc, 2);
                                loc += 2;
                                trackSize -= 2;
                                event = makeEvent(EVENT_TYPE_PITCH_BEND, channel, deltaTime,
                                                  (SInt32)eventParams[0] | (SInt32)eventParams[1] << 7, 2);
                                events.push_back(event);
                                break;

                            case 0xF0:  // System Exclusive
                            {
                                UInt32 length = 0;
                                std::vector<UInt8> sysexData;
                                UInt8 n;
                                do {
                                    length <<= 7;
                                    copy(&n, sizeof(n), data, loc, sizeof(n));
                                    loc += sizeof(n);
                                    trackSize -= sizeof(n);
                                    length |= (n & 0x7f);
                                } while (n & 0x80);

                                for (size_t l = length; l; --l) {
                                    UInt8 v;
                                    copy(&v, sizeof(v), data, loc, sizeof(v));
                                    loc += sizeof(v);
                                    trackSize -= sizeof(v);
                                    sysexData.push_back(v);
                                }

                                events.push_back(
                                    makeEvent(EVENT_TYPE_SYSTEM_EXCLUSIVE, channel, deltaTime, -1, -1, sysexData));
                                break;
                            }

                            default:
                                // Repeat last command
                                eventCode = lastEventCode;
                                loc--;
                                trackSize++;
                                repeat = YES;
                        }
                    } while (repeat);
                    lastEventCode = eventCode;
                }

            }  // while we have track data

            sequence->data.tracks[track].events = events;
            events.clear();
        }  // for each track

    } catch (InvalidMIDIFileException& e) {
        return nullptr;
    }

    return sequence;
}
