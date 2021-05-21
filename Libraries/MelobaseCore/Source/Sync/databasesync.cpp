//
//  databasesync.cpp
//  MelobaseCore
//
//  Created by Daniel Cliche on 2021-01-29.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#include "databasesync.h"

#define CPPHTTPLIB_USE_POLL
#include <httplib.h>

#include "../utils.h"
#include "folderssync.h"
#include "sequencessync.h"
#include "sync.h"

using namespace MelobaseCore;

// ---------------------------------------------------------------------------------------------------------------------
bool DatabaseSync::getRemoteLocalFolderIDs(const std::string& serverURL, MelobaseCore::SequencesDB* database) {
    // Request the list of folders
    FoldersSync foldersSync;
    if (!foldersSync.getRemoteFolders(serverURL)) {
        return false;
    }
    auto remoteFolderIDs = foldersSync.folderIDs();
    auto remoteFolderDates = foldersSync.folderDates();

    auto folders = database->getFolders(nullptr);
    int index = 0;
    for (auto remoteFolderDate : remoteFolderDates) {
        for (auto folder : folders) {
            if (folder->id <= LAST_STANDARD_FOLDER_ID) {
                // Standard folders (root, trash, sequences and other reserved folders)
                _remoteLocalFolderIDs[folder->id] = folder->id;
                _localRemoteFolderIDs[folder->id] = folder->id;

            } else if (fabs(remoteFolderDate - folder->date) < 0.001) {
                auto remoteID = remoteFolderIDs.at(index);
                _remoteLocalFolderIDs[remoteID] = folder->id;
                _localRemoteFolderIDs[folder->id] = remoteID;
                break;
            }
        }
        index++;
    }
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool DatabaseSync::checkServerCompatibility(const std::string& serverURL, int *error) {
    if (_didSetProgressFn) {
        _didSetProgressFn(this, 0.0f, "Checking the server compatibility", {});
    }

    std::stringstream request;
    request << "/";

    httplib::Client cli(serverURL.c_str());
    auto res = cli.Get(request.str().c_str());

    if (!res || res->status != 200) {
        *error = DATABASE_SYNC_COMMUNICATION_ERROR;
        return false;
    }

    auto content = res->body;

    auto compatibilityStringPos = content.find("Compatibility:");

    if (compatibilityStringPos == std::string::npos) return false;

    auto compatibilityString = content.substr(compatibilityStringPos + 14);

    int minAPI = -1;
    int maxAPI = -1;

    auto majorComponents = stringComponents(compatibilityString, ',');
    for (auto c : majorComponents) {
        auto minorComponents = stringComponents(c, '=');
        if (minorComponents.size() != 2) continue;

        auto left = trim(minorComponents[0]);
        auto right = trim(minorComponents[1]);

        if (left == "minAPI")
            minAPI = std::stoi(right);
        else if (left == "maxAPI")
            maxAPI = std::stoi(right);
    }

    // Check the maximum supported API by the server
    if (maxAPI < kSyncAPI) {
        *error = DATABASE_SYNC_INCOMPATIBLE_SERVER_ERROR;
        return false;
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool DatabaseSync::requestSave(const std::string& serverURL) {
    
    std::stringstream request;
    request << "/save";
    
    httplib::Client cli(serverURL.c_str());
    auto res = cli.Get(request.str().c_str());
    
    if (!res || res->status != 200) return false;
    
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool DatabaseSync::sync(const std::string& serverURL, SequencesDB* database, int* error) {
    *error = DATABASE_SYNC_SUCCESS;

    bool ret;

    //
    // Check if the server is compatible with the synchronization
    //

    if (!checkServerCompatibility(serverURL, error))
        return false;
    
    //
    // Request save on remote
    //
    
    if (!requestSave(serverURL)) {
        *error = DATABASE_SYNC_COMMUNICATION_ERROR;
        return false;
    }
    
    //
    // Create a mapping for remote and local folders
    //

    ret = getRemoteLocalFolderIDs(serverURL, database);
    if (!ret) {
        *error = DATABASE_SYNC_COMMUNICATION_ERROR;
        return false;
    }

    //
    // Synchronize the folders and sequences
    //

    FoldersSync foldersSync;
    foldersSync.setDidSetProgressFn(
        [this](FoldersSync* sender, float progress, const std::string& description, const std::vector<int>& vars) {
            if (_didSetProgressFn) _didSetProgressFn(this, progress, description, vars);
        });

    foldersSync.setUpdateLocalRemoteFoldersMappingFn(
        [this](FoldersSync* sender, std::string serverURL, MelobaseCore::SequencesDB* database) -> bool {
            bool ret = getRemoteLocalFolderIDs(serverURL, database);
            if (!ret) return false;

            sender->setRemoteLocalFolderIDs(_remoteLocalFolderIDs);
            sender->setLocalRemoteFolderIDs(_localRemoteFolderIDs);

            return ret;
        });

    ret = foldersSync.sync(serverURL, database);

    if (!ret) {
        *error = DATABASE_SYNC_COMMUNICATION_ERROR;
        return false;
    }

    SequencesSync sequencesSync;
    sequencesSync.setDidSetProgressFn(
        [this](SequencesSync* sender, float progress, const std::string& description, const std::vector<int>& vars) {
            if (_didSetProgressFn) _didSetProgressFn(this, progress, description, vars);
        });

    ret = sequencesSync.sync(serverURL, database, _remoteLocalFolderIDs, _localRemoteFolderIDs);

    return ret;
}
