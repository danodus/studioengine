//
//  folderpull.cpp
//  MelobaseCore
//
//  Created by Daniel Cliche on 2021-01-25.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#include "folderpull.h"

#define CPPHTTPLIB_USE_POLL
#include <httplib.h>

#include "folderparser.h"
#include "sync.h"

using namespace MelobaseCore;

// ---------------------------------------------------------------------------------------------------------------------
bool FolderPull::pullFolder(UInt64 remoteID, const std::string& serverURL, SequencesDB* database,
                            std::map<UInt64, UInt64> remoteLocalFolderIDs) {
    // Request the sequence

    std::stringstream request;
    request << "/folders/" << remoteID << "?api=" << kSyncAPI;

    httplib::Client cli(serverURL.c_str());
    auto res = cli.Get(request.str().c_str());

    if (!res) return false;

    if (res->status != 200) return false;

    // Create a new folder
    auto newFolder = std::shared_ptr<MelobaseCore::SequencesFolder>(new MelobaseCore::SequencesFolder);

    // Parse the response
    FolderParser folderParser;
    if (!folderParser.parseFolder(newFolder, res->body)) return false;

    newFolder->parentID = remoteLocalFolderIDs[newFolder->parentID];

    // Save it
    database->addFolder(newFolder);

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool FolderPull::updateFolder(SequencesFolder* folder, UInt64 remoteID, const std::string& serverURL,
                              SequencesDB* database, std::map<UInt64, UInt64> remoteLocalFolderIDs) {
    // Request the folder

    std::stringstream request;
    request << "/folders/" << remoteID << "?api=" << kSyncAPI;

    httplib::Client cli(serverURL.c_str());
    auto res = cli.Get(request.str().c_str());

    if (!res) return false;

    if (res->status != 200) return false;

    // Parse the response
    auto parsedFolder = std::make_shared<MelobaseCore::SequencesFolder>();
    auto folderToSave = std::make_shared<MelobaseCore::SequencesFolder>();

    FolderParser folderParser;
    if (!folderParser.parseFolder(parsedFolder, res->body)) return false;

    folderToSave->date = parsedFolder->date;
    folderToSave->name = parsedFolder->name;
    folderToSave->rating = parsedFolder->rating;
    folderToSave->version = parsedFolder->version;
    folderToSave->id = folder->id;
    folderToSave->parentID = remoteLocalFolderIDs[parsedFolder->parentID];

    // We save it
    database->updateFolder(folderToSave, true, false);

    return true;
}
