//
//  test_sequencesdb.cpp
//  MelobaseCoreTests
//
//  Created by Daniel Cliche on 2021-02-04.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#include "test_sequencesdb.h"

#include <iostream>

#include "sequenceutils.h"

// ---------------------------------------------------------------------------------------------------------------------
bool testSequencesDB() {
    MelobaseCore::SequencesDB sequencesDB("/tmp/test_sequencesdb.sqlite");
    if (!sequencesDB.open(true)) {
        std::cout << "Unable to open DB\n";
        return false;
    }

    auto sequence1 = std::make_shared<MelobaseCore::Sequence>();
    sequence1->name = "Test1_0";
    sequence1->folder = sequencesDB.getFolderWithID(SEQUENCES_FOLDER_ID);
    sequence1->date = 633974598.60000002;
    sequence1->version = sequence1->date;
    sequence1->dataVersion = sequence1->date;
    setEvents(sequence1.get(), 10);
    setAnnotations(sequence1.get(), 10);

    if (!sequencesDB.addSequence(sequence1)) {
        std::cout << "Unable to add sequence\n";
        return false;
    }

    // Using getSequences()

    auto sequences = sequencesDB.getSequences();
    auto sequence2 = sequences.at(0);

    if (!sequencesDB.readSequenceData(sequence2)) {
        std::cout << "Unable to read sequence data\n";
        return false;
    }

    if (!compareSequences(sequence1.get(), sequence2.get(), true)) {
        std::cout << "Sequence mismatch\n";
        return false;
    }

    // Using getSequence()

    auto nbSequences =
        sequencesDB.getNbSequences(MelobaseCore::SequencesDB::sequencesFilterEnum::All, "", nullptr, false);
    if (nbSequences != 1) {
        std::cout << "Invalid nb of sequences\n";
        return false;
    }

    sequence2 = sequencesDB.getSequence(0, MelobaseCore::SequencesDB::sequencesFilterEnum::All, "", nullptr, false);
    if (!sequence2) {
        std::cout << "Unable to get sequence\n";
        return false;
    }

    if (!sequencesDB.readSequenceData(sequence2)) {
        std::cout << "Unable to read sequence data\n";
        return false;
    }

    if (!compareSequences(sequence1.get(), sequence2.get(), true)) {
        std::cout << "Sequence mismatch\n";
        return false;
    }

    // Update annotations

    setAnnotations(sequence1.get(), 0);
    setAnnotations(sequence2.get(), 0);
    sequencesDB.updateSequences({sequence2});

    sequence2 = sequencesDB.getSequenceWithID(1);
    sequencesDB.readSequenceData(sequence2);

    if (!compareSequences(sequence1.get(), sequence2.get(), true)) {
        std::cout << "Sequence mismatch\n";
        return false;
    }

    return true;
}