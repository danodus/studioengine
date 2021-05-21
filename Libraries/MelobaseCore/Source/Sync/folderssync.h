//
//  folderssync.h
//  MelobaseCore
//
//  Created by Daniel Cliche on 2021-01-28.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#ifndef FOLDERSSYNC_H
#define FOLDERSSYNC_H

#include <expat.h>

#include <map>

#include "../sequencesdb.h"

namespace MelobaseCore {

class FoldersSync {
   public:
    typedef std::function<void(FoldersSync* sender, float progress, std::string description, std::vector<int>)> DidSetProgressFnType;
    typedef std::function<bool(FoldersSync* sender, std::string serverURL, MelobaseCore::SequencesDB* database)>
        UpdateLocalRemoteFoldersMappingFnType;

   private:
    std::string _serverURL;

    enum class ParserStates {
        Root,
        Folders,
        FoldersFolder,
        FoldersFolderID,
        FoldersFolderDate,
        FoldersFolderName,
        FoldersFolderRating,
        FoldersFolderVersion,
        FoldersFolderParentID,
    };
    ParserStates _parserState;

    UInt64 _folderID;
    Float64 _folderDate;
    std::string _folderName;
    Float32 _folderRating;
    Float64 _folderVersion;
    UInt64 _parentID;

    bool _errorDetected = false;

    std::vector<Float64> _folderDates;
    std::vector<UInt64> _folderIDs;
    std::vector<UInt64> _folderParentIDs;
    std::vector<Float64> _folderVersions;

    MelobaseCore::SequencesDB* _database;

    std::vector<UInt64> _folderIDsToPull;
    std::vector<std::shared_ptr<SequencesFolder>> _foldersToPush;
    std::vector<UInt64> _folderIDsToPullForUpdate;
    std::vector<std::shared_ptr<SequencesFolder>> _foldersToPullForUpdate;
    std::vector<UInt64> _folderIDsToPushForUpdate;
    std::vector<std::shared_ptr<SequencesFolder>> _foldersToPushForUpdate;

    size_t _countNbFoldersToSync;
    size_t _totalNbFoldersToSync;

    int _maxAPI;

    std::map<UInt64, UInt64> _remoteLocalFolderIDs;
    std::map<UInt64, UInt64> _localRemoteFolderIDs;

    DidSetProgressFnType _didSetProgressFn = nullptr;
    UpdateLocalRemoteFoldersMappingFnType _updateLocalRemoteFoldersMappingFn = nullptr;

    static void XMLCALL start(void* data, const char* el, const char** attr);
    static void XMLCALL chars(void* data, const char* el, int len);
    static void XMLCALL end(void* data, const char* el);

    void getFoldersToSynchronize();
    void addFolderToPushPull(std::shared_ptr<SequencesFolder> folderToPushPull,
                             std::vector<std::shared_ptr<SequencesFolder>>& folderList,
                             const std::vector<std::shared_ptr<SequencesFolder>>& allFolders);
    bool pullFolders();
    bool pullFoldersForUpdate();
    bool pushFolders();
    bool pushFoldersForUpdate();

   public:
    bool getRemoteFolders(std::string serverURL);
    bool sync(std::string serverURL, MelobaseCore::SequencesDB* database);

    std::vector<Float64> folderDates() { return _folderDates; }
    std::vector<UInt64> folderIDs() { return _folderIDs; }

    void setRemoteLocalFolderIDs(const std::map<UInt64, UInt64>& remoteLocalFolderIDs) {
        _remoteLocalFolderIDs = remoteLocalFolderIDs;
    }
    void setLocalRemoteFolderIDs(const std::map<UInt64, UInt64>& localRemoteFolderIDs) {
        _localRemoteFolderIDs = localRemoteFolderIDs;
    }

    void setDidSetProgressFn(DidSetProgressFnType didSetProgressFn) { _didSetProgressFn = didSetProgressFn; }
    void setUpdateLocalRemoteFoldersMappingFn(UpdateLocalRemoteFoldersMappingFnType updateLocalRemoteFoldersMappingFn) {
        _updateLocalRemoteFoldersMappingFn = updateLocalRemoteFoldersMappingFn;
    }
};

}  // namespace MelobaseCore

#endif  // FOLDERSSYNC_H
