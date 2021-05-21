//
//  sequencepush.cpp
//  MelobaseCore
//
//  Created by Daniel Cliche on 2021-02-01.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#include "sequencepush.h"

#define CPPHTTPLIB_USE_POLL
#include <httplib.h>

#include <sstream>

#include "../utils.h"

using namespace MelobaseCore;

// ---------------------------------------------------------------------------------------------------------------------
std::string SequencePush::stringFromSequence(std::shared_ptr<Sequence> sequence, bool areEventsIncluded) {
    std::stringstream ss;

    ss << std::fixed << "date=" << sequence->date << "&";
    if (sequence->folder) ss << "folderid=" << sequence->folder->id << "&";
    ss << "name=" << httplib::detail::encode_query_param(sequence->name) << "&"
       << "rating=" << sequence->rating << "&"
       << "version=" << sequence->version << "&"
       << "dataVersion=" << sequence->dataVersion << "&"
       << "playcount=" << sequence->playCount << "&"
       << "tickperiod=" << sequence->data.tickPeriod << "&"
       << "annotations=" << stringFromSequenceAnnotations(sequence.get()) << "&"
       << "dataFormat=" << 1;

    if (areEventsIncluded) {
        ss << "&data=" << stringFromSequenceData(sequence);
    }

    return ss.str();
}

// ---------------------------------------------------------------------------------------------------------------------
std::string SequencePush::stringFromSequenceAnnotations(const Sequence* sequence) {
    std::string s;
    MelobaseCore::base64Encode(s, MelobaseCore::getSequenceAnnotationsBlob(sequence));

    return s;
}

// ---------------------------------------------------------------------------------------------------------------------
std::string SequencePush::stringFromSequenceData(std::shared_ptr<Sequence> sequence) {
    std::string s;
    MelobaseCore::base64Encode(s, MelobaseCore::getSequenceDataBlob(sequence, true));

    return s;
}

// ---------------------------------------------------------------------------------------------------------------------
bool SequencePush::pushSequence(std::shared_ptr<Sequence> sequence, const std::string& serverURL, SequencesDB* database,
                                int maxAPI, std::map<UInt64, UInt64> localRemoteFolderIDs) {
    //
    // Post the sequence
    //

    // Read the sequence data
    database->readSequenceData(sequence);

    unsigned long localFolderID = 0;
    if (sequence->folder) {
        localFolderID = sequence->folder->id;
        sequence->folder->id = localRemoteFolderIDs[localFolderID];
    }

    std::stringstream request;
    request << "/sequences?api=" << maxAPI << "&" << stringFromSequence(sequence, maxAPI < 4);

    httplib::Client cli(serverURL.c_str());

    if (sequence->folder) {
        sequence->folder->id = localFolderID;
    }

    bool isSuccessful = true;
    if (maxAPI >= 6) {
        auto res = cli.Post(request.str().c_str(), stringFromSequenceData(sequence), "text/plain");
        if (!res || res->status != 200) isSuccessful = false;
    } else {
        auto res = cli.Post(request.str().c_str());
        if (!res || res->status != 200) isSuccessful = false;
    }

    // We no longer need the sequence data, therefore we clear it from the database cache
    sequence->data.tracks.clear();

    return isSuccessful;
}

// ---------------------------------------------------------------------------------------------------------------------
bool SequencePush::updateSequence(std::shared_ptr<Sequence> sequence, UInt64 remoteID, int fields,
                                  const std::string& serverURL, SequencesDB* database, int maxAPI,
                                  std::map<UInt64, UInt64> localRemoteFolderIDs) {
    // Post the folder

    // Read the sequence data
    database->readSequenceData(sequence);

    unsigned long localFolderID = 0;
    if (sequence->folder) {
        localFolderID = sequence->folder->id;
        sequence->folder->id = localRemoteFolderIDs[localFolderID];
    }

    std::stringstream request;
    request << "/sequences/" << remoteID << "?api=" << maxAPI << "&"
            << stringFromSequence(sequence, (maxAPI >= 4) ? false : (fields & 0x2));

    httplib::Client cli(serverURL.c_str());

    if (sequence->folder) {
        sequence->folder->id = localFolderID;
    }

    bool isSuccessful = true;
    if (fields & 0x2) {
        auto res = cli.Post(request.str().c_str(), stringFromSequenceData(sequence), "text/plain");
        if (!res || res->status != 200) isSuccessful = false;
    } else {
        auto res = cli.Post(request.str().c_str());
        if (!res || res->status != 200) isSuccessful = false;
    }

    // We no longer need the sequence data, therefore we clear it from the database cache
    sequence->data.tracks.clear();

    return isSuccessful;
}
