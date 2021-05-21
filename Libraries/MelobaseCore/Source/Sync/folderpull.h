//
//  folderpull.h
//  MelobaseCore
//
//  Created by Daniel Cliche on 2021-01-25.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#ifndef FOLDERPULL_H
#define FOLDERPULL_H

#include <map>

#include "../sequencesdb.h"

namespace MelobaseCore {

class FolderPull {
   public:
    bool pullFolder(UInt64 remoteID, const std::string& serverURL, SequencesDB* database,
                    std::map<UInt64, UInt64> remoteLocalFolderIDs);
    bool updateFolder(SequencesFolder* folder, UInt64 remoteID, const std::string& serverURL, SequencesDB* database,
                      std::map<UInt64, UInt64> remoteLocalFolderIDs);
};

}  // namespace MelobaseCore

#endif  // FOLDERPULL_H
