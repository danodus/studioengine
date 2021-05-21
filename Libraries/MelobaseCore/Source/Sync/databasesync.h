//
//  databasesync.h
//  MelobaseCore
//
//  Created by Daniel Cliche on 2021-01-29.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#ifndef DATABASESYNC_H
#define DATABASESYNC_H

#define DATABASE_SYNC_SUCCESS                     0
#define DATABASE_SYNC_COMMUNICATION_ERROR         1
#define DATABASE_SYNC_INCOMPATIBLE_SERVER_ERROR   2

#include <map>

#include "../sequencesdb.h"

namespace MelobaseCore {
class DatabaseSync {
   public:
    typedef std::function<void(DatabaseSync* sender, float progress, const std::string& description, const std::vector<int>& vars)> DidSetProgressFnType;

    std::map<UInt64, UInt64> _remoteLocalFolderIDs;
    std::map<UInt64, UInt64> _localRemoteFolderIDs;

    DidSetProgressFnType _didSetProgressFn = nullptr;

    bool getRemoteLocalFolderIDs(const std::string& serverURL, MelobaseCore::SequencesDB* database);
    bool checkServerCompatibility(const std::string& serverURL, int *error);
    bool requestSave(const std::string& serverURL);

   public:
    bool sync(const std::string& serverURL, SequencesDB* database, int *error);

    void setDidSetProgressFn(DidSetProgressFnType didSetProgressFn) { _didSetProgressFn = didSetProgressFn; }
};
}  // namespace MelobaseCore

#endif  // DATABASESYNC_H
