//
//  sequencessync.cpp
//  MelobaseCore
//
//  Created by Daniel Cliche on 2021-02-01.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#include "sequencessync.h"

#define CPPHTTPLIB_USE_POLL
#include <httplib.h>

#include <sstream>

#include "sequencepull.h"
#include "sequencepush.h"
#include "sync.h"

using namespace MelobaseCore;

// ---------------------------------------------------------------------------------------------------------------------
void XMLCALL SequencesSync::start(void* data, const char* el, const char** attr) {
    auto ss = reinterpret_cast<SequencesSync*>(data);

    auto elementName = std::string(el);

    std::map<std::string, std::string> attributeDict{};
    for (size_t i = 0; attr[i]; i += 2) attributeDict[attr[i]] = attr[i + 1];

    if (ss->_parserState == ParserStates::Root && elementName == "sequences") {
        ss->_parserState = ParserStates::Sequences;

        if (attributeDict.count("maxAPI") > 0) {
            ss->_maxAPI = std::stoi(attributeDict.at("maxAPI"));
            if (ss->_maxAPI > kSyncAPI) ss->_maxAPI = kSyncAPI;
        }
    } else if (ss->_parserState == ParserStates::Sequences && elementName == "sequence") {
        ss->_parserState = ParserStates::SequencesSequence;
    } else if (ss->_parserState == ParserStates::SequencesSequence && elementName == "id") {
        ss->_parserState = ParserStates::SequencesSequenceID;
    } else if (ss->_parserState == ParserStates::SequencesSequence && elementName == "folderid") {
        ss->_parserState = ParserStates::SequencesSequenceFolderID;
    } else if (ss->_parserState == ParserStates::SequencesSequence && elementName == "date") {
        ss->_parserState = ParserStates::SequencesSequenceDate;
    } else if (ss->_parserState == ParserStates::SequencesSequence && elementName == "name") {
        ss->_parserState = ParserStates::SequencesSequenceName;
    } else if (ss->_parserState == ParserStates::SequencesSequence && elementName == "rating") {
        ss->_parserState = ParserStates::SequencesSequenceRating;
    } else if (ss->_parserState == ParserStates::SequencesSequence && elementName == "version") {
        ss->_parserState = ParserStates::SequencesSequenceVersion;
    } else if (ss->_parserState == ParserStates::SequencesSequence && elementName == "dataVersion") {
        ss->_parserState = ParserStates::SequencesSequenceDataVersion;
    } else if (ss->_parserState == ParserStates::SequencesSequence && elementName == "playcount") {
        ss->_parserState = ParserStates::SequencesSequencePlayCount;
    } else if (ss->_parserState == ParserStates::SequencesSequence && elementName == "annotations") {
        ss->_parserState = ParserStates::SequencesSequenceAnnotations;
    } else {
        assert(0);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void XMLCALL SequencesSync::chars(void* data, const char* el, int len) {
    auto ss = reinterpret_cast<SequencesSync*>(data);

    auto string = std::string(el, len);

    switch (ss->_parserState) {
        case ParserStates::SequencesSequenceID:
            ss->_sequenceID = std::stoull(string);
            break;
        case ParserStates::SequencesSequenceFolderID:
            break;
        case ParserStates::SequencesSequenceDate:
            ss->_sequenceDate = std::stod(string);
            break;
        case ParserStates::SequencesSequenceName:
            break;
        case ParserStates::SequencesSequenceRating:
            break;
        case ParserStates::SequencesSequenceVersion:
            ss->_sequenceVersion = std::stod(string);
            break;
        case ParserStates::SequencesSequenceDataVersion:
            ss->_sequenceDataVersion = std::stod(string);
            break;
        case ParserStates::SequencesSequencePlayCount:
            break;
        case ParserStates::SequencesSequenceAnnotations:
            break;
        default:
            assert(0);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void XMLCALL SequencesSync::end(void* data, const char* el) {
    auto ss = reinterpret_cast<SequencesSync*>(data);

    auto elementName = std::string(el);

    if (ss->_parserState == ParserStates::Sequences && elementName == "sequences") {
        ss->_parserState = ParserStates::Root;
    } else if (ss->_parserState == ParserStates::SequencesSequence && elementName == "sequence") {
        // We add this sequence declaration

        ss->_sequenceDates.emplace_back(ss->_sequenceDate);
        ss->_sequenceIDs.emplace_back(ss->_sequenceID);
        ss->_sequenceVersions.emplace_back(ss->_sequenceVersion);
        ss->_sequenceDataVersions.emplace_back(ss->_sequenceDataVersion);

        ss->_parserState = ParserStates::Sequences;
    } else if (ss->_parserState == ParserStates::SequencesSequenceID && elementName == "id") {
        ss->_parserState = ParserStates::SequencesSequence;
    } else if (ss->_parserState == ParserStates::SequencesSequenceFolderID && elementName == "folderid") {
        ss->_parserState = ParserStates::SequencesSequence;
    } else if (ss->_parserState == ParserStates::SequencesSequenceDate && elementName == "date") {
        ss->_parserState = ParserStates::SequencesSequence;
    } else if (ss->_parserState == ParserStates::SequencesSequenceName && elementName == "name") {
        ss->_parserState = ParserStates::SequencesSequence;
    } else if (ss->_parserState == ParserStates::SequencesSequenceRating && elementName == "rating") {
        ss->_parserState = ParserStates::SequencesSequence;
    } else if (ss->_parserState == ParserStates::SequencesSequenceVersion && elementName == "version") {
        ss->_parserState = ParserStates::SequencesSequence;
    } else if (ss->_parserState == ParserStates::SequencesSequenceDataVersion && elementName == "dataVersion") {
        ss->_parserState = ParserStates::SequencesSequence;
    } else if (ss->_parserState == ParserStates::SequencesSequencePlayCount && elementName == "playcount") {
        ss->_parserState = ParserStates::SequencesSequence;
    } else if (ss->_parserState == ParserStates::SequencesSequenceAnnotations && elementName == "annotations") {
        ss->_parserState = ParserStates::SequencesSequence;
    } else {
        assert(0);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool SequencesSync::getRemoteSequences(std::string serverURL) {
    _errorDetected = false;

    _serverURL = serverURL;

    //
    // Request the list of sequences
    //

    if (_didSetProgressFn) _didSetProgressFn(this, 0.0f, "Requesting the list of sequences", {});

    std::stringstream request;
    request << "/sequences?api=" << kSyncAPI;

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
// Get a list of sequence IDs to synchronize
void SequencesSync::getSequencesToSynchronize() {
    // First, we get all the local sequence dates

    auto localSequences = _database->getSequences();

    // For each sequence declaration
    size_t index = 0;
    for (auto date : _sequenceDates) {
        // We check if the sequence exist in the database
        std::shared_ptr<Sequence> foundSequence;
        for (auto sequence : localSequences) {
            if (std::fabs(sequence->date - date) < 0.001) {
                foundSequence = sequence;
                break;
            }
        }

        // If no sequence has not been found
        if (!foundSequence) {
            // We do not have this sequence, so we add it to the pull list
            _sequenceIDsToPull.emplace_back(_sequenceIDs.at(index));
        } else {
            // We remove this sequence from our local list
            localSequences.erase(std::remove(localSequences.begin(), localSequences.end(), foundSequence),
                                 localSequences.end());

            // We do have this sequence, but it may need to be updated depending on the version
            auto sequenceToUpdate = foundSequence;
            // Note: The version is an integer at the server level
            auto localVersion = std::floor(sequenceToUpdate->version);
            auto remoteVersion = std::floor(_sequenceVersions.at(index));
            auto localDataVersion = std::floor(sequenceToUpdate->dataVersion);
            auto remoteDataVersion = std::floor(_sequenceDataVersions.at(index));

            // If the local version has en earlier time then the remote version
            if (localVersion < remoteVersion || localDataVersion < remoteDataVersion) {
                // Update this sequence

                // Find the fields to pull
                int fields = 0;
                if (localVersion < remoteVersion) fields |= 0x1;
                if (localDataVersion < remoteDataVersion) fields |= 0x2;

                _sequenceIDsToPullForUpdate.emplace_back(_sequenceIDs.at(index));
                _sequencesToPullForUpdate.emplace_back(sequenceToUpdate);
                _pullFields.emplace_back(fields);
            }

            if (localVersion > remoteVersion || localDataVersion > remoteDataVersion) {
                int fields = 0;
                if (localVersion > remoteVersion) fields |= 0x1;
                if (localDataVersion > remoteDataVersion) fields |= 0x2;

                _sequenceIDsToPushForUpdate.emplace_back(_sequenceIDs.at(index));
                _sequencesToPushForUpdate.emplace_back(sequenceToUpdate);
                _pushFields.emplace_back(fields);
            }
        }

        index++;
    }  // for each sequence declaration

    //
    // The remaining local sequences in our list are not present on the remote server.
    //

    for (auto sequence : localSequences) _sequencesToPush.emplace_back(sequence);
}

// ---------------------------------------------------------------------------------------------------------------------
bool SequencesSync::pullSequences(std::map<UInt64, UInt64> remoteLocalFolderIDs) {
    SequencePull pull;
    size_t sequenceIndex = 0;
    size_t totalNbSequences = _sequenceIDsToPull.size();
    for (auto pullSequenceID : _sequenceIDsToPull) {
        if (_didSetProgressFn) {
            _didSetProgressFn(this, (float)_countNbSequencesToSync / (float)_totalNbSequencesToSync,
                              "Downloading sequence %d of %d",
                              {static_cast<int>(sequenceIndex + 1), static_cast<int>(totalNbSequences)});
        }

        if (!(pull.pullSequence(pullSequenceID, _serverURL, _database, remoteLocalFolderIDs))) {
            _errorDetected = true;
            break;
        }
        sequenceIndex++;
        _countNbSequencesToSync++;
    }
    return !_errorDetected;
}

// ---------------------------------------------------------------------------------------------------------------------
bool SequencesSync::pullSequencesForUpdate(std::map<UInt64, UInt64> remoteLocalFolderIDs) {
    SequencePull pull;
    size_t sequenceIndex = 0;
    size_t totalNbSequences = _sequenceIDsToPullForUpdate.size();

    for (auto sequence : _sequencesToPullForUpdate) {
        if (_didSetProgressFn) {
            _didSetProgressFn(this, (float)_countNbSequencesToSync / (float)_totalNbSequencesToSync,
                              "Updating local sequence %d of %d",
                              {static_cast<int>(sequenceIndex + 1), static_cast<int>(totalNbSequences)});
        }

        if (!pull.updateSequence(sequence, _sequenceIDsToPullForUpdate.at(sequenceIndex), _pullFields.at(sequenceIndex),
                                 _serverURL, _database, remoteLocalFolderIDs)) {
            _errorDetected = true;
            break;
        }
        sequenceIndex++;
        _countNbSequencesToSync++;
    }
    return !_errorDetected;
}

// ---------------------------------------------------------------------------------------------------------------------
bool SequencesSync::pushSequences(std::map<UInt64, UInt64> localRemoteFolderIDs) {
    SequencePush push;
    size_t sequenceIndex = 0;
    size_t totalNbSequences = _sequencesToPush.size();

    for (auto sequence : _sequencesToPush) {
        if (_didSetProgressFn) {
            _didSetProgressFn(this, (float)_countNbSequencesToSync / (float)_totalNbSequencesToSync,
                              "Uploading sequence %d of %d",
                              {static_cast<int>(sequenceIndex + 1), static_cast<int>(totalNbSequences)});
        }
        if (!(push.pushSequence(sequence, _serverURL, _database, _maxAPI, localRemoteFolderIDs))) {
            _errorDetected = true;
            break;
        }
        sequenceIndex++;
        _countNbSequencesToSync++;
    }
    return !_errorDetected;
}

// ---------------------------------------------------------------------------------------------------------------------
bool SequencesSync::pushSequencesForUpdate(std::map<UInt64, UInt64> localRemoteFolderIDs) {
    SequencePush push;
    size_t sequenceIndex = 0;
    size_t totalNbSequences = _sequencesToPushForUpdate.size();
    for (auto sequence : _sequencesToPushForUpdate) {
        if (_didSetProgressFn) {
            _didSetProgressFn(this, (float)_countNbSequencesToSync / (float)_totalNbSequencesToSync,
                              "Updating remote sequence %d of %d",
                              {static_cast<int>(sequenceIndex + 1), static_cast<int>(totalNbSequences)});
        }

        if (!(push.updateSequence(sequence, _sequenceIDsToPushForUpdate.at(sequenceIndex),
                                  _pushFields.at(sequenceIndex), _serverURL, _database, _maxAPI,
                                  localRemoteFolderIDs))) {
            _errorDetected = true;
            break;
        }
        sequenceIndex++;
        _countNbSequencesToSync++;
    }
    return !_errorDetected;
}

// ---------------------------------------------------------------------------------------------------------------------
bool SequencesSync::sync(std::string serverURL, MelobaseCore::SequencesDB* database,
                         std::map<UInt64, UInt64> remoteLocalFolderIDs,
                         std::map<UInt64, UInt64> localRemoteFolderIDs) {
    // By default, the maximum API is 7 unless specified by the server
    _maxAPI = 7;
    _database = database;

    //
    // Get the remote folders
    //

    if (!getRemoteSequences(serverURL)) return false;

    //
    // We get the list of folders to synchronize
    //

    if (_didSetProgressFn) _didSetProgressFn(this, 0.0f, "Getting the list of sequences to synchronize", {});

    getSequencesToSynchronize();

    //
    // Set the progress counters
    //

    _countNbSequencesToSync = 0;
    _totalNbSequencesToSync = _sequenceIDsToPull.size() + _sequencesToPush.size() + _sequencesToPullForUpdate.size() +
                              _sequencesToPushForUpdate.size();

    // Disable the undo manager registration
    if (_database->undoManager()) _database->undoManager()->disableRegistration();

    //
    // Pull the sequences
    //

    if (!_errorDetected) pullSequences(remoteLocalFolderIDs);

    if (!_errorDetected) pullSequencesForUpdate(remoteLocalFolderIDs);

    //
    // We push the sequences
    //

    if (!_errorDetected) pushSequences(localRemoteFolderIDs);

    if (!_errorDetected) pushSequencesForUpdate(localRemoteFolderIDs);

    // Enable undo manager registration
    if (_database->undoManager()) _database->undoManager()->enableRegistration();

    if (_errorDetected) return false;

    return true;
}
