//
//  sequencesdb.h
//  MelobaseCore
//
//  Created by Daniel Cliche on 2014-06-15.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#ifndef SEQUENCESDB_H
#define SEQUENCESDB_H

#include <DB/db.h>
#include <undomanager.h>

#include <functional>
#include <memory>
#include <mutex>
#include <string>

#include "melobasecore_sequence.h"

// Standard folder IDs

#define ROOT_FOLDER_ID 0
#define TRASH_FOLDER_ID 1
#define SEQUENCES_FOLDER_ID 2
#define RESERVED0_FOLDER_ID 3
#define RESERVED1_FOLDER_ID 4
#define RESERVED2_FOLDER_ID 5
#define RESERVED3_FOLDER_ID 6
#define RESERVED4_FOLDER_ID 7
#define RESERVED5_FOLDER_ID 8
#define RESERVED6_FOLDER_ID 9
#define LAST_STANDARD_FOLDER_ID RESERVED6_FOLDER_ID

namespace MelobaseCore {

class SequencesDB {
   public:
    typedef std::function<void(SequencesDB* sender)> sequenceAddedFnType;
    typedef std::function<void(SequencesDB* sender, std::shared_ptr<SequencesFolder> folder)> folderAddedFnType;
    typedef std::function<void(SequencesDB* sender, std::shared_ptr<Sequence> sequence)> willRemoveSequenceFnType;
    typedef std::function<void(SequencesDB* sender, std::shared_ptr<SequencesFolder> folder)> willRemoveFolderFnType;
    typedef std::function<void(SequencesDB* sender)> sequenceRemovedFnType;
    typedef std::function<void(SequencesDB* sender)> folderRemovedFnType;
    typedef std::function<void(SequencesDB* sender)> sequenceUpdatedFnType;
    typedef std::function<void(SequencesDB* sender)> folderUpdatedFnType;
    typedef enum { None, Trash, All, New, Annotated, Filter1, Filter2, Filter3, Filter4, Filter5 } sequencesFilterEnum;
    typedef enum { Date, Rating, Name } sequencesOrderFieldEnum;
    typedef enum { Ascending, Descending } orderDirectionEnum;

   private:
    MDStudio::DB* _db;
    std::string _dbPath;
    std::mutex _dbMutex;

    sequenceAddedFnType _sequenceAddedFn;
    folderAddedFnType _folderAddedFn;
    willRemoveSequenceFnType _willRemoveSequenceFn;
    willRemoveFolderFnType _willRemoveFolderFn;
    sequenceRemovedFnType _sequenceRemovedFn;
    folderRemovedFnType _folderRemovedFn;
    sequenceUpdatedFnType _sequenceUpdatedFn;
    folderUpdatedFnType _folderUpdatedFn;

    UInt64 _maxSequenceID;
    UInt64 _maxSequenceDataID;
    UInt64 _maxSequencesFolderID;

    void setSequence(std::shared_ptr<Sequence> sequence, std::vector<std::pair<std::string, std::string>> columns);
    void setSequenceData(SequenceData* sequenceData, std::vector<std::pair<std::string, std::string>> columns);
    void setFolder(std::shared_ptr<SequencesFolder> folder, std::vector<std::pair<std::string, std::string>> columns);

    bool readSequenceAnnotations(Sequence* sequence);

    std::string filterString(sequencesFilterEnum filter, std::string nameSearch,
                             std::shared_ptr<SequencesFolder> folder, bool isIncludingSubfolders);
    std::string orderString(sequencesOrderFieldEnum orderField, orderDirectionEnum orderDirection);

    std::shared_ptr<SequencesFolder> getFolderWithIDInternal(UInt64 id);

    bool addStandardFolders();
    bool validateAndFixStandardFolders();

    MDStudio::UndoManager* _undoManager;

    bool _isSequencesCacheValid, _isFoldersCacheValid;
    std::vector<std::shared_ptr<Sequence>> _cachedSequences;
    std::vector<std::shared_ptr<SequencesFolder>> _cachedFolders;

   public:
    SequencesDB(std::string dbPath, MDStudio::UndoManager* undoManager = nullptr);
    ~SequencesDB();

    bool open(bool isInitiallyEmpty = false);

    // Thread-safe
    bool addSequence(std::shared_ptr<Sequence> sequence, bool isDelegateNotified = true, bool isSpecificID = false,
                     UInt64 specificID = 0L, UInt64 specificDataID = 0L,
                     const std::vector<UInt64>& specificAnnotationIDs = {});
    bool addFolder(std::shared_ptr<SequencesFolder> folder, bool isDelegateNotified = true, bool isSpecificID = false,
                   unsigned long specificID = 0L, bool isInsideTransaction = false);

