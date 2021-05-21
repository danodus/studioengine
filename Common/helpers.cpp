//
//  helpers.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-12-15.
//  Copyright © 2015-2020 Daniel Cliche. All rights reserved.
//

#include "helpers.h"
#include <script.h>
#include <platform.h>
#include <sequencesdb.h>
#include <regex>
#include <cctype>
#include <fstream>

static MDStudio::Script *commonScript = nullptr;

MDStudio::Color channelColors[STUDIO_MAX_CHANNELS] = {
    MDStudio::redColor,
    MDStudio::limeColor,
    MDStudio::magentaColor,
    MDStudio::blueColor,
    MDStudio::orangeColor,
    MDStudio::cyanColor,
    MDStudio::tanColor,
    MDStudio::lightCoralColor,
    MDStudio::deepPinkColor,
    MDStudio::yellowColor,
    MDStudio::indigoColor,
    MDStudio::tomatoColor,
    MDStudio::violetColor,
    MDStudio::pinkColor,
    MDStudio::goldColor,
    MDStudio::oliveColor
};

const float pianoRollCursorWidth = 2.0f;

// ---------------------------------------------------------------------------------------------------------------------
bool getIsMetaEvent(std::shared_ptr<MelobaseCore::ChannelEvent> event)
{
    return (event->type() == CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE) || (event->type() == CHANNEL_EVENT_TYPE_META_SET_TEMPO) || (event->type() == CHANNEL_EVENT_TYPE_META_END_OF_TRACK) || (event->type() == CHANNEL_EVENT_TYPE_META_GENERIC);
}

// ---------------------------------------------------------------------------------------------------------------------
void loadCommonScript()
{
    commonScript = new MDStudio::Script();
    commonScript->execute(MDStudio::Platform::sharedInstance()->resourcesPath() + "/Common.lua", {}, false);
}

// ---------------------------------------------------------------------------------------------------------------------
void unloadCommonScript()
{
    delete commonScript;
}

// ---------------------------------------------------------------------------------------------------------------------
std::string localizedFolderName(MelobaseCore::SequencesFolder *folder)
{
    if (folder->id == TRASH_FOLDER_ID) {
        return commonScript->findString("trashStr");
    } else if (folder->id == SEQUENCES_FOLDER_ID) {
        return commonScript->findString("sequencesStr");
    }
    return folder->name;
}

// ---------------------------------------------------------------------------------------------------------------------
bool isNumber(const std::string& s, bool isSigned)
{
    std::string re;
    if (isSigned)
        re += "(\\+|-)?";
    re += "[[:digit:]]+";
    std::regex integer(re);
    return std::regex_match(s, integer);
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<UInt8> parseHexString(const std::string &text)
{
    std::string t = text;
    
    // Add spaces if missing
    int i = 0;
    for (auto it = t.begin(); it != t.end(); ++it) {
        if (*it == ' ') {
            i = 0;
        } else {
            if (i == 2) {
                i = 0;
                if (*it != ' ')
                    it = t.insert(it, ' ');
            } else {
                ++i;
            }
        }
    }
    
    std::istringstream ss(t);
    std::vector<UInt8> data;
    
    std::string word;
    while(ss >> word)
    {
        int temp;
        std::stringstream converter;
        converter << std::hex << word;
        converter >> temp;
        data.push_back((UInt8)temp);
    }
    return data;
}

// ---------------------------------------------------------------------------------------------------------------------
bool isFileExist(const std::string &path)
{
    std::ifstream f(path);
    return f.good();
}

