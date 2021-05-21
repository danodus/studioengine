//
//  folderparser.h
//  MelobaseCore
//
//  Created by Daniel Cliche on 2021-01-20.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#ifndef FOLDERPARSER_H
#define FOLDERPARSER_H

#include <expat.h>

#include <map>
#include <memory>

#include "../melobasecore_sequence.h"

namespace MelobaseCore {

class FolderParser {
    enum class ParserStates { Root, Folder, FolderDate, FolderName, FolderRating, FolderVersion, FolderParentID };
    ParserStates _parserState;

    std::shared_ptr<SequencesFolder> _newFolder;

    double _newFolderVersion = 0.0;
    bool _isNameSet = false;

    static void XMLCALL start(void* data, const char* el, const char** attr);
    static void XMLCALL chars(void* data, const char* el, int len);
    static void XMLCALL end(void* data, const char* el);

   public:
    bool parseFolder(std::shared_ptr<SequencesFolder> folder, const std::string& data);
    static void parseFolder(std::shared_ptr<SequencesFolder> folder,
                            const std::map<std::string, std::string>& queryMap);
};

}  // namespace MelobaseCore

#endif  // FOLDERPARSER_H
