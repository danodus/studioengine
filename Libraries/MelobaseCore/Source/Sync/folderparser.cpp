//
//  folderparser.cpp
//  MelobaseCore
//
//  Created by Daniel Cliche on 2021-01-20.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#include "folderparser.h"

using namespace MelobaseCore;

// ---------------------------------------------------------------------------------------------------------------------
void XMLCALL FolderParser::start(void* data, const char* el, const char** attr) {
    auto fp = reinterpret_cast<FolderParser*>(data);

    auto elementName = std::string(el);

    if (fp->_parserState == ParserStates::Root && elementName == "folder") {
        fp->_parserState = ParserStates::Folder;
    } else if (fp->_parserState == ParserStates::Folder && elementName == "date") {
        fp->_parserState = ParserStates::FolderDate;
    } else if (fp->_parserState == ParserStates::Folder && elementName == "name") {
        fp->_parserState = ParserStates::FolderName;
    } else if (fp->_parserState == ParserStates::Folder && elementName == "rating") {
        fp->_parserState = ParserStates::FolderRating;
    } else if (fp->_parserState == ParserStates::Folder && elementName == "version") {
        fp->_parserState = ParserStates::FolderVersion;
    } else if (fp->_parserState == ParserStates::Folder && elementName == "parentid") {
        fp->_parserState = ParserStates::FolderParentID;
    } else {
        assert(0);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void XMLCALL FolderParser::chars(void* data, const char* el, int len) {
    FolderParser* fp = reinterpret_cast<FolderParser*>(data);

    auto string = std::string(el, len);

    switch (fp->_parserState) {
        case ParserStates::FolderDate:
            fp->_newFolder->date = std::stod(string);
            break;
        case ParserStates::FolderName:
            if (!fp->_isNameSet) fp->_newFolder->name = "";
            fp->_newFolder->name += string;
            fp->_isNameSet = true;
            break;
        case ParserStates::FolderRating:
            fp->_newFolder->rating = std::stof(string);
            break;
        case ParserStates::FolderVersion:
            fp->_newFolderVersion = std::stod(string);
            break;
        case ParserStates::FolderParentID:
            fp->_newFolder->parentID = std::stoull(string);
            break;
        default:
            assert(0);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void XMLCALL FolderParser::end(void* data, const char* el) {
    FolderParser* fp = reinterpret_cast<FolderParser*>(data);

    auto elementName = std::string(el);

    if (elementName == "folder") {
        fp->_parserState = ParserStates::Root;
    } else if (fp->_parserState == ParserStates::FolderDate && elementName == "date") {
        fp->_parserState = ParserStates::Folder;
    } else if (fp->_parserState == ParserStates::FolderName && elementName == "name") {
        fp->_isNameSet = false;
        fp->_parserState = ParserStates::Folder;
    } else if (fp->_parserState == ParserStates::FolderRating && elementName == "rating") {
        fp->_parserState = ParserStates::Folder;
    } else if (fp->_parserState == ParserStates::FolderVersion && elementName == "version") {
        fp->_parserState = ParserStates::Folder;
    } else if (fp->_parserState == ParserStates::FolderParentID && elementName == "parentid") {
        fp->_parserState = ParserStates::Folder;
    } else {
        assert(0);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool FolderParser::parseFolder(std::shared_ptr<SequencesFolder> folder, const std::string& data) {
    _newFolder = folder;

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
    if (_newFolderVersion > 0.0) folder->version = _newFolderVersion;
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
void FolderParser::parseFolder(std::shared_ptr<SequencesFolder> folder,
                               const std::map<std::string, std::string>& queryMap) {
    if (queryMap.count("date") > 0) folder->date = std::stod(queryMap.at("date"));
    if (queryMap.count("name") > 0) folder->name = queryMap.at("name");
    if (queryMap.count("rating") > 0) folder->rating = std::stof(queryMap.at("rating"));
    if (queryMap.count("parentid") > 0) folder->parentID = std::stoull(queryMap.at("parentid"));
    if (queryMap.count("version") > 0) folder->version = std::stod(queryMap.at("version"));
}
