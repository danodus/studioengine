//
//  sequencepull.cpp
//  MelobaseCore
//
//  Created by Daniel Cliche on 2021-02-01.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#include "sequencepull.h"

#define CPPHTTPLIB_USE_POLL
#include <httplib.h>

#include <sstream>

#include "sequenceparser.h"
#include "sync.h"

using namespace MelobaseCore;

// ---------------------------------------------------------------------------------------------------------------------
bool SequencePull::pullSequence(UInt64 remoteID, const std::string& serverURL, SequencesDB* database,
                                std::map<UInt64, UInt64> remoteLocalFolderIDs) {
    // Request the sequence

    std::stringstream request;
    request << "/sequences/" << remoteID << "?api=" << kSyncAPI;

    httplib::Client cli(serverURL.c_str());
    auto res = cli.Get(request.str().c_str());

    if (!res) return false;

    if (res->status != 200) return false;

    // Parse the response
    auto newSequence = std::shared_ptr<MelobaseCore::Sequence>(new MelobaseCore::Sequence);

    SequenceParser sequenceParser;

    UInt64 remoteFolderID = 0;
    bool isRemoteFolderIDAvailable = false;
    if (!sequenceParser.parseSequence(newSequence, res->body, &isRemoteFolderIDAvailable, &remoteFolderID)) return false;

    if (isRemoteFolderIDAvailable) {
        // Find the local folder ID associated with the remote folder ID
        auto folder = std::make_shared<MelobaseCore::SequencesFolder>();
        folder->id = remoteLocalFolderIDs[remoteFolderID];
        newSequence->folder = folder;
    } else {
        newSequence->folder = nullptr;
    }

    // We save it
    database->addSequence(newSequence);

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool SequencePull::updateSequence(std::shared_ptr<Sequence> sequence, UInt64 remoteID, int fields,
                                  const std::string& serverURL, SequencesDB* database,
                                  std::map<UInt64, UInt64> remoteLocalFolderIDs) {
    // Request the sequence

    std::stringstream request;
    request << "/sequences/" << remoteID << "?api=" << kSyncAPI
            << "&areEventsIncluded=" << ((fields & 0x2) ? "true" : "false");

    httplib::Client cli(serverURL.c_str());
    auto res = cli.Get(request.str().c_str());

    if (!res) return false;

    if (res->status != 200) return false;

    // Parse the response
    SequenceParser sequenceParser;

    database->readSequenceData(sequence);

    auto parsedSequence = std::make_shared<MelobaseCore::Sequence>();
    auto sequenceToSave = std::make_shared<MelobaseCore::Sequence>();

    bool isRemoteFolderIDAvailable = false;
    UInt64 remoteFolderID = 0;
    if (!sequenceParser.parseSequence(parsedSequence, res->body, &isRemoteFolderIDAvailable, &remoteFolderID)) return false;

    std::shared_ptr<MelobaseCore::SequencesFolder> parsedSequenceFolder = nullptr;

    if (isRemoteFolderIDAvailable) {
        parsedSequenceFolder = std::make_shared<MelobaseCore::SequencesFolder>();
        // Find the local folder ID associated with the remote folder ID
        parsedSequenceFolder->id = remoteLocalFolderIDs[remoteFolderID];
    }

    sequenceToSave->folder = (fields & 0x1) ? parsedSequenceFolder : sequence->folder;
    sequenceToSave->date = (fields & 0x1) ? parsedSequence->date : sequence->date;
    sequenceToSave->name = (fields & 0x1) ? parsedSequence->name : sequence->name;
    sequenceToSave->rating = (fields & 0x1) ? parsedSequence->rating : sequence->rating;
    sequenceToSave->playCount = (fields & 0x1) ? parsedSequence->playCount : sequence->playCount;
    sequenceToSave->version = (fields & 0x1) ? parsedSequence->version : sequence->version;
    sequenceToSave->annotations = (fields & 0x1) ? parsedSequence->annotations : sequence->annotations;

    sequenceToSave->dataVersion = (fields & 0x2) ? parsedSequence->dataVersion : sequence->dataVersion;
    sequenceToSave->data = (fields & 0x2) ? parsedSequence->data : sequence->data;

    sequenceToSave->id = sequence->id;
    sequenceToSave->data.id = sequence->data.id;

    // We save it
    database->updateSequences({sequenceToSave}, true, false, true, false);

    // We no longer need the sequence data, therefore we clear it from the database cache
    sequence->data.tracks.clear();

    return true;
}
