//
//  sequencepull.h
//  MelobaseCore
//
//  Created by Daniel Cliche on 2021-02-01.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#ifndef SEQUENCEPULL_H
#define SEQUENCEPULL_H

#include <map>

#include "../sequencesdb.h"

namespace MelobaseCore {

class SequencePull {
   public:
    bool pullSequence(UInt64 remoteID, const std::string& serverURL, SequencesDB* database,
                      std::map<UInt64, UInt64> remoteLocalFolderIDs);
    bool updateSequence(std::shared_ptr<Sequence> sequence, UInt64 remoteID, int fields, const std::string& serverURL,
                        SequencesDB* database, std::map<UInt64, UInt64> remoteLocalFolderIDs);
};

}  // namespace MelobaseCore

#endif  // SEQUENCEPULL_H
