//
//  folderssync.cpp
//  MelobaseCore
//
//  Created by Daniel Cliche on 2021-01-28.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#include "folderssync.h"

#define CPPHTTPLIB_USE_POLL
#include <httplib.h>

#include <sstream>

#include "folderpull.h"
#include "folderpush.h"
#include "sync.h"

using namespace MelobaseCore;

// ---------------------------------------------------------------------------------------------------------------------
void XMLCALL FoldersSync::start(void* data, const char* el, const char** attr) {
    auto fs = reinterpret_cast<FoldersSync*>(data);

    auto elementName = std::string(el);

    std::map<std::string, std::string> attributeDict{};
    for (size_t i = 0; attr[i]; i += 2) attributeDict[attr[i]] = attr[i + 1];

    if (fs->_parserState == ParserStates::Root && elementName == "folders") {
        fs->_parserState = ParserStates::Folders;

        if (attributeDict.count("maxAPI") > 0) {
            fs->_maxAPI = std::stoi(attributeDict.at("maxAPI"));
            if (fs->_maxAPI > kSyncAPI) fs->_maxAPI = kSyncAPI;
        }
    } else if (fs->_parserState == ParserStates::Folders && elementName == "folder") {
        fs->_parserState = ParserStates::FoldersFolder;
    } else if (fs->_parserState == ParserStates::FoldersFolder && elementName == "id") {
        fs->_parserState = ParserStates::FoldersFolderID;
    } else if (fs->_parserState == ParserStates::FoldersFolder && elementName == "date") {
        fs->_parserState = ParserStates::FoldersFolderDate;
    } else if (fs->_parserState == ParserStates::FoldersFolder && elementName == "name") {
        fs->_parserState = ParserStates::FoldersFolderName;
    } else if (fs->_parserState == ParserStates::FoldersFolder && elementName == "rating") {
        fs->_parserState = ParserStates::FoldersFolderRating;
    } else if (fs->_parserState == ParserStates::FoldersFolder && elementName == "version") {
        fs->_parserState = ParserStates::FoldersFolderVersion;
    } else if (fs->_parserState == ParserStates::FoldersFolder && elementName == "parentid") {
        fs->_parserState = ParserStates::FoldersFolderParentID;
    } else {
        assert(0);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void XMLCALL FoldersSync::chars(void* data, const char* el, int len) {
    auto fs = reinterpret_cast<FoldersSync*>(data);

    auto string = std::string(el, len);

    switch (fs->_parserState) {
        case ParserStates::FoldersFolderID:
            fs->_folderID = std::stoull(string);
            break;
        case ParserStates::FoldersFolderDate:
            fs->_folderDate = std::stod(string);
            break;
        case ParserStates::FoldersFolderName:
            fs->_folderName = string;
            break;
        case ParserStates::FoldersFolderRating:
            fs->_folderRating = std::stof(string);
            break;
        case ParserStates::FoldersFolderVersion:
            fs->_folderVersion = std::stod(string);
            break;
        case ParserStates::FoldersFolderParentID:
            fs->_parentID = std::stoull(string);
            break;
        default:
            assert(0);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void XMLCALL FoldersSync::end(void* data, const char* el) {
    auto fs = reinterpret_cast<FoldersSync*>(data);

    auto elementName = std::string(el);

    if (fs->_parserState == ParserStates::Folders && elementName == "folders") {
        fs->_parserState = ParserStates::Root;
    } else if (fs->_parserState == ParserStates::FoldersFolder && elementName == "folder") {
        // We add this folder declaration

        fs->_folderDates.emplace_back(fs->_folderDate);
        fs->_folderIDs.emplace_back(fs->_folderID);
        fs->_folderParentIDs.emplace_back(fs->_parentID);
        fs->_folderVersions.emplace_back(fs->_folderVersion);

        fs->_parserState = ParserStates::Folders;
    } else if (fs->_parserState == ParserStates::FoldersFolderID && elementName == "id") {
        fs->_parserState = ParserStates::FoldersFolder;
    } else if (fs->_parserState == ParserStates::FoldersFolderDate && elementName == "date") {
        fs->_parserState = ParserStates::FoldersFolder;
    } else if (fs->_parserState == ParserStates::FoldersFolderName && elementName == "name") {
        fs->_parserState = ParserStates::FoldersFolder;
    } else if (fs->_parserState == ParserStates::FoldersFolderRating && elementName == "rating") {
        fs->_parserState = ParserStates::FoldersFolder;
    } else if (fs->_parserState == ParserStates::FoldersFolderVersion && elementName == "version") {
        fs->_parserState = ParserStates::FoldersFolder;
    } else if (fs->_parserState == ParserStates::FoldersFolderParentID && elementName == "parentid") {
        fs->_parserState = ParserStates::FoldersFolder;
    } else {
        assert(0);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool FoldersSync::getRemoteFolders(std::string serverURL) {
    _errorDetected = false;

    _serverURL = serverURL;

    //
    // Request the list of folders
    //

    if (_didSetProgressFn) _didSetProgressFn(this, 0.0f, "Requesting the list of folders", {});

    std::stringstream request;
    request << "/folders?api=" << kSyncAPI;

    httplib::Client cli(serverURL.c_str());
    auto res = cli.Get(request.str().c_str());

    if (!res) return false;

    if (res->status != 200) return false;

    //
    // Parse
    //

    XML_Parser parser = XML_ParserCreate(NULL);

    XML_SetUserData(parser, this);
    _parserState = ParserStates::Root;

    XML_SetElementHandler(parser, start, end);
    XML_SetCharacterDataHandler(parser, chars);

    for (;;) {
        int done;
        int len;

        len = (int)res->body.length();
        done = 1;

        if (XML_Parse(parser, res->body.data(), len, done) == XML_STATUS_ERROR) {
            fprintf(stderr, "Parse error at line %lu:\n%s\n", XML_GetCurrentLineNumber(parser),
                    XML_ErrorString(XML_GetErrorCode(parser)));
            XML_ParserFree(parser);
            return false;
        }

        if (done) break;
    }
    XML_ParserFree(parser);

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
// Get a list of folder IDs to synchronize
void FoldersSync::getFoldersToSynchronize() {
    // First, we get all the local folder dates

    auto localFolders = _database->getFolders(nullptr);

    // For each folder declaration
    size_t index = 0;
    for (auto date : _folderDates) {
        // We check if the folder exist in the database
        std::shared_ptr<SequencesFolder> foundFolder;
        for (auto folder : localFolders) {
            if (std::fabs(folder->date - date) < 0.001) {
                foundFolder = folder;
                break;
            }
        }

        // If no folder has been found
        if (!foundFolder) {
            // We do not have this folder, so we add it to the pull list
            _folderIDsToPull.emplace_back(_folderIDs.at(index));
        } else {
            // We remove this folder from our local list
            localFolders.erase(std::remove(localFolders.begin(), localFolders.end(), foundFolder), localFolders.end());

            // We do have this folder, but it may need to be updated depending on the version
            auto folderToUpdate = foundFolder;
            // Note: The version is an integer at the server level
            auto localVersion = std::floor(folderToUpdate->version);
            auto remoteVersion = std::floor(_folderVersions.at(index));

            // If the local version has en earlier time then the remote version
            if (localVersion < remoteVersion) {
                // Update this folder

                _folderIDsToPullForUpdate.emplace_back(_folderIDs.at(index));
                _foldersToPullForUpdate.emplace_back(folderToUpdate);
            }

            if (localVersion > remoteVersion) {
                _folderIDsToPushForUpdate.emplace_back(_folderIDs.at(index));
                _foldersToPushForUpdate.emplace_back(folderToUpdate);
            }
        }

        index++;
    }  // for each folder declaration

    //
    // The remaining local folders in our list are not present on the remote server.
    //

    for (auto folder : localFolders) _foldersToPush.emplace_back(folder);
}

// ---------------------------------------------------------------------------------------------------------------------
void FoldersSync::addFolderToPushPull(std::shared_ptr<SequencesFolder> folderToPushPull,
                                      std::vector<std::shared_ptr<SequencesFolder>>& folderList,
                                      const std::vector<std::shared_ptr<SequencesFolder>>& allFolders) {
    //
    // If the parent of the folder to push is not yet present in the folder list, we do so recursively
    //

    bool isFound = false;
    for (auto f : folderList) {
        if (f->id == folderToPushPull->parentID) {
            isFound = true;
            break;
        }
    }

    if (!isFound) {
        for (auto f : allFolders) {
            if (f->id == folderToPushPull->parentID) {
                addFolderToPushPull(f, folderList, allFolders);
                break;
            }
        }
    }

    //
    // If the folder is not yet added, we do so
    //

    isFound = false;
    for (auto f : folderList) {
        if (f->id == folderToPushPull->id) {
            isFound = true;
            break;
        }
    }

    if (!isFound) folderList.emplace_back(folderToPushPull);
}
// ---------------------------------------------------------------------------------------------------------------------
bool FoldersSync::pullFolders() {
    // Create a temporary list of folders to pull
    std::vector<std::shared_ptr<SequencesFolder>> foldersToPull;

    for (auto pullFolderID : _folderIDsToPull) {
        // Find the index of this folder ID
        size_t i = 0;
        for (auto fid : _folderIDs) {
            if (fid == pullFolderID) break;
            ++i;
        }

        auto f2 = std::make_shared<MelobaseCore::SequencesFolder>();
        f2->id = _folderIDs.at(i);
        f2->parentID = _folderParentIDs.at(i);
        foldersToPull.emplace_back(f2);
    }

    // Create a list of folders to pull in the order of dependencies
    std::vector<std::shared_ptr<SequencesFolder>> foldersToPullOrdered;

    for (auto folder : foldersToPull) addFolderToPushPull(folder, foldersToPullOrdered, foldersToPull);

    FolderPull pull;
    size_t folderIndex = 0;
    size_t totalNbFolders = _folderIDsToPull.size();
    for (auto pullFolder : foldersToPullOrdered) {
        if (_didSetProgressFn) {
            _didSetProgressFn(this, (float)_countNbFoldersToSync / (float)_totalNbFoldersToSync,
                              "Downloading folder %d of %d",
                              {static_cast<int>(folderIndex + 1), static_cast<int>(totalNbFolders)});
        }

        if (!_updateLocalRemoteFoldersMappingFn(this, _serverURL, _database)) {
            _errorDetected = true;
            break;
        }

        if (!(pull.pullFolder(pullFolder->id, _serverURL, _database, _remoteLocalFolderIDs))) {
            _errorDetected = true;
            break;
        }
        folderIndex++;
        _countNbFoldersToSync++;
    }
    return !_errorDetected;
}

// ---------------------------------------------------------------------------------------------------------------------
bool FoldersSync::pullFoldersForUpdate() {
    FolderPull pull;
    size_t folderIndex = 0;
    size_t totalNbFolders = _folderIDsToPullForUpdate.size();

    for (auto folder : _foldersToPullForUpdate) {
        if (_didSetProgressFn) {
            _didSetProgressFn(this, (float)_countNbFoldersToSync / (float)_totalNbFoldersToSync,
                              "Updating local folder %d of %d",
                              {static_cast<int>(folderIndex + 1), static_cast<int>(totalNbFolders)});
        }

        if (!_updateLocalRemoteFoldersMappingFn(this, _serverURL, _database)) {
            _errorDetected = true;
            break;
        }

        if (!pull.updateFolder(folder.get(), _folderIDsToPullForUpdate.at(folderIndex), _serverURL, _database,
                               _remoteLocalFolderIDs)) {
            _errorDetected = true;
            break;
        }
        folderIndex++;
        _countNbFoldersToSync++;
    }
    return !_errorDetected;
}

// ---------------------------------------------------------------------------------------------------------------------
bool FoldersSync::pushFolders() {
    FolderPush push;
    size_t folderIndex = 0;
    size_t totalNbFolders = _foldersToPush.size();

    // Create a list of folders to push in the order of dependencies
    std::vector<std::shared_ptr<SequencesFolder>> foldersToPushOrdered;

    for (auto folder : _foldersToPush) addFolderToPushPull(folder, foldersToPushOrdered, _foldersToPush);

    for (auto folder : foldersToPushOrdered) {
        if (_didSetProgressFn) {
            _didSetProgressFn(this, (float)_countNbFoldersToSync / (float)_totalNbFoldersToSync,
                              "Uploading folder %d of %d",
                              {static_cast<int>(folderIndex + 1), static_cast<int>(totalNbFolders)});
        }

        if (!_updateLocalRemoteFoldersMappingFn(this, _serverURL, _database)) {
            _errorDetected = true;
            break;
        }

        if (!(push.pushFolder(folder.get(), _serverURL, _database, _maxAPI, _localRemoteFolderIDs))) {
            _errorDetected = true;
            break;
        }
        folderIndex++;
        _countNbFoldersToSync++;
    }
    return !_errorDetected;
}

// ---------------------------------------------------------------------------------------------------------------------
bool FoldersSync::pushFoldersForUpdate() {
    FolderPush push;
    size_t folderIndex = 0;
    size_t totalNbFolders = _foldersToPushForUpdate.size();
    for (auto folder : _foldersToPushForUpdate) {
        if (_didSetProgressFn) {
            _didSetProgressFn(this, (float)_countNbFoldersToSync / (float)_totalNbFoldersToSync,
                              "Updating remote folder %d of %d",
                              {static_cast<int>(folderIndex + 1), static_cast<int>(totalNbFolders)});
        }

        if (!_updateLocalRemoteFoldersMappingFn(this, _serverURL, _database)) {
            _errorDetected = true;
            break;
        }

        if (!(push.updateFolder(folder.get(), _folderIDsToPushForUpdate.at(folderIndex), _serverURL, _database, _maxAPI,
                                _localRemoteFolderIDs))) {
            _errorDetected = true;
            break;
        }
        folderIndex++;
        _countNbFoldersToSync++;
    }
    return !_errorDetected;
}

// ---------------------------------------------------------------------------------------------------------------------
bool FoldersSync::sync(std::string serverURL, MelobaseCore::SequencesDB* database) {
    // By default, the maximum API is the one used by
    _maxAPI = kSyncAPI;
    _database = database;

    //
    // Get the remote folders
    //

    if (!getRemoteFolders(serverURL)) return false;

    //
    // We get the list of folders to synchronize
    //

    if (_didSetProgressFn) _didSetProgressFn(this, 0.0f, "Getting the list of folders to synchronize", {});

    getFoldersToSynchronize();

    //
    // Set the progress counters
    //

    _countNbFoldersToSync = 0;
    _totalNbFoldersToSync = _folderIDsToPull.size() + _foldersToPush.size() + _foldersToPullForUpdate.size() +
                            _foldersToPushForUpdate.size();

    // Disable the undo manager registration
    if (_database->undoManager()) _database->undoManager()->disableRegistration();

    //
    // Pull the folders
    //

    if (!_errorDetected) pullFolders();

    // Update folders mapping
    if (!_errorDetected) {
        if (!_updateLocalRemoteFoldersMappingFn(this, serverURL, database)) {
            _errorDetected = true;
        }
    }

    if (!_errorDetected) pullFoldersForUpdate();

    // Update folders mapping
    if (!_errorDetected) {
        if (!_updateLocalRemoteFoldersMappingFn(this, serverURL, database)) {
            _errorDetected = true;
        }
    }

    //
    // We push the folders
    //

    if (!_errorDetected) pushFolders();

    // Update folders mapping
    if (!_errorDetected) {
        if (!_updateLocalRemoteFoldersMappingFn(this, serverURL, database)) {
            _errorDetected = true;
        }
    }

    if (!_errorDetected) pushFoldersForUpdate();

    // Enable undo manager registration
    if (_database->undoManager()) _database->undoManager()->enableRegistration();

    if (_errorDetected) return false;

    return true;
}
