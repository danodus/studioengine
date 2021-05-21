//
//  helpers.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-12-15.
//  Copyright © 2015-2019 Daniel Cliche. All rights reserved.
//

#ifndef HELPERS_H
#define HELPERS_H

#include <color.h>
#include <studio.h>
#include <melobasecore_event.h>
#include <melobasecore_sequence.h>

#include <string>
#include <sstream>
#include <iomanip>

extern MDStudio::Color channelColors[STUDIO_MAX_CHANNELS];
extern const float pianoRollCursorWidth;

// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::string toStringWithPrecision(const T value, const int n = 6)
{
    std::ostringstream out;
    out << std::setprecision(n) << value;
    return out.str();
}

// ---------------------------------------------------------------------------------------------------------------------
template<typename T>
std::string toStringHex(const T i)
{
    std::stringstream stream;
    stream << std::setfill ('0') << std::setw(sizeof(T)*2) << std::hex << (unsigned long)i;
    return stream.str();
}

// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
float normalize(const T value, const T min, const T max)
{
    return static_cast<float>((value - min) / (max - min));
}

// ---------------------------------------------------------------------------------------------------------------------
bool getIsMetaEvent(std::shared_ptr<MelobaseCore::ChannelEvent> event);

// ---------------------------------------------------------------------------------------------------------------------
void loadCommonScript();

// ---------------------------------------------------------------------------------------------------------------------
void unloadCommonScript();

// ---------------------------------------------------------------------------------------------------------------------
std::string localizedFolderName(MelobaseCore::SequencesFolder *folder);

// ---------------------------------------------------------------------------------------------------------------------
bool isFileExist(const std::string &path);

// ---------------------------------------------------------------------------------------------------------------------
bool isNumber(const std::string& s, bool isSigned = false);

// ---------------------------------------------------------------------------------------------------------------------
std::vector<UInt8> parseHexString(const std::string &text);


#endif // HELPERS_H

