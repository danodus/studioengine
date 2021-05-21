//
//  folderpush.h
//  MelobaseCore
//
//  Created by Daniel Cliche on 2021-01-25.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#ifndef FOLDERPUSH_H
#define FOLDERPUSH_H

#include <map>

#include "../sequencesdb.h"

namespace MelobaseCore {

class FolderPush {
    std::string stringFromFolder(SequencesFolder* folder);

   public:
    bool pushFolder(SequencesFolder* folder, const std::string& serverURL, SequencesDB* database, int maxAPI,
                    std::map<UInt64, UInt64> localRemoteFolderIDs);
    bool updateFolder(SequencesFolder* folder, UInt64 remoteID, const std::string& serverURL, SequencesDB* database,
                      int maxAPI, std::map<UInt64, UInt64> localRemoteFolderIDs);
};

}  // namespace MelobaseCore

#endif  // FOLDERPUSH_H
