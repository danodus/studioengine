//
//  folderpush.cpp
//  MelobaseCore
//
//  Created by Daniel Cliche on 2021-01-25.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#include "folderpush.h"

#define CPPHTTPLIB_USE_POLL
#include <httplib.h>

#include <sstream>

#include "../utils.h"

using namespace MelobaseCore;

// ---------------------------------------------------------------------------------------------------------------------
std::string FolderPush::stringFromFolder(SequencesFolder* folder) {
    std::stringstream ss;

    ss << std::fixed << "date=" << folder->date << "&"
       << "name=" << httplib::detail::encode_query_param(folder->name) << "&"
       << "rating=" << folder->rating << "&"
       << "version=" << folder->version << "&"
       << "parentid=" << folder->parentID;

    return ss.str();
}

// ---------------------------------------------------------------------------------------------------------------------
bool FolderPush::pushFolder(SequencesFolder* folder, const std::string& serverURL, SequencesDB* database, int maxAPI,
                            std::map<UInt64, UInt64> localRemoteFolderIDs) {
    // Post the folder

    unsigned long localParentFolderID = folder->parentID;
    folder->parentID = localRemoteFolderIDs.at(localParentFolderID);

    std::stringstream request;
    request << "/folders?api=" << maxAPI << "&" << stringFromFolder(folder);

    folder->parentID = localParentFolderID;

    httplib::Client cli(serverURL.c_str());
    auto res = cli.Post(request.str().c_str());

    if (!res) return false;

    if (res->status != 200) return false;

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool FolderPush::updateFolder(SequencesFolder* folder, UInt64 remoteID, const std::string& serverURL,
                              SequencesDB* database, int maxAPI,
                              std::map<UInt64, UInt64> localRemoteFolderIDs) {
    // Post the folder

    UInt64 localParentFolderID = folder->parentID;
    folder->parentID = localRemoteFolderIDs.at(localParentFolderID);

    std::stringstream request;
    request << "/folders/" << remoteID << "?api=" << maxAPI << "&" << stringFromFolder(folder);

    folder->parentID = localParentFolderID;

    httplib::Client cli(serverURL.c_str());
    auto res = cli.Post(request.str().c_str());

    if (!res) return false;

    if (res->status != 200) return false;

    return true;
}