    // Thread-safe
    bool removeSequence(std::shared_ptr<Sequence> sequence, bool isDelegateNotified = true);
    bool removeFolder(std::shared_ptr<SequencesFolder> folder, bool isDelegateNotified = true);

    // Thread-safe
    std::vector<std::shared_ptr<Sequence>> getSequences();
    std::vector<std::shared_ptr<SequencesFolder>> getFolders(std::shared_ptr<SequencesFolder> parentFolder);

    // Thread-safe
    void invalidateSequencesCache();
    void invalidateFoldersCache();

    // Thread-safe
    unsigned long getNbSequences(sequencesFilterEnum filter, std::string nameSearch,
                                 std::shared_ptr<SequencesFolder> folder, bool isIncludingSubfolders);
    unsigned long getNbFolders(std::shared_ptr<SequencesFolder> parentFolder);

    // Thread-safe
    std::shared_ptr<Sequence> getSequence(unsigned long index, sequencesFilterEnum filter, std::string nameSearch,
                                          std::shared_ptr<SequencesFolder> folder, bool isIncludingSubfolders,
                                          sequencesOrderFieldEnum orderField = Date,
                                          orderDirectionEnum orderDirection = Descending);
    std::shared_ptr<SequencesFolder> getFolder(unsigned long index, std::shared_ptr<SequencesFolder> folder);

    // Thread-safe
    std::shared_ptr<Sequence> getSequenceWithID(UInt64 id);
    std::shared_ptr<SequencesFolder> getFolderWithID(UInt64 id);

    // Thread-safe
    bool readSequenceData(std::shared_ptr<Sequence> sequence);

    // Thread-safe
    bool updateSequences(std::vector<std::shared_ptr<Sequence>> sequences, bool isDelegateNotified = true,
                         bool isVersionUpdated = true, bool isDataUpdated = false, bool isDataVersionUpdated = false);
    bool updateFolder(std::shared_ptr<SequencesFolder> folder, bool isDelegateNotified = true,
                      bool isVersionUpdated = true);

    // Thread-safe
    bool promoteAllSequences(std::shared_ptr<SequencesFolder> folder, bool isIncludingSubfolders,
                             bool isDelegateNotified = true);

    // Thread-safe
    bool demoteAllSequences(std::shared_ptr<SequencesFolder> folder, bool isIncludingSubfolders,
                            bool isDelegateNotified = true);

    // Thread-safe
    bool setAsPlayedAllSequences(std::shared_ptr<SequencesFolder> folder, bool isIncludingSubfolders,
                                 bool isDelegateNotified = true);

    // Thread-safe
    bool cleanupSequences(std::shared_ptr<SequencesFolder> folder, bool isIncludingSubfolders,
                          bool isDelegateNotified = true);

    // Thread-safe
    bool emptyTrash(bool isDelegateNotified = true);

    void setSequenceAddedFn(sequenceAddedFnType sequenceAddedFn) { _sequenceAddedFn = sequenceAddedFn; }
    void setFolderAddedFn(folderAddedFnType folderAddedFn) { _folderAddedFn = folderAddedFn; }
    void setWillRemoveSequenceFn(willRemoveSequenceFnType willRemoveSequenceFn) {
        _willRemoveSequenceFn = willRemoveSequenceFn;
    }
    void setWillRemoveFolderFn(willRemoveFolderFnType willRemoveFolderFn) { _willRemoveFolderFn = willRemoveFolderFn; }
    void setSequenceRemovedFn(sequenceRemovedFnType sequenceRemovedFn) { _sequenceRemovedFn = sequenceRemovedFn; }
    void setFolderRemovedFn(folderRemovedFnType folderRemovedFn) { _folderRemovedFn = folderRemovedFn; }
    void setSequenceUpdatedFn(sequenceUpdatedFnType sequenceUpdatedFn) { _sequenceUpdatedFn = sequenceUpdatedFn; }
    void setFolderUpdatedFn(folderUpdatedFnType folderUpdatedFn) { _folderUpdatedFn = folderUpdatedFn; }

    MDStudio::UndoManager* undoManager() { return _undoManager; }
};

//
// Utilities
//

std::vector<char> base64Decode(const char* encodedData);
void base64Encode(std::string& dataEncoded, const std::vector<char>& data);

std::vector<char> getSequenceDataBlob(std::shared_ptr<Sequence> sequence, bool isVLE);
bool setSequenceDataFromBlob(std::shared_ptr<Sequence> sequence, char* blob, size_t size);

std::vector<char> getSequenceAnnotationsBlob(const Sequence* sequence);
bool setSequenceAnnotationsFromBlob(Sequence* sequence, const char* blob, size_t size);

}  // namespace MelobaseCore

#endif  // SEQUENCESDB_H
