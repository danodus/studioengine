//
//  sequenceutils.cpp
//  MelobaseCoreTests
//
//  Created by Daniel Cliche on 2021-02-04.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#include "sequenceutils.h"

#include <iostream>

// ---------------------------------------------------------------------------------------------------------------------
void setEvents(MelobaseCore::Sequence* sequence, size_t nbEvents) {
    sequence->data.tracks.at(0)->clips.at(0)->events.clear();
    for (auto i = 0; i < nbEvents; i++) {
        auto event = std::make_shared<MelobaseCore::ChannelEvent>(CHANNEL_EVENT_TYPE_NOTE, 0, 0, 100);
        sequence->data.tracks.at(0)->clips.at(0)->addEvent(event);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void setAnnotations(MelobaseCore::Sequence* sequence, size_t nbAnnotations) {
    sequence->annotations.clear();
    for (auto i = 0; i < nbAnnotations; i++) {
        auto annotation = std::make_shared<MelobaseCore::SequenceAnnotation>();
        annotation->tickCount = i * 100;
        sequence->annotations.emplace_back(annotation);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool compareDatabaseFolders(MelobaseCore::SequencesDB& sequencesDB1, MelobaseCore::SequencesDB& sequencesDB2,
                            UInt64 parentFolderID1, UInt64 parentFolderID2) {
    auto folders1 = sequencesDB1.getFolders(sequencesDB2.getFolderWithID(parentFolderID1));
    auto folders2 = sequencesDB2.getFolders(sequencesDB2.getFolderWithID(parentFolderID2));

    if (folders1.size() != folders2.size()) {
        std::cout << "Invalid nb of folders\n";
        return false;
    }

    size_t i = 0;
    for (auto f : folders1) {
        // Find folder
        MelobaseCore::SequencesFolder* foundFolder = nullptr;
        for (auto f2 : folders2) {
            if (f2->date == f->date) {
                foundFolder = f2.get();
                break;
            }
        }

        if (!foundFolder) {
            std::cout << "Folder not found\n";
            return false;
        }

        if (f->name != foundFolder->name) {
            std::cout << "Invalid folder name\n";
            return false;
        }

        if (!compareDatabaseFolders(sequencesDB1, sequencesDB2, f->id, foundFolder->id)) return false;
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool compareSequences(MelobaseCore::Sequence* s1, MelobaseCore::Sequence* s2, bool mustHaveEvents) {
    if (s1->name != s2->name) {
        std::cout << "Invalid sequence name\n";
        return false;
    }

    //
    // Compare annotations
    //

    if (s1->annotations.size() != s2->annotations.size()) {
        std::cout << "Nb of annotations mismatch\n";
        return false;
    }

    for (auto i = 0; i < s1->annotations.size(); ++i) {
        if (s1->annotations[i]->tickCount != s2->annotations[i]->tickCount) {
            std::cout << "Annotation mismatch\n";
            return false;
        }
    }

    //
    // Compare data
    //

    auto events1 = s1->data.tracks.at(0)->clips.at(0)->events;
    auto events2 = s2->data.tracks.at(0)->clips.at(0)->events;

    if (mustHaveEvents) {
        if (events1.size() == 0) {
            std::cout << "No events for sequence " << s1->name << "\n";
            return false;
        }

        if (events2.size() == 0) {
            std::cout << "No events for sequence " << s2->name << "\n";
            return false;
        }
    }

    if (events1.size() != events2.size()) {
        std::cout << "Event data size mismatch\n";
        return false;
    }

    size_t i = 0;
    for (auto e : events1) {
        auto channelEvent1 = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(e);
        auto channelEvent2 = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(events2[i]);

        if (channelEvent1->type() != channelEvent2->type()) {
            std::cout << "Channel event mismatch\n";
            return false;
        }
        i++;
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool compareDatabaseSequences(MelobaseCore::SequencesDB& sequencesDB1, MelobaseCore::SequencesDB& sequencesDB2,
                              bool mustHaveEvents) {
    auto sequences1 = sequencesDB1.getSequences();
    auto sequences2 = sequencesDB2.getSequences();

    if (sequences1.size() != sequences2.size()) {
        std::cout << "Invalid nb of sequences\n";
        return false;
    }

    size_t i = 0;
    for (auto s : sequences1) {
        // Find sequence
        std::shared_ptr<MelobaseCore::Sequence> foundSequence = nullptr;
        for (auto s2 : sequences2) {
            if (s2->date == s->date) {
                foundSequence = s2;
                break;
            }
        }

        if (!foundSequence) {
            std::cout << "Sequence not found\n";
            return false;
        }

        if (!sequencesDB1.readSequenceData(s)) {
            std::cout << "Unable to read events\n";
            return false;
        }

        if (!sequencesDB2.readSequenceData(foundSequence)) {
            std::cout << "Unable to read events\n";
            return false;
        }

        if (!compareSequences(s.get(), foundSequence.get(), mustHaveEvents)) {
            return false;
        }
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool compareDatabases(MelobaseCore::SequencesDB& sequencesDB1, MelobaseCore::SequencesDB& sequencesDB2,
                      bool mustHaveEvents) {
    if (!compareDatabaseFolders(sequencesDB1, sequencesDB2, SEQUENCES_FOLDER_ID, SEQUENCES_FOLDER_ID)) return false;
    if (!compareDatabaseSequences(sequencesDB1, sequencesDB2, mustHaveEvents)) return false;
    return true;
}
