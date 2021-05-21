//
//  sequencessync.h
//  MelobaseCore
//
//  Created by Daniel Cliche on 2021-02-01.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#ifndef SEQUENCESSYNC_H
#define SEQUENCESSYNC_H

#include <expat.h>

#include <map>

#include "../sequencesdb.h"

namespace MelobaseCore {

class SequencesSync {
   public:
    typedef std::function<void(SequencesSync* sender, float progress, std::string description, std::vector<int> vars)>
        DidSetProgressFnType;

   private:
    std::string _serverURL;

    enum class ParserStates {
        Root,
        Sequences,
        SequencesSequence,
        SequencesSequenceID,
        SequencesSequenceFolderID,
        SequencesSequenceDate,
        SequencesSequenceName,
        SequencesSequenceRating,
        SequencesSequenceVersion,
        SequencesSequenceDataVersion,
        SequencesSequencePlayCount,
        SequencesSequenceAnnotations
    };
    ParserStates _parserState;

    UInt64 _sequenceID;
    Float64 _sequenceDate;
    Float64 _sequenceVersion;
    Float64 _sequenceDataVersion;

    bool _errorDetected = false;

    std::vector<Float64> _sequenceDates;
    std::vector<UInt64> _sequenceIDs;
    std::vector<Float64> _sequenceVersions;
    std::vector<Float64> _sequenceDataVersions;

    MelobaseCore::SequencesDB* _database;

    std::vector<UInt64> _sequenceIDsToPull;
    std::vector<std::shared_ptr<Sequence>> _sequencesToPush;
    std::vector<UInt64> _sequenceIDsToPullForUpdate;
    std::vector<std::shared_ptr<Sequence>> _sequencesToPullForUpdate;
    std::vector<UInt64> _sequenceIDsToPushForUpdate;
    std::vector<std::shared_ptr<Sequence>> _sequencesToPushForUpdate;

    std::vector<int> _pullFields;
    std::vector<int> _pushFields;

    size_t _countNbSequencesToSync;
    size_t _totalNbSequencesToSync;

    int _maxAPI;

    DidSetProgressFnType _didSetProgressFn = nullptr;

    static void XMLCALL start(void* data, const char* el, const char** attr);
    static void XMLCALL chars(void* data, const char* el, int len);
    static void XMLCALL end(void* data, const char* el);

    void getSequencesToSynchronize();
    bool pullSequences(std::map<UInt64, UInt64> remoteLocalFolderIDs);
    bool pullSequencesForUpdate(std::map<UInt64, UInt64> remoteLocalFolderIDs);
    bool pushSequences(std::map<UInt64, UInt64> localRemoteFolderIDs);
    bool pushSequencesForUpdate(std::map<UInt64, UInt64> localRemoteFolderIDs);
    bool getRemoteSequences(std::string serverURL);

   public:
    bool sync(std::string serverURL, MelobaseCore::SequencesDB* database,
              std::map<UInt64, UInt64> remoteLocalFolderIDs,
              std::map<UInt64, UInt64> localRemoteFolderIDs);

    void setDidSetProgressFn(DidSetProgressFnType didSetProgressFn) { _didSetProgressFn = didSetProgressFn; }
};

}  // namespace MelobaseCore

#endif  // SEQUENCESSYNC_H
