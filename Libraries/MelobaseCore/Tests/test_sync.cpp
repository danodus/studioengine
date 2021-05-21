//
//  test_sync.cpp
//  MelobaseStationTests
//
//  Created by Daniel Cliche on 2021-01-26.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#include <Sync/databasesync.h>
#include <platform.h>
#include <server.h>

#include <iostream>
#include <thread>

#include "sequenceutils.h"

// ---------------------------------------------------------------------------------------------------------------------
bool sync(MelobaseCore::SequencesDB& sequencesDB) {
    std::cout << "Sync start --->\n";
    std::atomic<int> ret(0);
    std::thread t([&sequencesDB, &ret] {
        MelobaseCore::DatabaseSync sync;

        sync.setDidSetProgressFn(
            [](MelobaseCore::DatabaseSync* sender, float progress, const std::string& description,
               const std::vector<int>& vars) { std::cout << description << " (" << progress * 100.0f << "%)\n"; });

        int error = 0;
        if (!sync.sync("http://localhost:50000", &sequencesDB, &error)) {
            ret = -1;
            return;
        }
        ret = 1;
    });

    while (ret == 0) MDStudio::Platform::sharedInstance()->process();

    t.join();

    MDStudio::Platform::sharedInstance()->process();

    std::cout << "<--- Sync end\n";

    if (ret != 1) return false;

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool testSync() {
    MelobaseCore::SequencesDB sequencesDB1("/tmp/test_sync1.sqlite");
    if (!sequencesDB1.open(true)) return false;

    MelobaseCore::SequencesDB sequencesDB2("/tmp/test_sync2.sqlite");
    if (!sequencesDB2.open(true)) return false;

    MelobaseCore::Server server(&sequencesDB1);
    server.start(50000);

    double date = 633974578.60000002;
    const double dateDelta = 30.0;
    // Note: A date delta less than 30 seconds will cause the sequence sync test to fail due to the float cast hack.

    //
    // Test download folder
    //

    auto folder1 = std::make_shared<MelobaseCore::SequencesFolder>();
    folder1->parentID = SEQUENCES_FOLDER_ID;
    folder1->name = "Test & 1";
    folder1->date = date;
    folder1->version = folder1->date;
    date += dateDelta;

    sequencesDB1.addFolder(folder1);

    auto folder1_1 = std::make_shared<MelobaseCore::SequencesFolder>();
    folder1_1->parentID = folder1->id;
    folder1_1->name = "Test & 1_1";
    folder1_1->date = date;
    folder1_1->version = folder1_1->date;
    date += dateDelta;

    sequencesDB1.addFolder(folder1_1);

    if (!sync(sequencesDB2)) {
        std::cout << "Sync failed\n";
        return false;
    }

    if (!compareDatabases(sequencesDB1, sequencesDB2, true)) {
        std::cout << "Database mismatch detected\n";
        return false;
    }

    //
    // Test upload folders
    //

    auto folder2 = std::make_shared<MelobaseCore::SequencesFolder>();
    folder2->parentID = SEQUENCES_FOLDER_ID;
    folder2->name = "Test & 2";
    folder2->date = date;
    folder2->version = folder2->date;
    date += dateDelta;

    sequencesDB2.addFolder(folder2);

    auto folder2_1 = std::make_shared<MelobaseCore::SequencesFolder>();
    folder2_1->parentID = folder2->id;
    folder2_1->name = "Test & 2_1";
    folder2_1->date = date;
    folder2_1->version = folder2_1->date;
    date += dateDelta;

    sequencesDB2.addFolder(folder2_1);

    if (!sync(sequencesDB2)) {
        std::cout << "Sync failed\n";
        return false;
    }

    if (!compareDatabases(sequencesDB1, sequencesDB2, true)) {
        std::cout << "Database mismatch detected\n";
        return false;
    }

    // Test update folders
    auto folders1 = sequencesDB1.getFolders(sequencesDB1.getFolderWithID(SEQUENCES_FOLDER_ID));
    folders1.at(0)->name = "Test & 1_Modified";
    folders1.at(0)->version = date;
    date += dateDelta;
    sequencesDB1.updateFolder(folders1.at(0));

    auto folders2 = sequencesDB2.getFolders(sequencesDB2.getFolderWithID(SEQUENCES_FOLDER_ID));
    auto folders2_1 = sequencesDB2.getFolders(folders2.at(0));
    folders2_1.at(0)->name = "Test & 1_1_Modified";
    folders2_1.at(0)->version = date;
    date += dateDelta;
    sequencesDB2.updateFolder(folders2_1.at(0));

    if (!sync(sequencesDB2)) {
        std::cout << "Sync failed\n";
        return false;
    }

    if (!compareDatabases(sequencesDB1, sequencesDB2, true)) {
        std::cout << "Database mismatch detected\n";
        return false;
    }

    //
    // Test download & upload sequences
    //

    auto sequence1 = std::make_shared<MelobaseCore::Sequence>();
    sequence1->name = "Test & 1_0";
    sequence1->folder = sequencesDB1.getFolderWithID(SEQUENCES_FOLDER_ID);
    sequence1->date = date;
    sequence1->version = sequence1->date;
    sequence1->dataVersion = sequence1->date;
    date += dateDelta;
    setEvents(sequence1.get(), 10);
    setAnnotations(sequence1.get(), 10);

    sequencesDB1.addSequence(sequence1);

    auto sequence1_1 = std::make_shared<MelobaseCore::Sequence>();
    sequence1_1->name = "Test & 1_1";
    sequence1_1->folder = sequencesDB1.getFolderWithID(SEQUENCES_FOLDER_ID);
    sequence1_1->date = date;
    sequence1_1->version = sequence1_1->date;
    sequence1_1->dataVersion = sequence1_1->date;
    date += dateDelta;
    setEvents(sequence1_1.get(), 50);
    setAnnotations(sequence1_1.get(), 20);

    sequencesDB1.addSequence(sequence1_1);

    auto sequence2 = std::make_shared<MelobaseCore::Sequence>();
    sequence2->name = "Test & 2_0";
    sequence2->folder = sequencesDB2.getFolderWithID(SEQUENCES_FOLDER_ID);
    sequence2->date = date;
    sequence2->version = sequence2->date;
    sequence2->dataVersion = sequence2->date;
    date += dateDelta;
    setEvents(sequence2.get(), 120000);
    setAnnotations(sequence2.get(), 30);

    sequencesDB2.addSequence(sequence2);

    auto sequence2_1 = std::make_shared<MelobaseCore::Sequence>();
    sequence2_1->name = "Test & 2_1";
    sequence2_1->folder = sequencesDB2.getFolderWithID(SEQUENCES_FOLDER_ID);
    sequence2_1->date = date;
    sequence2_1->version = sequence2_1->date;
    sequence2_1->dataVersion = sequence2_1->date;
    date += dateDelta;
    setEvents(sequence2_1.get(), 200);
    setAnnotations(sequence2_1.get(), 40);

    sequencesDB2.addSequence(sequence2_1);

    if (!sync(sequencesDB2)) {
        std::cout << "Sync failed\n";
        return false;
    }

    if (!compareDatabases(sequencesDB1, sequencesDB2, true)) {
        std::cout << "Database mismatch detected\n";
        return false;
    }

    //
    // Test update sequence
    //

    auto sequences1 = sequencesDB1.getSequences();
    sequencesDB1.readSequenceData(sequences1.at(1));
    sequences1.at(1)->name = "Test & 1_1_Modified";
    sequences1.at(1)->version = sequences1.at(1)->version + 1.0;
    sequences1.at(1)->dataVersion = sequences1.at(1)->dataVersion + 1.0;
    setEvents(sequences1.at(1).get(), 100);
    setAnnotations(sequences1.at(1).get(), 50);
    sequencesDB1.updateSequences({sequences1.at(1)}, false, false, true);

    auto sequences2 = sequencesDB2.getSequences();
    sequencesDB2.readSequenceData(sequences2.at(1));
    sequences2.at(1)->name = "Test & 2_1_Modified";
    sequences2.at(1)->version = sequences2.at(1)->version + 1.0;
    sequences2.at(1)->dataVersion = sequences2.at(1)->dataVersion + 1.0;
    setEvents(sequences2.at(1).get(), 40);
    setAnnotations(sequences2.at(1).get(), 60);
    sequencesDB2.updateSequences({sequences2.at(1)}, false, false, true);

    if (!sync(sequencesDB2)) {
        std::cout << "Sync failed\n";
        return false;
    }

    if (!compareDatabases(sequencesDB1, sequencesDB2, true)) {
        std::cout << "Database mismatch detected\n";
        return false;
    }

    sequences2 = sequencesDB2.getSequences();
    sequencesDB2.readSequenceData(sequences2.at(3));
    if (sequences2.at(3)->name != "Test & 1_1_Modified") {
        std::cout << "Wrong name\n";
        return false;
    }

    if (sequences2.at(3)->data.tracks.at(0)->clips.at(0)->events.size() != 100) {
        std::cout << "Wrong number of events\n";
        return false;
    }

    return true;
}