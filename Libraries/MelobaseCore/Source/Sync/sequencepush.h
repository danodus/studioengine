//
//  sequencepush.h
//  MelobaseCore
//
//  Created by Daniel Cliche on 2021-02-01.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#ifndef SEQUENCEPUSH_H
#define SEQUENCEPUSH_H

#include <map>

#include "../sequencesdb.h"

namespace MelobaseCore {

class SequencePush {
    std::string stringFromSequence(std::shared_ptr<Sequence> sequence, bool areEventsIncluded);
    std::string stringFromSequenceAnnotations(const Sequence* sequence);
    std::string stringFromSequenceData(std::shared_ptr<Sequence> sequence);

   public:
    bool pushSequence(std::shared_ptr<Sequence> sequence, const std::string& serverURL, SequencesDB* database,
                      int maxAPI, std::map<UInt64, UInt64> localRemoteFolderIDs);
    bool updateSequence(std::shared_ptr<Sequence> sequence, UInt64 remoteID, int fields, const std::string& serverURL,
                        SequencesDB* database, int maxAPI, std::map<UInt64, UInt64> localRemoteFolderIDs);
};

}  // namespace MelobaseCore

#endif  // SEQUENCEPUSH_H
