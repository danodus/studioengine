//
//  midiimport.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-11-27.
//  Copyright (c) 2015-2021 Daniel Cliche. All rights reserved.
//

#include "midiimport.h"

#include <midifile.h>
#include <platform.h>

// ---------------------------------------------------------------------------------------------------------------------
static std::string getFileName(const std::string& s) {
    std::string ret = s;

    // Remove the directory
    char sep = '/';

#ifdef _WIN32
    sep = '\\';
#endif

    size_t i = ret.rfind(sep, ret.length());
    if (i != std::string::npos) {
        ret = ret.substr(i + 1, ret.length() - i);
    }

    // Remove the extension
    i = ret.rfind(".", ret.length());
    if (i != std::string::npos) {
        ret = ret.substr(0, i);
    }

    return ret;
}

// ---------------------------------------------------------------------------------------------------------------------
MIDIImport::MIDIImport(MelobaseCore::SequencesDB* sequencesDB) : _sequencesDB(sequencesDB) {
    _midiImportDidStartFn = nullptr;
    _midiImportDidSetProgressFn = nullptr;
    _midiImportDidFinishFn = nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------
// MIDI import thread
void MIDIImport::importMIDIThread() {
    size_t count = 0;
    size_t total = _paths.size();

    bool isSuccessful = false;
    for (auto path : _paths) {
        std::shared_ptr<MDStudio::Sequence> studioSequence = MDStudio::readMIDIFile(path);
        if (studioSequence) {
            std::shared_ptr<MelobaseCore::Sequence> sequence = MelobaseCore::getMelobaseCoreSequence(studioSequence);

            sequence->date = sequence->version = sequence->dataVersion = MDStudio::getTimestamp();
            sequence->rating = 0.0f;
            sequence->name = getFileName(path);
            sequence->playCount = 0;

            sequence->folder = _folder;
            if (_paths.size() > 1) _sequencesDB->undoManager()->disableRegistration();
            _sequencesDB->addSequence(sequence, false);
            _lastImportedSequenceID = sequence->id;
            if (_paths.size() > 1) _sequencesDB->undoManager()->enableRegistration();
            isSuccessful = true;
        }
        MDStudio::Platform::sharedInstance()->invoke(
            [=] { setProgress(static_cast<float>(count) / static_cast<float>(total)); });
        count++;
    }

    MDStudio::Platform::sharedInstance()->invoke([=] { importMIDICompleted(isSuccessful); });
}

// ---------------------------------------------------------------------------------------------------------------------
bool MIDIImport::importMIDI(std::vector<std::string> paths, std::shared_ptr<MelobaseCore::SequencesFolder> folder) {
    _paths = paths;
    _folder = folder;

    if (_midiImportDidStartFn) _midiImportDidStartFn(this);

    // Create a new MIDI import thread
    _importMIDIThread = std::thread(&MIDIImport::importMIDIThread, this);

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
void MIDIImport::importMIDICompleted(bool isSuccessful) {
    _importMIDIThread.join();

    if (_midiImportDidFinishFn) _midiImportDidFinishFn(this, isSuccessful);
}

// ---------------------------------------------------------------------------------------------------------------------
void MIDIImport::setProgress(float progress) {
    if (_midiImportDidSetProgressFn) _midiImportDidSetProgressFn(this, progress);
}
