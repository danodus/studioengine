//
//  sequenceparser.cpp
//  MelobaseCore
//
//  Created by Daniel Cliche on 2021-01-20.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#include "sequenceparser.h"

#include <sequencesdb.h>

#include "../utils.h"

using namespace MelobaseCore;

// ---------------------------------------------------------------------------------------------------------------------
void XMLCALL SequenceParser::start(void* data, const char* el, const char** attr) {
    auto sp = reinterpret_cast<SequenceParser*>(data);

    auto elementName = std::string(el);

    if (sp->_parserState == ParserStates::Root && elementName == "sequence") {
        sp->_parserState = ParserStates::Sequence;
    } else if (sp->_parserState == ParserStates::Sequence && elementName == "date") {
        sp->_parserState = ParserStates::SequenceDate;
    } else if (sp->_parserState == ParserStates::Sequence && elementName == "folderid") {
        sp->_parserState = ParserStates::SequenceFolderID;
    } else if (sp->_parserState == ParserStates::Sequence && elementName == "name") {
        sp->_parserState = ParserStates::SequenceName;
    } else if (sp->_parserState == ParserStates::Sequence && elementName == "rating") {
        sp->_parserState = ParserStates::SequenceRating;
    } else if (sp->_parserState == ParserStates::Sequence && elementName == "version") {
        sp->_parserState = ParserStates::SequenceVersion;
    } else if (sp->_parserState == ParserStates::Sequence && elementName == "dataVersion") {
        sp->_parserState = ParserStates::SequenceDataVersion;
    } else if (sp->_parserState == ParserStates::Sequence && elementName == "playcount") {
        sp->_parserState = ParserStates::SequencePlayCount;
    } else if (sp->_parserState == ParserStates::Sequence && elementName == "tickperiod") {
        sp->_parserState = ParserStates::SequenceTickPeriod;
    } else if (sp->_parserState == ParserStates::Sequence && elementName == "annotations") {
        sp->_parserState = ParserStates::SequenceAnnotations;
    } else if (sp->_parserState == ParserStates::Sequence && elementName == "data") {
        sp->_parserState = ParserStates::SequenceData;
    } else {
        assert(0);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void XMLCALL SequenceParser::chars(void* data, const char* el, int len) {
    SequenceParser* sp = reinterpret_cast<SequenceParser*>(data);

    auto string = std::string(el, len);

    switch (sp->_parserState) {
        case ParserStates::SequenceDate:
            sp->_newSequence->date = std::stod(string);
            break;
        case ParserStates::SequenceFolderID:
            sp->_isNewSequenceFolderIDAvailable = true;
            sp->_newSequenceFolderID = std::stoull(string);
            break;
        case ParserStates::SequenceName:
            if (!sp->_nameSet) sp->_newSequence->name = "";
            sp->_newSequence->name += string;
            sp->_nameSet = true;
            break;
        case ParserStates::SequenceRating:
            sp->_newSequence->rating = std::stof(string);
            break;
        case ParserStates::SequenceVersion:
            sp->_newSequenceVersion = std::stod(string);
            break;
        case ParserStates::SequenceDataVersion:
            sp->_newSequenceDataVersion = std::stod(string);
            break;
        case ParserStates::SequencePlayCount:
            sp->_newSequence->playCount = std::stoi(string);
            break;
        case ParserStates::SequenceTickPeriod:
            sp->_newSequence->data.tickPeriod = std::stod(string);
            break;
        case ParserStates::SequenceAnnotations:
            if (!sp->_sequenceAnnotationsStrSet) {
                sp->_newSequenceAnnotationsStr = string;
                sp->_sequenceAnnotationsStrSet = true;
            } else {
                sp->_newSequenceAnnotationsStr += string;
            }
            break;
        case ParserStates::SequenceData:
            if (!sp->_sequenceDataStrSet) {
                sp->_newSequenceDataStr = string;
                sp->_sequenceDataStrSet = true;
            } else {
                sp->_newSequenceDataStr += string;
            }
            break;
        default:
            assert(0);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void XMLCALL SequenceParser::end(void* data, const char* el) {
    SequenceParser* sp = reinterpret_cast<SequenceParser*>(data);

    auto elementName = std::string(el);

    if (elementName == "sequence") {
        sp->_parserState = ParserStates::Root;
    } else if (sp->_parserState == ParserStates::SequenceDate && elementName == "date") {
        sp->_parserState = ParserStates::Sequence;
    } else if (sp->_parserState == ParserStates::SequenceFolderID && elementName == "folderid") {
        sp->_parserState = ParserStates::Sequence;
    } else if (sp->_parserState == ParserStates::SequenceName && elementName == "name") {
        sp->_nameSet = false;
        sp->_parserState = ParserStates::Sequence;
    } else if (sp->_parserState == ParserStates::SequenceRating && elementName == "rating") {
        sp->_parserState = ParserStates::Sequence;
    } else if (sp->_parserState == ParserStates::SequenceVersion && elementName == "version") {
        sp->_parserState = ParserStates::Sequence;
    } else if (sp->_parserState == ParserStates::SequenceDataVersion && elementName == "dataVersion") {
        sp->_parserState = ParserStates::Sequence;
    } else if (sp->_parserState == ParserStates::SequencePlayCount && elementName == "playcount") {
        sp->_parserState = ParserStates::Sequence;
    } else if (sp->_parserState == ParserStates::SequenceTickPeriod && elementName == "tickperiod") {
        sp->_parserState = ParserStates::Sequence;
    } else if (sp->_parserState == ParserStates::SequenceAnnotations && elementName == "annotations") {
        auto v = base64Decode(sp->_newSequenceAnnotationsStr.c_str());
        setSequenceAnnotationsFromBlob(sp->_newSequence.get(), &v[0], v.size());
        sp->_parserState = ParserStates::Sequence;
    } else if (sp->_parserState == ParserStates::SequenceData && elementName == "data") {
        auto v = base64Decode(sp->_newSequenceDataStr.c_str());
        setSequenceDataFromBlob(sp->_newSequence, &v[0], v.size());
        sp->_parserState = ParserStates::Sequence;
    } else {
        assert(0);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool SequenceParser::parseSequence(std::shared_ptr<Sequence> sequence, std::string data, bool* isRemoteFolderIDAvailable, UInt64* folderID) {
    *isRemoteFolderIDAvailable = false;
    
    _isNewSequenceFolderIDAvailable = false;
    _newSequenceFolderID = 0;

    _newSequence = sequence;
    XML_Parser parser = XML_ParserCreate(NULL);

    XML_SetUserData(parser, this);
    _parserState = ParserStates::Root;

    XML_SetElementHandler(parser, start, end);
    XML_SetCharacterDataHandler(parser, chars);

    for (;;) {
        int done;
        int len;

        len = (int)data.length();
        done = 1;

        if (XML_Parse(parser, data.data(), len, done) == XML_STATUS_ERROR) {
            fprintf(stderr, "Parse error at line %lu:\n%s\n", XML_GetCurrentLineNumber(parser),
                    XML_ErrorString(XML_GetErrorCode(parser)));
            XML_ParserFree(parser);
            return false;
        }

        if (done) break;
    }
    XML_ParserFree(parser);

    // Force the parsed version if any
    if (_newSequenceVersion > 0.0) sequence->version = _newSequenceVersion;

    if (_newSequenceDataVersion > 0.0) sequence->dataVersion = _newSequenceDataVersion;

    *isRemoteFolderIDAvailable = _isNewSequenceFolderIDAvailable;
    *folderID = _newSequenceFolderID;

    return true;
}
