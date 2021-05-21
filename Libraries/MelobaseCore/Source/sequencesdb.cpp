//
//  sequencesdb.cpp
//  MelobaseCore
//
//  Created by Daniel Cliche on 2014-06-15.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#include "sequencesdb.h"

#include <string.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>

#include "base64.h"
#include "platform.h"
#include "plist.h"

using namespace MelobaseCore;

// ---------------------------------------------------------------------------------------------------------------------
std::vector<char> MelobaseCore::base64Decode(const char* encodedData) {
    using namespace std;

    vector<char> data;
    insert_iterator<vector<char>> ii(data, data.begin());
    base64<char> b64;
    int state = 0;
    b64.get(encodedData, encodedData + strlen(encodedData), ii, state);

    return data;
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::base64Encode(std::string& dataEncoded, const std::vector<char>& data) {
    using namespace std;
    dataEncoded.clear();
    insert_iterator<string> ii(dataEncoded, dataEncoded.begin());
    base64<char> b64;
    int state = 0;

#if defined(_WIN32) || defined(_WIN64)
    b64.put(data.begin(), data.end(), ii, state, base64<>::crlf());
#else
    b64.put(data.begin(), data.end(), ii, state, base64<>::lf());
#endif
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<char> MelobaseCore::getSequenceDataBlob(std::shared_ptr<Sequence> sequence, bool isVLE) {
    Plist::dictionary_type plistMain;

    Plist::array_type plistTracks;
    // For each track
    for (auto track : sequence->data.tracks) {
        Plist::array_type plistClips;

        // For each clip
        for (auto clip : track->clips) {
            // Create an array of events
            Plist::data_type plistEvents;

            for (auto event : clip->events) {
                std::vector<char> data = encodeEvent(event, isVLE);
                plistEvents.insert(plistEvents.end(), data.begin(), data.end());
            }

            Plist::dictionary_type plistClip;
            plistClip.insert(std::pair<std::string, Any>(isVLE ? "$eventsVLE" : "$events", plistEvents));
            plistClips.push_back(plistClip);
        }

        Plist::dictionary_type plistTrack;
        plistTrack.insert(std::pair<std::string, Any>("$name", track->name));
        plistTrack.insert(std::pair<std::string, Any>("$channel", (Plist::integer_type)track->channel));
        plistTrack.insert(std::pair<std::string, Any>("$clips", plistClips));
        plistTracks.push_back(plistTrack);
    }

    Plist::dictionary_type plistSequence;
    plistSequence.insert(std::pair<std::string, Any>("$format", (Plist::integer_type)sequence->data.format));
    plistSequence.insert(std::pair<std::string, Any>("$tracks", plistTracks));
    plistMain.insert(std::pair<std::string, Any>("$sequence", plistSequence));

    std::vector<char> plist;
    Plist::writePlistBinary(plist, plistMain);

    return plist;
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::setSequenceDataFromBlob(std::shared_ptr<Sequence> sequence, char* blob, size_t size) {
    Any message;

    try {
        Plist::readPlist(blob, size, message);
    } catch (Plist::Error err) {
        return false;
    }

    Plist::dictionary_type d = message;

    if (d.find("$sequence") != d.end()) {
        // This is a Melobase Core sequence

        sequence->data.tracks.clear();

        Plist::dictionary_type plistSequence = d["$sequence"];

        if (plistSequence.find("$format") != plistSequence.end()) {
            Plist::integer_type format = plistSequence["$format"];
            sequence->data.format = (UInt8)format;
        }

        Plist::array_type plistTracks = plistSequence["$tracks"];

        for (Plist::dictionary_type plistTrack : plistTracks) {
            auto track = std::make_shared<Track>();

            if (plistTrack.find("$name") != plistTrack.end()) {
                Plist::string_type name = plistTrack["$name"];
                track->name = name;
            }

            if (plistTrack.find("$channel") != plistTrack.end()) {
                Plist::integer_type channel = plistTrack["$channel"];
                track->channel = (UInt8)channel;
            }

            track->clips.clear();

            Plist::array_type plistClips = plistTrack["$clips"];

            for (Plist::dictionary_type plistClip : plistClips) {
                auto clip = std::make_shared<Clip>();

                Plist::boolean_type isVLE = false;
                Plist::data_type data;
                if (plistClip.find("$eventsVLE") != plistClip.end()) {
                    isVLE = true;
                    data = plistClip["$eventsVLE"].as<Plist::data_type>();
                } else {
                    data = plistClip["$events"].as<Plist::data_type>();
                }
                for (size_t i = 0; i < data.size();) {
                    size_t size = 0;
                    auto event = decodeEvent(&data[i], &size, isVLE);
                    assert(event);
                    clip->events.push_back(event);
                    i += size;
                }
                track->clips.push_back(clip);
            }

            sequence->data.tracks.push_back(track);
        }
    } else if (d.find("$events") != d.end()) {
        struct OldEvent {
            UInt8 type;
            UInt8 channel;
            UInt32 tickCount;
            SInt32 param1;
            SInt32 param2;
        };

        // This is a studio sequence
        std::shared_ptr<MDStudio::Sequence> studioSequence = std::make_shared<MDStudio::Sequence>();

        Plist::data_type data = d["$events"];
        for (size_t i = 0; i < data.size(); i += sizeof(OldEvent)) {
            OldEvent* oldEvent = (OldEvent*)(&data[i]);
            MDStudio::Event event = MDStudio::makeEvent(oldEvent->type, oldEvent->channel, oldEvent->tickCount,
                                                        oldEvent->param1, oldEvent->param2);
            studioSequence->data.tracks[0].events.push_back(event);
        }
        std::shared_ptr<Sequence> melobaseCoreSequence = getMelobaseCoreSequence(studioSequence);
        sequence->data.tracks = melobaseCoreSequence->data.tracks;

    } else if (d.find("$objects") != d.end()) {
        // Note: This format is deprecated

        // This is a studio sequence
        std::shared_ptr<MDStudio::Sequence> studioSequence = std::make_shared<MDStudio::Sequence>();

        Plist::array_type a = d["$objects"];

        for (auto o : a) {
            if (o.is<Plist::dictionary_type>()) {
                Plist::dictionary_type d1 = o;

                MDStudio::Event event;
                for (auto it = d1.cbegin(); it != d1.cend(); ++it) {
                    if (it->first == "$0") {
                        event.type = it->second.as<Plist::integer_type>();
                    } else if (it->first == "$1") {
                        event.channel = it->second.as<Plist::integer_type>();
                    } else if (it->first == "$2") {
                        event.tickCount = static_cast<UInt32>(it->second.as<Plist::integer_type>());
                    } else if (it->first == "$3") {
                        event.param1 = static_cast<SInt32>(it->second.as<Plist::integer_type>());
                    } else if (it->first == "$4") {
                        event.param2 = static_cast<SInt32>(it->second.as<Plist::integer_type>());
                        studioSequence->data.tracks[0].events.push_back(event);
                    }
                }
            }
        }

        std::shared_ptr<Sequence> melobaseCoreSequence = getMelobaseCoreSequence(studioSequence);
        sequence->data.tracks = melobaseCoreSequence->data.tracks;
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<char> MelobaseCore::getSequenceAnnotationsBlob(const Sequence* sequence) {
    // If no annotation is present, return an empty vector
    if (sequence->annotations.empty()) return {};

    Plist::array_type plistAnnotations;
    // For each annotation
    for (auto annotation : sequence->annotations) {
        Plist::dictionary_type plistAnnotation;
        plistAnnotation.insert(std::pair<std::string, Any>("$tickCount", (Plist::integer_type)annotation->tickCount));
        plistAnnotations.push_back(plistAnnotation);
    }

    std::vector<char> plist;
    Plist::writePlistBinary(plist, plistAnnotations);

    return plist;
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::setSequenceAnnotationsFromBlob(Sequence* sequence, const char* blob, size_t size) {
    sequence->annotations.clear();

    // If no annotations are found, return right away
    if (size == 0) return true;

    Any message;

    try {
        Plist::readPlist(blob, size, message);
    } catch (Plist::Error err) {
        return false;
    }

    Plist::array_type plistAnnotations = message;

    for (Plist::dictionary_type plistAnnotation : plistAnnotations) {
        auto annotation = std::make_shared<SequenceAnnotation>();
        if (plistAnnotation.find("$tickCount") != plistAnnotation.end()) {
            Plist::integer_type plistTickCount = plistAnnotation["$tickCount"];
            annotation->tickCount = static_cast<UInt32>(plistTickCount);
        }
        sequence->annotations.emplace_back(annotation);
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
static std::string sanitizedSQLString(std::string s) {
    std::string ret;
    for (auto c : s) {
        if (c == '\'') {
            ret += "\'\'";
        } else {
            ret += c;
        }
    }
    return ret;
}

// ---------------------------------------------------------------------------------------------------------------------
MelobaseCore::SequencesDB::SequencesDB(std::string dbPath, MDStudio::UndoManager* undoManager)
    : _dbPath(dbPath), _undoManager(undoManager) {
    _sequenceAddedFn = nullptr;
    _folderAddedFn = nullptr;
    _willRemoveSequenceFn = nullptr;
    _willRemoveFolderFn = nullptr;
    _sequenceRemovedFn = nullptr;
    _folderRemovedFn = nullptr;
    _sequenceUpdatedFn = nullptr;
    _folderUpdatedFn = nullptr;

    _maxSequenceID = 0L;
    _maxSequenceDataID = 0L;
    _maxSequencesFolderID = 0L;

    _isSequencesCacheValid = false;
    _isFoldersCacheValid = false;

    _db = new MDStudio::DB();
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::SequencesDB::addStandardFolders() {
    // Add root, trash and draft folders
    auto rootFolder = std::make_shared<SequencesFolder>();
    rootFolder->name = "Root";
    if (!addFolder(rootFolder, false, true, ROOT_FOLDER_ID, true)) return false;

    auto trashFolder = std::make_shared<SequencesFolder>();
    trashFolder->name = "Trash";
    trashFolder->parent = rootFolder;
    if (!addFolder(trashFolder, false, true, TRASH_FOLDER_ID, true)) return false;

    auto sequencesFolder = std::make_shared<SequencesFolder>();
    sequencesFolder->name = "Sequences";
    sequencesFolder->parent = rootFolder;
    if (!addFolder(sequencesFolder, false, true, SEQUENCES_FOLDER_ID, true)) return false;

    // Add reserved folders (id 3 to 9)
    for (int i = 0; i < 7; ++i) {
        if (!addFolder(nullptr, false, true, RESERVED0_FOLDER_ID + i, true)) return false;
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::SequencesDB::validateAndFixStandardFolders() {
    invalidateFoldersCache();

    bool areStandardFoldersValid = true;

    // Check Foot folder
    if (!getFolder(ROOT_FOLDER_ID, nullptr)) areStandardFoldersValid = false;

    // Check Standard folder
    if (areStandardFoldersValid && !getFolder(TRASH_FOLDER_ID, nullptr)) areStandardFoldersValid = false;

    // Check Sequences folder
    if (areStandardFoldersValid && !getFolder(SEQUENCES_FOLDER_ID, nullptr)) areStandardFoldersValid = false;

    // If one or more standard folders are invalid
    if (!areStandardFoldersValid) {
        std::cout << "The standard folders are invalid! Fixing..." << std::endl;

        if (!_db->exec("BEGIN TRANSACTION;\n", false)) return false;

        // Remove all the folders
        if (!_db->exec("DELETE FROM `ZMDSEQUENCESFOLDER`;\n", false)) return false;

        if (!_db->exec("UPDATE `Z_PRIMARYKEY` SET Z_MAX=0 WHERE Z_NAME='MDSequencesFolder';\n")) return false;

        if (!addStandardFolders()) return false;

        // Move all the sequences to the Sequences folder
        if (!_db->exec("UPDATE `ZMDSEQUENCE` SET ZFOLDER=2;\n", false)) return false;

        if (!_db->exec("COMMIT;\n", true)) return false;

        std::cout << "Fix performed successfully." << std::endl;
    }

    invalidateFoldersCache();

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::SequencesDB::open(bool isInitiallyEmpty) {
    const char* path = _dbPath.c_str();

    std::ifstream ifile(path);

    bool isNew = false;

    if (ifile) {
        ifile.close();

        if (isInitiallyEmpty) {
            std::remove(path);
            isNew = true;
        }
    } else {
        isNew = true;
    }

    if (!_db->open(path)) return false;

    if (isNew) {
        _db->clearResults();
        if (!_db->exec("BEGIN TRANSACTION;\n", false)) return false;
        if (!_db->exec("CREATE TABLE Z_PRIMARYKEY (Z_ENT INTEGER PRIMARY KEY, Z_NAME VARCHAR, Z_SUPER INTEGER, Z_MAX "
                       "INTEGER);\n",
                       true))
            return false;
        if (!_db->exec("INSERT INTO `Z_PRIMARYKEY` VALUES(1,'MDSequence',0,0);\n", true)) return false;
        if (!_db->exec("INSERT INTO `Z_PRIMARYKEY` VALUES(2,'MDSequenceData',0,0);\n", true)) return false;
        if (!_db->exec("INSERT INTO `Z_PRIMARYKEY` VALUES(3,'MDSequencesFolder',0,0);\n", true)) return false;
        if (!_db->exec(
                "CREATE TABLE ZMDSEQUENCESFOLDER ( Z_PK INTEGER PRIMARY KEY, Z_ENT INTEGER, Z_OPT INTEGER, ZPARENT "
                "INTEGER, ZDESC VARCHAR, ZNAME VARCHAR, ZDATE TIMESTAMP, ZRATING FLOAT, ZVERSION INTEGER);\n",
                true))
            return false;
        if (!_db->exec("CREATE TABLE ZMDSEQUENCEDATA ( Z_PK INTEGER PRIMARY KEY, Z_ENT INTEGER, Z_OPT INTEGER, "
                       "ZCURRENTPOSITION INTEGER, ZSEQUENCE INTEGER, ZTICKPERIOD FLOAT, ZEVENTS BLOB );\n",
                       true))
            return false;
        if (!_db->exec("CREATE TABLE ZMDSEQUENCE ( Z_PK INTEGER PRIMARY KEY, Z_ENT INTEGER, Z_OPT INTEGER, ZPLAYCOUNT "
                       "INTEGER, ZVERSION INTEGER, ZDATA INTEGER, ZFOLDER INTEGER, ZDATE TIMESTAMP, ZRATING FLOAT, "
                       "ZDESC VARCHAR, ZNAME VARCHAR, ZDATAVERSION INTEGER, ZANNOTATIONS BLOB );\n",
                       true))
            return false;

        // Add standard folders
        if (!addStandardFolders()) return false;

        // Update database version
        if (!_db->exec("PRAGMA user_version = 4;\n", false)) return false;
        if (!_db->exec("COMMIT;\n", true)) return false;

    } else {
        // Read the current version
        _db->clearResults();

        if (!_db->exec("BEGIN TRANSACTION;\n", false)) return false;
        if (!_db->exec("PRAGMA user_version;\n", false)) return false;
        if (!_db->exec("COMMIT;\n", true)) return false;

        unsigned long version = std::stol(_db->rows()[0][0].second);

        std::cout << "Database version: " << version << std::endl;

        // Check if the application is out-dated
        if (version > 4) {
            std::cout << "The database is more recent than the application therefore it cannot be opened." << std::endl;
            _db->close();
            if (MDStudio::Platform::sharedInstance()->language() == "fr") {
                MDStudio::Platform::sharedInstance()->alert(
                    "Impossible d'ouvrir la base de données",
                    "La base de données a été modifiée par une version plus récente de l'application. Veuillez "
                    "télécharger la dernière version à www.melobase.com.",
                    true);
            } else {
                MDStudio::Platform::sharedInstance()->alert(
                    "Unable to open the database",
                    "The database has been modified by a more recent version of the application. Please download the "
                    "latest version at www.melobase.com.",
                    true);
            }
            return false;
        }

        // Perform the migration if necessary
        if (version < 1) {
            std::cout << "Performing database migration to version 1..." << std::endl;

            if (!_db->exec("BEGIN TRANSACTION;\n", false)) return false;
            if (!_db->exec("ALTER TABLE ZMDSEQUENCE ADD COLUMN ZDATAVERSION INTEGER;\n", false)) return false;
            if (!_db->exec("PRAGMA user_version = 1;\n", false)) return false;
            if (!_db->exec("COMMIT;\n", true)) return false;

            std::cout << "Migration to version 1 successful." << std::endl;
        }

        // Perform the migration if necessary
        if (version < 2) {
            std::cout << "Performing database migration to version 2..." << std::endl;

            if (!_db->exec("BEGIN TRANSACTION;\n", false)) return false;
            if (!_db->exec("ALTER TABLE ZMDSEQUENCESFOLDER ADD COLUMN ZDATE TIMESTAMP;\n", false)) return false;
            if (!_db->exec("ALTER TABLE ZMDSEQUENCESFOLDER ADD COLUMN ZRATING FLOAT;\n", false)) return false;
            if (!_db->exec("ALTER TABLE ZMDSEQUENCESFOLDER ADD COLUMN ZVERSION INTEGER;\n", false)) return false;

            // Add standard folders
            if (!addStandardFolders()) return false;

            // Move all the sequences to the Sequences folder
            if (!_db->exec("UPDATE `ZMDSEQUENCE` SET ZFOLDER=2;\n", false)) return false;

            // Update database version
            if (!_db->exec("PRAGMA user_version = 2;\n", false)) return false;
            if (!_db->exec("COMMIT;\n", true)) return false;

            std::cout << "Migration to version 2 successful." << std::endl;
        }

        // Perform the migration if necessary
        if (version < 3) {
            std::cout << "Performing database migration to version 3..." << std::endl;

            if (!_db->exec("BEGIN TRANSACTION;\n", false)) return false;

            // Update database version
            if (!_db->exec("PRAGMA user_version = 3;\n", false)) return false;
            if (!_db->exec("COMMIT;\n", true)) return false;

            std::cout << "Migration to version 3 successful." << std::endl;
        }

        // Perform the migration if necessary
        if (version < 4) {
            std::cout << "Performing database migration to version 4..." << std::endl;

            if (!_db->exec("BEGIN TRANSACTION;\n", false)) return false;

            if (!_db->exec("ALTER TABLE ZMDSEQUENCE ADD COLUMN ZANNOTATIONS BLOB;\n", false)) return false;

            // Update database version
            if (!_db->exec("PRAGMA user_version = 4;\n", false)) return false;
            if (!_db->exec("COMMIT;\n", true)) return false;

            std::cout << "Migration to version 4 successful." << std::endl;
        }
    }

    // Validate and fix if necessary the standard folders due to non-atomic SQL operation in previous versions
    validateAndFixStandardFolders();

    // Read the primary keys table
    std::string s;
    _db->clearResults();
    if (!_db->exec("BEGIN TRANSACTION;\n", false)) return false;
    if (!_db->exec("SELECT * FROM Z_PRIMARYKEY;\n", true)) return false;
    if (!_db->exec("COMMIT;\n", true)) return false;

    UInt64* maxID = nullptr;

    for (auto row : _db->rows()) {
        for (auto column : row) {
            if (column.first == std::string("Z_NAME")) {
                if (column.second == std::string("MDSequence")) {
                    maxID = &_maxSequenceID;
                } else if (column.second == std::string("MDSequenceData")) {
                    maxID = &_maxSequenceDataID;
                } else if (column.second == std::string("MDSequencesFolder")) {
                    maxID = &_maxSequencesFolderID;
                }
            } else if (column.first == std::string("Z_MAX")) {
                assert(maxID);
                *maxID = std::stoul(column.second);
                maxID = nullptr;
            }
        }
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::SequencesDB::addSequence(std::shared_ptr<Sequence> sequence, bool isDelegateNotified,
                                            bool isSpecificID, UInt64 specificID, UInt64 specificDataID,
                                            const std::vector<UInt64>& specificAnnotationIDs) {
    _dbMutex.lock();

    if (_undoManager) _undoManager->pushFn([=]() { removeSequence(sequence); });

    //
    // Add to the database
    //

    UInt64 sequenceID;
    UInt64 sequenceDataID;

    std::vector<UInt64> sequenceAnnotationIDs;

    if (isSpecificID) {
        sequenceID = specificID;
        sequenceDataID = specificDataID;
    } else {
        sequenceID = _maxSequenceID + 1;
        sequenceDataID = _maxSequenceDataID + 1;
    }

    _db->clearResults();
    if (!_db->exec("BEGIN TRANSACTION;\n", false)) {
        _dbMutex.unlock();
        return false;
    }

    std::vector<char> plist = getSequenceDataBlob(sequence, true);

    std::string s =
        std::string("INSERT INTO `ZMDSEQUENCEDATA` VALUES (" + std::to_string(sequenceDataID) + ",2,2,0," +
                    std::to_string(sequenceID) + ",'" + std::to_string(sequence->data.tickPeriod) + "',?);\n");
    if (!_db->writeBlob(s.c_str(), plist.data(), plist.size(), true)) {
        _dbMutex.unlock();
        return false;
    }

    plist = getSequenceAnnotationsBlob(sequence.get());

    s = std::string("INSERT INTO `ZMDSEQUENCE` VALUES(" + std::to_string(sequenceID) + ",1,5," +
                    std::to_string(sequence->playCount) + "," + std::to_string(sequence->version) + "," +
                    std::to_string(sequenceDataID) + "," +
                    (sequence->folder ? std::to_string(sequence->folder->id) : std::string("''")) + std::string(",") +
                    std::to_string(sequence->date) + "," + std::to_string(sequence->rating) + ",'','" +
                    sanitizedSQLString(sequence->name) + "'," + std::to_string(sequence->dataVersion) + ",?)\n");
    if (!_db->writeBlob(s.c_str(), plist.size() > 0 ? plist.data() : nullptr, plist.size(), true)) {
        _dbMutex.unlock();
        return false;
    }
    s = std::string("UPDATE `Z_PRIMARYKEY` SET Z_MAX=" + std::to_string(sequenceID) + " WHERE Z_NAME='MDSequence';\n");
    if (!_db->exec(s.c_str(), true)) {
        _dbMutex.unlock();
        return false;
    }
    s = std::string("UPDATE `Z_PRIMARYKEY` SET Z_MAX=" + std::to_string(sequenceDataID) +
                    " WHERE Z_NAME='MDSequenceData';\n");
    if (!_db->exec(s.c_str(), true)) {
        _dbMutex.unlock();
        return false;
    }

    if (!_db->exec("COMMIT;\n", true)) {
        _dbMutex.unlock();
        return false;
    }

    if (!isSpecificID) {
        // Everything is fine, so we update our our copy of the max IDs
        _maxSequenceID = sequenceID;
        _maxSequenceDataID = sequenceDataID;

        sequence->id = _maxSequenceID;
        sequence->data.id = _maxSequenceDataID;
    }

    _dbMutex.unlock();

    if (isDelegateNotified && _sequenceAddedFn) {
        _sequenceAddedFn(this);
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::SequencesDB::addFolder(std::shared_ptr<SequencesFolder> folder, bool isDelegateNotified,
                                          bool isSpecificID, unsigned long specificID, bool isInsideTransaction) {
    _dbMutex.lock();

    if (_undoManager) _undoManager->pushFn([=]() { removeFolder(folder); });

    //
    // Add to the database
    //

    unsigned long maxSequencesFolderID = _maxSequencesFolderID + 1;

    if (isSpecificID) {
        maxSequencesFolderID = specificID;
    }

    _db->clearResults();

    if (!isInsideTransaction) {
        if (!_db->exec("BEGIN TRANSACTION;\n", false)) {
            _dbMutex.unlock();
            return false;
        }
    }

    std::string s;
    if (folder) {
        s = std::string("INSERT INTO `ZMDSEQUENCESFOLDER` VALUES(" + std::to_string(maxSequencesFolderID) + ",3,6," +
                        std::to_string(folder->parentID) + ",''," + "'" + sanitizedSQLString(folder->name) +
                        std::string("',") + std::to_string(folder->date) + "," + std::to_string(folder->rating) + "," +
                        std::to_string(folder->version) + ")\n");
        if (!_db->exec(s.c_str(), true)) {
            _dbMutex.unlock();
            return false;
        }
    }
    s = std::string("UPDATE `Z_PRIMARYKEY` SET Z_MAX=" + std::to_string(maxSequencesFolderID) +
                    " WHERE Z_NAME='MDSequencesFolder';\n");
    if (!_db->exec(s.c_str(), true)) {
        _dbMutex.unlock();
        return false;
    }

    if (!isInsideTransaction) {
        if (!_db->exec("COMMIT;\n", true)) {
            _dbMutex.unlock();
            return false;
        }
    }

    if (!isSpecificID && folder) {
        // Everything is fine, so we update our our copy of the max IDs
        _maxSequencesFolderID = maxSequencesFolderID;

        folder->id = _maxSequencesFolderID;
    }

    _dbMutex.unlock();

    if (isDelegateNotified && _folderAddedFn) {
        _folderAddedFn(this, folder);
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::SequencesDB::removeSequence(std::shared_ptr<Sequence> sequence, bool isDelegateNotified) {
    if (isDelegateNotified && _willRemoveSequenceFn) {
        _willRemoveSequenceFn(this, sequence);
    }

    _dbMutex.lock();

    if (_undoManager)
        _undoManager->pushFn([=]() { addSequence(sequence, true, true, sequence->id, sequence->data.id); });

    std::string s;
    _db->clearResults();
    if (!_db->exec("BEGIN TRANSACTION;\n", false)) {
        _dbMutex.unlock();
        return false;
    }

    s = std::string("DELETE FROM `ZMDSEQUENCE` WHERE Z_PK=") + std::to_string(sequence->id) + std::string(";\n");
    s += std::string("DELETE FROM `ZMDSEQUENCEDATA` WHERE Z_PK=") + std::to_string(sequence->data.id) +
         std::string(";\n");

    if (!_db->exec(s.c_str(), true)) {
        _dbMutex.unlock();
        return false;
    }
    if (!_db->exec("COMMIT;\n", true)) {
        _dbMutex.unlock();
        return false;
    }
    _dbMutex.unlock();

    if (isDelegateNotified && _sequenceRemovedFn) {
        _sequenceRemovedFn(this);
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::SequencesDB::removeFolder(std::shared_ptr<SequencesFolder> folder, bool isDelegateNotified) {
    if (!folder) return false;

    if (isDelegateNotified && _willRemoveFolderFn) {
        _willRemoveFolderFn(this, folder);
    }

    _dbMutex.lock();

    if (_undoManager) _undoManager->pushFn([=]() { addFolder(folder, true, true, folder->id); });

    std::string s;
    _db->clearResults();
    if (!_db->exec("BEGIN TRANSACTION;\n", false)) {
        _dbMutex.unlock();
        return false;
    }

    s = std::string("UPDATE `ZMDSEQUENCE` SET ZFOLDER='' WHERE ZFOLDER=") + std::to_string(folder->id) +
        std::string(";\n");

    if (!_db->exec(s.c_str(), true)) {
        _dbMutex.unlock();
        return false;
    }

    s = std::string("DELETE FROM `ZMDSEQUENCESFOLDER` WHERE Z_PK=") + std::to_string(folder->id) + std::string(";\n");

    if (!_db->exec(s.c_str(), true)) {
        _dbMutex.unlock();
        return false;
    }

    if (!_db->exec("COMMIT;\n", true)) {
        _dbMutex.unlock();
        return false;
    }
    _dbMutex.unlock();

    if (isDelegateNotified && _folderRemovedFn) {
        _folderRemovedFn(this);
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequencesDB::invalidateSequencesCache() {
    _dbMutex.lock();
    _isSequencesCacheValid = false;
    _dbMutex.unlock();
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequencesDB::invalidateFoldersCache() {
    _dbMutex.lock();
    _isFoldersCacheValid = false;
    _dbMutex.unlock();
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned long MelobaseCore::SequencesDB::getNbSequences(sequencesFilterEnum filter, std::string nameSearch,
                                                        std::shared_ptr<SequencesFolder> folder,
                                                        bool isIncludingSubfolders) {
    _dbMutex.lock();
    std::string s;
    _db->clearResults();
    if (!_db->exec("BEGIN TRANSACTION;\n", false)) {
        _dbMutex.unlock();
        return false;
    }
    if (folder && isIncludingSubfolders) {
        s = std::string("WITH RECURSIVE inside_folder(n) AS ( VALUES(" + std::to_string(folder->id) +
                        ") UNION SELECT Z_PK FROM ZMDSEQUENCESFOLDER, inside_folder WHERE ZPARENT=inside_folder.n) "
                        "SELECT COUNT(*) FROM `ZMDSEQUENCE` " +
                        filterString(filter, nameSearch, nullptr, true) + ";\n");
    } else {
        s = std::string("SELECT COUNT(*) FROM `ZMDSEQUENCE` ") + filterString(filter, nameSearch, folder, false) +
            ";\n";
    }
    if (!_db->exec(s.c_str(), true)) {
        _dbMutex.unlock();
        return false;
    }
    if (!_db->exec("COMMIT;\n", true)) {
        _dbMutex.unlock();
        return false;
    }

    unsigned long ret = std::stol(_db->rows()[0][0].second);
    _dbMutex.unlock();
    return ret;
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned long MelobaseCore::SequencesDB::getNbFolders(std::shared_ptr<SequencesFolder> parentFolder) {
    _dbMutex.lock();
    std::string s;

    std::string filter = parentFolder ? " WHERE ZPARENT=" + std::to_string(parentFolder->id) + " AND Z_PK<>0" : "";

    _db->clearResults();
    if (!_db->exec("BEGIN TRANSACTION;\n", false)) {
        _dbMutex.unlock();
        return false;
    }
    s = std::string("SELECT COUNT(*) FROM `ZMDSEQUENCESFOLDER`" + filter + ";\n");
    if (!_db->exec(s.c_str(), true)) {
        _dbMutex.unlock();
        return false;
    }
    if (!_db->exec("COMMIT;\n", true)) {
        _dbMutex.unlock();
        return false;
    }

    unsigned long ret = std::stol(_db->rows()[0][0].second);
    _dbMutex.unlock();
    return ret;
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequencesDB::setSequence(std::shared_ptr<Sequence> sequence,
                                            std::vector<std::pair<std::string, std::string>> columns) {
    for (auto column : columns) {
        if (column.first == std::string("Z_PK")) {
            sequence->id = std::stol(column.second);
        } else if (column.first == std::string("ZDATE")) {
            sequence->date = std::stod(column.second);
        } else if (column.first == std::string("ZVERSION")) {
            sequence->version = std::stod(column.second);
        } else if (column.first == std::string("ZNAME")) {
            sequence->name = column.second;
        } else if (column.first == std::string("ZRATING")) {
            sequence->rating = std::stof(column.second);
        } else if (column.first == std::string("ZPLAYCOUNT")) {
            sequence->playCount = std::stoi(column.second);
        } else if (column.first == std::string("ZFOLDER")) {
            if (!column.second.empty()) {
                sequence->folder = getFolderWithIDInternal(stoull(column.second));
            } else {
                sequence->folder = nullptr;
            }
        } else if (column.first == std::string("ZDATAVERSION")) {
            if (column.second != "") {
                sequence->dataVersion = std::stod(column.second);
            } else {
                sequence->dataVersion = 0;
            }
        }
    }

    if (sequence->dataVersion == 0) sequence->dataVersion = sequence->version;
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequencesDB::setSequenceData(SequenceData* sequenceData,
                                                std::vector<std::pair<std::string, std::string>> columns) {
    for (auto column : columns) {
        if (column.first == std::string("Z_PK")) {
            sequenceData->id = std::stol(column.second);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequencesDB::setFolder(std::shared_ptr<SequencesFolder> folder,
                                          std::vector<std::pair<std::string, std::string>> columns) {
    for (auto column : columns) {
        if (column.first == std::string("Z_PK")) {
            folder->id = std::stol(column.second);
        } else if (column.first == std::string("ZDATE")) {
            if (column.second != "") {
                folder->date = std::stod(column.second);
            } else {
                folder->date = 0;
            }
        } else if (column.first == std::string("ZNAME")) {
            folder->name = column.second;
        } else if (column.first == std::string("ZRATING")) {
            if (column.second != "") {
                folder->rating = std::stof(column.second);
            } else {
                folder->rating = 0;
            }
        } else if (column.first == std::string("ZVERSION")) {
            if (column.second != "") {
                folder->version = std::stod(column.second);
            } else {
                folder->version = 0;
            }
        } else if (column.first == std::string("ZPARENT")) {
            if (column.second != "") {
                folder->parentID = std::stol(column.second);
            } else {
                folder->parentID = 0;
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::SequencesDB::readSequenceAnnotations(Sequence* sequence) {
    auto s = std::string("SELECT ZANNOTATIONS FROM `ZMDSEQUENCE` WHERE Z_PK=") + std::to_string(sequence->id) +
             std::string(";\n");

    char* blob;
    size_t size;

    if (_db->readBlob(s.c_str(), &blob, &size)) {
        Any message;

        if (!setSequenceAnnotationsFromBlob(sequence, blob, size)) {
            free(blob);
            return false;
        }
    }

    free(blob);
    return true;
}
// ---------------------------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<Sequence>> MelobaseCore::SequencesDB::getSequences() {
    _dbMutex.lock();
    std::vector<std::shared_ptr<Sequence>> sequences;

    _db->clearResults();
    if (!_db->exec("BEGIN TRANSACTION;\n", false)) {
        _dbMutex.unlock();
        return {};
    }
    if (!_db->exec("SELECT Z_PK,ZDATE,ZVERSION,ZNAME,ZRATING,ZPLAYCOUNT,ZFOLDER,ZDATAVERSION FROM `ZMDSEQUENCE`;\n",
                   true)) {
        _dbMutex.unlock();
        return {};
    }
    if (!_db->exec("COMMIT;\n", true)) {
        _dbMutex.unlock();
        return {};
    }

    for (auto row : _db->rows()) {
        std::shared_ptr<Sequence> sequence = std::shared_ptr<Sequence>(new Sequence());
        setSequence(sequence, row);
        if (!readSequenceAnnotations(sequence.get())) {
            _dbMutex.unlock();
            return {};
        }

        sequences.push_back(sequence);
    }

    _dbMutex.unlock();
    return sequences;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<SequencesFolder>> MelobaseCore::SequencesDB::getFolders(
    std::shared_ptr<SequencesFolder> parentFolder) {
    _dbMutex.lock();
    std::vector<std::shared_ptr<SequencesFolder>> folders;

    std::string s;
    std::string filter = parentFolder ? " WHERE ZPARENT=" + std::to_string(parentFolder->id) + " AND Z_PK<>0" : "";

    _db->clearResults();
    if (!_db->exec("BEGIN TRANSACTION;\n", false)) {
        _dbMutex.unlock();
        return folders;
    }
    s = std::string("SELECT * FROM `ZMDSEQUENCESFOLDER`" + filter + ";\n");
    if (!_db->exec(s.c_str(), true)) {
        _dbMutex.unlock();
        return folders;
    }
    if (!_db->exec("COMMIT;\n", true)) {
        _dbMutex.unlock();
        return folders;
    }

    for (auto row : _db->rows()) {
        std::shared_ptr<SequencesFolder> folder = std::shared_ptr<SequencesFolder>(new SequencesFolder());
        setFolder(folder, row);
        folders.push_back(folder);
    }
    _dbMutex.unlock();
    return folders;
}

// ---------------------------------------------------------------------------------------------------------------------
std::string MelobaseCore::SequencesDB::filterString(sequencesFilterEnum filter, std::string nameSearch,
                                                    std::shared_ptr<SequencesFolder> folder,
                                                    bool isIncludingSubfolders) {
    std::string filterString;
    switch (filter) {
        case None:
            break;
        case Trash:
            filterString = std::string("WHERE ZRATING < 0");
            break;
        case All:
            filterString = std::string("");
            break;
        case New:
            filterString = std::string("WHERE ZPLAYCOUNT = 0");
            break;
        case Annotated:
            filterString = std::string("WHERE ZANNOTATIONS NOT NULL");
            break;
        case Filter1:
            filterString = std::string("WHERE ZRATING >= 0.2");
            break;
        case Filter2:
            filterString = std::string("WHERE ZRATING >= 0.4");
            break;
        case Filter3:
            filterString = std::string("WHERE ZRATING >= 0.6");
            break;
        case Filter4:
            filterString = std::string("WHERE ZRATING >= 0.8");
            break;
        case Filter5:
            filterString = std::string("WHERE ZRATING >= 1.0");
            break;
    }

    if (nameSearch != "") {
        if (filterString.empty()) {
            filterString += "WHERE ";
        } else {
            filterString += " AND ";
        }
        filterString += "ZNAME LIKE '%%" + nameSearch + "%%'";
    }

    if (folder && !isIncludingSubfolders) {
        if (filterString.empty()) {
            filterString += "WHERE ";
        } else {
            filterString += " AND ";
        }
        filterString += "ZFOLDER = " + std::to_string(folder->id);
    }

    if (isIncludingSubfolders) {
        if (filterString.empty()) {
            filterString += "WHERE ";
        } else {
            filterString += " AND ";
        }
        filterString += "ZMDSEQUENCE.ZFOLDER IN inside_folder";
    }

    return filterString;
}

// ---------------------------------------------------------------------------------------------------------------------
std::string MelobaseCore::SequencesDB::orderString(sequencesOrderFieldEnum orderField,
                                                   orderDirectionEnum orderDirection) {
    std::string orderString = "ORDER BY ";

    if (orderField == Date) {
        orderString += "ZDATE ";
    } else if (orderField == Rating) {
        orderString += "ZRATING ";
    } else {
        orderString += "ZNAME COLLATE NOCASE ";
    }

    if (orderDirection == Ascending) {
        orderString += "ASC";
    } else {
        orderString += "DESC";
    }

    return orderString;
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<Sequence> MelobaseCore::SequencesDB::getSequence(
    unsigned long index, sequencesFilterEnum filter, std::string nameSearch, std::shared_ptr<SequencesFolder> folder,
    bool isIncludingSubfolders, sequencesOrderFieldEnum orderField, orderDirectionEnum orderDirection) {
    _dbMutex.lock();

    if (!_isSequencesCacheValid) {
        _cachedSequences.clear();

        std::string s;

        _db->clearResults();
        if (!_db->exec("BEGIN TRANSACTION;\n", false)) {
            _dbMutex.unlock();
            return nullptr;
        }

        if (folder && isIncludingSubfolders) {
            s = std::string(
                "WITH RECURSIVE inside_folder(n) AS ( VALUES(" + std::to_string(folder->id) +
                ") UNION SELECT Z_PK FROM ZMDSEQUENCESFOLDER, inside_folder WHERE ZPARENT=inside_folder.n) "
                "SELECT Z_PK,ZDATE,ZVERSION,ZNAME,ZRATING,ZPLAYCOUNT,ZFOLDER,ZDATAVERSION FROM `ZMDSEQUENCE` " +
                filterString(filter, nameSearch, nullptr, true) + " " + orderString(orderField, orderDirection) +
                ";\n");
        } else {
            s = std::string(
                "SELECT Z_PK,ZDATE,ZVERSION,ZNAME,ZRATING,ZPLAYCOUNT,ZFOLDER,ZDATAVERSION FROM `ZMDSEQUENCE` " +
                filterString(filter, nameSearch, folder, false) + " " + orderString(orderField, orderDirection) +
                ";\n");
        }

        if (!_db->exec(s.c_str(), true)) {
            _dbMutex.unlock();
            return nullptr;
        }
        if (!_db->exec("COMMIT;\n", true)) {
            _dbMutex.unlock();
            return nullptr;
        }

        for (auto row : _db->rows()) {
            std::shared_ptr<Sequence> sequence = std::shared_ptr<Sequence>(new Sequence());
            std::vector<std::pair<std::string, std::string>> columns = row;
            setSequence(sequence, columns);
            if (!readSequenceAnnotations(sequence.get())) {
                _dbMutex.unlock();
                return nullptr;
            }

            _cachedSequences.push_back(sequence);
        }

        _isSequencesCacheValid = true;
    }

    assert(index < _cachedSequences.size());

    std::shared_ptr<Sequence> retSequence = _cachedSequences[index];

    _dbMutex.unlock();

    return retSequence;
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<SequencesFolder> MelobaseCore::SequencesDB::getFolder(unsigned long index,
                                                                      std::shared_ptr<SequencesFolder> parentFolder) {
    _dbMutex.lock();

    if (!_isFoldersCacheValid) {
        _cachedFolders.clear();

        std::string s;

        std::string filter = parentFolder ? " WHERE ZPARENT=" + std::to_string(parentFolder->id) + " AND Z_PK<>0" : "";

        _db->clearResults();
        if (!_db->exec("BEGIN TRANSACTION;\n", false)) {
            _dbMutex.unlock();
            return nullptr;
        }
        s = std::string("SELECT * FROM `ZMDSEQUENCESFOLDER`" + filter + ";\n");
        if (!_db->exec(s.c_str(), true)) {
            _dbMutex.unlock();
            return nullptr;
        }
        if (!_db->exec("COMMIT;\n", true)) {
            _dbMutex.unlock();
            return nullptr;
        }

        for (auto row : _db->rows()) {
            std::shared_ptr<SequencesFolder> folder = std::shared_ptr<SequencesFolder>(new SequencesFolder());
            std::vector<std::pair<std::string, std::string>> columns = row;
            setFolder(folder, columns);
            _cachedFolders.push_back(folder);
        }

        _isFoldersCacheValid = true;
    }

    if (index >= _cachedFolders.size()) {
        _dbMutex.unlock();
        return nullptr;
    }

    std::shared_ptr<SequencesFolder> retFolder = _cachedFolders[index];

    _dbMutex.unlock();

    return retFolder;
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<Sequence> MelobaseCore::SequencesDB::getSequenceWithID(UInt64 id) {
    _dbMutex.lock();

    std::string s;

    _db->clearResults();
    if (!_db->exec("BEGIN TRANSACTION;\n", false)) {
        _dbMutex.unlock();
        return nullptr;
    }

    s = std::string(
            "SELECT Z_PK,ZDATE,ZVERSION,ZNAME,ZRATING,ZPLAYCOUNT,ZFOLDER,ZDATAVERSION FROM `ZMDSEQUENCE` WHERE Z_PK=") +
        std::to_string(id) + std::string(";\n");
    if (!_db->exec(s.c_str(), true)) {
        _dbMutex.unlock();
        return nullptr;
    }

    if (!_db->exec("COMMIT;\n", true)) {
        _dbMutex.unlock();
        return nullptr;
    }

    if (_db->rows().size() == 0) {
        // Sequence not found
        _dbMutex.unlock();
        return nullptr;
    }

    std::shared_ptr<Sequence> sequence = std::shared_ptr<Sequence>(new Sequence());
    std::vector<std::pair<std::string, std::string>> columns = _db->rows().at(0);

    setSequence(sequence, columns);
    if (!readSequenceAnnotations(sequence.get())) {
        _dbMutex.unlock();
        return nullptr;
    }

    _dbMutex.unlock();
    return sequence;
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<SequencesFolder> MelobaseCore::SequencesDB::getFolderWithIDInternal(UInt64 id) {
    std::string s;

    _db->clearResults();
    if (!_db->exec("BEGIN TRANSACTION;\n", false)) {
        return nullptr;
    }

    s = std::string("SELECT * FROM `ZMDSEQUENCESFOLDER` WHERE Z_PK=") + std::to_string(id) + std::string(";\n");
    if (!_db->exec(s.c_str(), true)) {
        return nullptr;
    }

    if (!_db->exec("COMMIT;\n", true)) {
        return nullptr;
    }

    if (_db->rows().size() == 0) return nullptr;

    std::shared_ptr<SequencesFolder> folder = std::shared_ptr<SequencesFolder>(new SequencesFolder());
    std::vector<std::pair<std::string, std::string>> columns = _db->rows().at(0);

    setFolder(folder, columns);

    return folder;
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<SequencesFolder> MelobaseCore::SequencesDB::getFolderWithID(UInt64 id) {
    std::shared_ptr<SequencesFolder> folder = nullptr;
    _dbMutex.lock();
    folder = getFolderWithIDInternal(id);
    _dbMutex.unlock();
    return folder;
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::SequencesDB::readSequenceData(std::shared_ptr<Sequence> sequence) {
    _dbMutex.lock();

    std::string s;

    _db->clearResults();
    if (!_db->exec("BEGIN TRANSACTION;\n", false)) {
        _dbMutex.unlock();
        return false;
    }
    s = std::string("SELECT ZDATA FROM `ZMDSEQUENCE` WHERE Z_PK=") + std::to_string(sequence->id) + std::string(";\n");
    if (!_db->exec(s.c_str(), true)) {
        _dbMutex.unlock();
        return false;
    }
    if (!_db->exec("COMMIT;\n", true)) {
        _dbMutex.unlock();
        return false;
    }

    std::vector<std::pair<std::string, std::string>> columns = _db->rows().at(0);

    long dataID = std::stol(columns.at(0).second);
    sequence->data.id = dataID;

    _db->clearResults();
    s = std::string("SELECT ZTICKPERIOD FROM `ZMDSEQUENCEDATA` WHERE Z_PK=") + std::to_string(dataID) +
        std::string(";\n");
    if (!_db->exec(s.c_str(), true)) {
        _dbMutex.unlock();
        return false;
    }
    columns = _db->rows().at(0);
    Float64 tickPeriod = std::stold(columns.at(0).second);
    if (tickPeriod == 0) {
        std::cout << "Warning: tickPeriod is zero, using 0.001" << std::endl;
        tickPeriod = 0.001;
    }
    sequence->data.tickPeriod = tickPeriod;

    s = std::string("SELECT ZEVENTS FROM `ZMDSEQUENCEDATA` WHERE Z_PK=") + std::to_string(dataID) + std::string(";\n");

    char* blob;
    size_t size;

    if (_db->readBlob(s.c_str(), &blob, &size)) {
        Any message;

        if (!setSequenceDataFromBlob(sequence, blob, size)) {
            free(blob);
            _dbMutex.unlock();
            return false;
        }

        free(blob);
    } else {
        _dbMutex.unlock();
        return false;
    }
    _dbMutex.unlock();
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::SequencesDB::updateSequences(std::vector<std::shared_ptr<Sequence>> sequences,
                                                bool isDelegateNotified, bool isVersionUpdated, bool isDataUpdated,
                                                bool isDataVersionUpdated) {
    std::vector<std::shared_ptr<Sequence>> oldSequences;

    for (auto sequence : sequences) oldSequences.push_back(getSequenceWithID(sequence->id));

    _dbMutex.lock();

    if (_undoManager) _undoManager->pushFn([=]() { updateSequences(oldSequences, true, isVersionUpdated); });

    for (auto sequence : sequences) {
        if (isVersionUpdated) sequence->version = MDStudio::getTimestamp();

        if (isDataVersionUpdated) sequence->dataVersion = MDStudio::getTimestamp();

        std::string s;

        _db->clearResults();
        if (!_db->exec("BEGIN TRANSACTION;\n", false)) {
            _dbMutex.unlock();
            return false;
        }
        s = std::string("UPDATE `ZMDSEQUENCE` SET ZNAME='" + sanitizedSQLString(sequence->name) + "',ZRATING=" +
                        std::to_string(sequence->rating) + ",ZPLAYCOUNT=" + std::to_string(sequence->playCount) +
                        ",ZVERSION=" + std::to_string(sequence->version) +
                        ",ZDATAVERSION=" + std::to_string(sequence->dataVersion) + ",ZFOLDER=" +
                        (sequence->folder ? std::to_string(sequence->folder->id) : "''") + " WHERE Z_PK=") +
            std::to_string(sequence->id) + std::string(";\n");
        if (!_db->exec(s.c_str(), true)) {
            _dbMutex.unlock();
            return false;
        }

        // Update annotations
        auto annotationsPlist = getSequenceAnnotationsBlob(sequence.get());
        s = std::string("UPDATE `ZMDSEQUENCE` SET ZANNOTATIONS=? WHERE Z_PK=") + std::to_string(sequence->id) +
            std::string(";\n");
        if (!_db->writeBlob(s.c_str(), annotationsPlist.size() > 0 ? annotationsPlist.data() : nullptr,
                            annotationsPlist.size(), true)) {
            _dbMutex.unlock();
            return false;
        }

        if (isDataUpdated) {
            std::vector<char> plist = getSequenceDataBlob(sequence, true);

            //
            // Update the database
            //

            std::string s = std::string("UPDATE `ZMDSEQUENCEDATA` SET ZEVENTS=? WHERE Z_PK=") +
                            std::to_string(sequence->data.id) + std::string(";\n");
            if (!_db->writeBlob(s.c_str(), plist.data(), plist.size(), true)) {
                _dbMutex.unlock();
                return false;
            }
        }

        if (!_db->exec("COMMIT;\n", true)) {
            _dbMutex.unlock();
            return false;
        }
    }  // for each sequence

    _dbMutex.unlock();

    if (isDelegateNotified && _sequenceUpdatedFn) {
        _sequenceUpdatedFn(this);
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::SequencesDB::updateFolder(std::shared_ptr<SequencesFolder> folder, bool isDelegateNotified,
                                             bool isVersionUpdated) {
    std::shared_ptr<SequencesFolder> oldFolder = getFolderWithID(folder->id);

    _dbMutex.lock();

    if (_undoManager) _undoManager->pushFn([=]() { updateFolder(oldFolder); });

    if (isVersionUpdated) folder->version = MDStudio::getTimestamp();

    std::string s;

    _db->clearResults();
    if (!_db->exec("BEGIN TRANSACTION;\n", false)) {
        _dbMutex.unlock();
        return false;
    }
    s = std::string("UPDATE `ZMDSEQUENCESFOLDER` SET ZNAME='" + sanitizedSQLString(folder->name) +
                    "',ZRATING=" + std::to_string(folder->rating) + ",ZVERSION=" + std::to_string(folder->version) +
                    ",ZPARENT=" + std::to_string(folder->parentID) + " WHERE Z_PK=" + std::to_string(folder->id) +
                    std::string(";\n"));
    if (!_db->exec(s.c_str(), true)) {
        _dbMutex.unlock();
        return false;
    }

    if (!_db->exec("COMMIT;\n", true)) {
        _dbMutex.unlock();
        return false;
    }
    _dbMutex.unlock();

    if (isDelegateNotified && _folderUpdatedFn) {
        _folderUpdatedFn(this);
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::SequencesDB::promoteAllSequences(std::shared_ptr<SequencesFolder> folder, bool isIncludingSubfolders,
                                                    bool isDelegateNotified) {
    _dbMutex.lock();

    if (_undoManager) _undoManager->pushFn([=]() { demoteAllSequences(folder, isIncludingSubfolders); });

    std::string folderFilter;
    std::string recursivePrefix;

    if (folder && isIncludingSubfolders) {
        recursivePrefix =
            std::string("WITH RECURSIVE inside_folder(n) AS ( VALUES(" + std::to_string(folder->id) +
                        ") UNION SELECT Z_PK FROM ZMDSEQUENCESFOLDER, inside_folder WHERE ZPARENT=inside_folder.n) ");
    } else {
        folderFilter = std::string("AND ZFOLDER = ") + (folder ? std::to_string(folder->id) : "''");
    }

    std::string s;

    double version = MDStudio::getTimestamp();

    _db->clearResults();
    if (!_db->exec("BEGIN TRANSACTION;\n", false)) {
        _dbMutex.unlock();
        return false;
    }
    s = recursivePrefix + std::string("UPDATE `ZMDSEQUENCE` SET ZRATING=ZRATING+0.2,ZVERSION=" +
                                      std::to_string(version) + " WHERE ZPLAYCOUNT > 0 " + folderFilter + ";\n");
    if (!_db->exec(s.c_str(), true)) {
        _dbMutex.unlock();
        return false;
    }
    if (!_db->exec("COMMIT;\n", true)) {
        _dbMutex.unlock();
        return false;
    }
    _dbMutex.unlock();

    if (isDelegateNotified && _sequenceUpdatedFn) {
        _sequenceUpdatedFn(this);
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::SequencesDB::demoteAllSequences(std::shared_ptr<SequencesFolder> folder, bool isIncludingSubfolders,
                                                   bool isDelegateNotified) {
    _dbMutex.lock();

    if (_undoManager) _undoManager->pushFn([=]() { promoteAllSequences(folder, isIncludingSubfolders); });

    std::string folderFilter;
    std::string recursivePrefix;

    if (folder && isIncludingSubfolders) {
        recursivePrefix =
            std::string("WITH RECURSIVE inside_folder(n) AS ( VALUES(" + std::to_string(folder->id) +
                        ") UNION SELECT Z_PK FROM ZMDSEQUENCESFOLDER, inside_folder WHERE ZPARENT=inside_folder.n) ");
    } else {
        folderFilter = std::string("AND ZFOLDER = ") + (folder ? std::to_string(folder->id) : "''");
    }

    std::string s;

    double version = MDStudio::getTimestamp();

    _db->clearResults();
    if (!_db->exec("BEGIN TRANSACTION;\n", false)) {
        _dbMutex.unlock();
        return false;
    }
    s = recursivePrefix + std::string("UPDATE `ZMDSEQUENCE` SET ZRATING=ZRATING-0.2,ZVERSION=" +
                                      std::to_string(version) + " WHERE ZPLAYCOUNT > 0 " + folderFilter + ";\n");
    if (!_db->exec(s.c_str(), true)) {
        _dbMutex.unlock();
        return false;
    }
    if (!_db->exec("COMMIT;\n", true)) {
        _dbMutex.unlock();
        return false;
    }
    _dbMutex.unlock();

    if (isDelegateNotified && _sequenceUpdatedFn) {
        _sequenceUpdatedFn(this);
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::SequencesDB::setAsPlayedAllSequences(std::shared_ptr<SequencesFolder> folder,
                                                        bool isIncludingSubfolders, bool isDelegateNotified) {
    _dbMutex.lock();

    // For now, this operation cannot be undone so we clear the undo/redo stacks
    if (_undoManager) _undoManager->clear();

    std::string folderFilter;
    std::string recursivePrefix;

    if (folder && isIncludingSubfolders) {
        recursivePrefix =
            std::string("WITH RECURSIVE inside_folder(n) AS ( VALUES(" + std::to_string(folder->id) +
                        ") UNION SELECT Z_PK FROM ZMDSEQUENCESFOLDER, inside_folder WHERE ZPARENT=inside_folder.n) ");
    } else {
        folderFilter = std::string("AND ZFOLDER = ") + (folder ? std::to_string(folder->id) : "''");
    }

    std::string s;

    _db->clearResults();
    if (!_db->exec("BEGIN TRANSACTION;\n", false)) {
        _dbMutex.unlock();
        return false;
    }

    s = recursivePrefix +
        std::string("UPDATE `ZMDSEQUENCE` SET ZPLAYCOUNT=1 WHERE ZPLAYCOUNT <= 0 " + folderFilter + ";\n");
    if (!_db->exec(s.c_str(), true)) {
        _dbMutex.unlock();
        return false;
    }
    if (!_db->exec("COMMIT;\n", true)) {
        _dbMutex.unlock();
        return false;
    }
    _dbMutex.unlock();

    if (isDelegateNotified && _sequenceUpdatedFn) {
        _sequenceUpdatedFn(this);
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::SequencesDB::cleanupSequences(std::shared_ptr<SequencesFolder> folder, bool isIncludingSubfolders,
                                                 bool isDelegateNotified) {
    _dbMutex.lock();

    // For now, this operation cannot be undone so we clear the undo/redo stacks
    if (_undoManager) _undoManager->clear();

    std::string folderFilter;
    std::string recursivePrefix;

    if (folder && isIncludingSubfolders) {
        recursivePrefix =
            std::string("WITH RECURSIVE inside_folder(n) AS ( VALUES(" + std::to_string(folder->id) +
                        ") UNION SELECT Z_PK FROM ZMDSEQUENCESFOLDER, inside_folder WHERE ZPARENT=inside_folder.n) ");
    } else {
        folderFilter = std::string("AND ZFOLDER = ") + (folder ? std::to_string(folder->id) : "''");
    }

    std::string s;

    double version = MDStudio::getTimestamp();

    _db->clearResults();
    if (!_db->exec("BEGIN TRANSACTION;\n", false)) {
        _dbMutex.unlock();
        return false;
    }
    s = recursivePrefix +
        std::string("UPDATE `ZMDSEQUENCE` SET ZFOLDER=" + std::to_string(TRASH_FOLDER_ID) +
                    ",ZVERSION=" + std::to_string(version) + " WHERE ZRATING < 0 " + folderFilter + ";\n");
    if (!_db->exec(s.c_str(), true)) {
        _dbMutex.unlock();
        return false;
    }
    if (!_db->exec("COMMIT;\n", true)) {
        _dbMutex.unlock();
        return false;
    }
    _dbMutex.unlock();

    if (isDelegateNotified && _sequenceUpdatedFn) {
        _sequenceUpdatedFn(this);
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::SequencesDB::emptyTrash(bool isDelegateNotified) {
    _dbMutex.lock();

    // For now, this operation cannot be undone so we clear the undo/redo stacks
    if (_undoManager) _undoManager->clear();

    std::string s;

    _db->clearResults();
    if (!_db->exec("BEGIN TRANSACTION;\n", false)) {
        _dbMutex.unlock();
        return false;
    }

    s = std::string("WITH RECURSIVE inside_folder(n) AS ( VALUES(" + std::to_string(TRASH_FOLDER_ID) +
                    ") UNION SELECT Z_PK FROM ZMDSEQUENCESFOLDER, inside_folder WHERE ZPARENT=inside_folder.n) DELETE "
                    "FROM `ZMDSEQUENCEDATA` WHERE ZMDSEQUENCEDATA.ZSEQUENCE IN (SELECT ZMDSEQUENCE.ZDATA FROM "
                    "`ZMDSEQUENCE` WHERE ZMDSEQUENCE.ZFOLDER IN inside_folder);\n");
    if (!_db->exec(s.c_str(), true)) {
        _dbMutex.unlock();
        return false;
    }

    s = std::string("WITH RECURSIVE inside_folder(n) AS ( VALUES(" + std::to_string(TRASH_FOLDER_ID) +
                    ") UNION SELECT Z_PK FROM ZMDSEQUENCESFOLDER, inside_folder WHERE ZPARENT=inside_folder.n) DELETE "
                    "FROM `ZMDSEQUENCE` WHERE ZMDSEQUENCE.ZFOLDER IN inside_folder;\n");
    if (!_db->exec(s.c_str(), true)) {
        _dbMutex.unlock();
        return false;
    }

    s = std::string("WITH RECURSIVE inside_folder(n) AS ( VALUES(" + std::to_string(TRASH_FOLDER_ID) +
                    ") UNION SELECT Z_PK FROM ZMDSEQUENCESFOLDER, inside_folder WHERE ZPARENT=inside_folder.n) DELETE "
                    "FROM `ZMDSEQUENCESFOLDER` WHERE ZMDSEQUENCESFOLDER.ZPARENT IN inside_folder;\n");
    if (!_db->exec(s.c_str(), true)) {
        _dbMutex.unlock();
        return false;
    }

    if (!_db->exec("COMMIT;\n", true)) {
        _dbMutex.unlock();
        return false;
    }

    _dbMutex.unlock();

    if (isDelegateNotified) {
        if (_folderUpdatedFn) _folderUpdatedFn(this);
        if (_sequenceUpdatedFn) _sequenceUpdatedFn(this);
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
MelobaseCore::SequencesDB::~SequencesDB() {
    _db->close();
    delete _db;
}
