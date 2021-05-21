//
//  midiimport.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-11-27.
//  Copyright © 2015 Daniel Cliche. All rights reserved.
//

#ifndef MIDIIMPORT_H
#define MIDIIMPORT_H

#include <sequence.h>
#include <sequencesdb.h>
#include <thread>
#include <vector>
#include <string>


class MIDIImport {
    
public:
    
    typedef std::function<void(MIDIImport *sender)>midiImportDidStartFnType;
    typedef std::function<void(MIDIImport *sender, float progress)>midiImportDidSetProgressFnType;
    typedef std::function<void(MIDIImport *sender, bool isSuccessful)>midiImportDidFinishFnType;
    
private:
    
    MelobaseCore::SequencesDB *_sequencesDB;
    
    std::vector<std::string> _paths;
    std::shared_ptr<MelobaseCore::SequencesFolder> _folder;
    
    std::thread _importMIDIThread;
    
    UInt64 _lastImportedSequenceID;
    
    void importMIDIThread();
    void importMIDICompleted(bool isSuccessful);
    
    midiImportDidStartFnType _midiImportDidStartFn;
    midiImportDidSetProgressFnType _midiImportDidSetProgressFn;
    midiImportDidFinishFnType _midiImportDidFinishFn;
    
    void setProgress(float progress);
    
public:
    
    MIDIImport(MelobaseCore::SequencesDB *sequencesDB);
    
    bool importMIDI(std::vector<std::string> paths, std::shared_ptr<MelobaseCore::SequencesFolder> folder);
    
    std::vector<std::string> paths() { return _paths; }
    
    UInt64 lastImportedSequenceID() { return _lastImportedSequenceID; }
    
    void setMIDIImportDidStartFn(midiImportDidStartFnType midiImportDidStartFn) { _midiImportDidStartFn = midiImportDidStartFn; }
    void setMIDIImportDidSetProgressFn(midiImportDidSetProgressFnType midiImportDidSetProgressFn) { _midiImportDidSetProgressFn = midiImportDidSetProgressFn; }
    void setMIDIImportDidFinishFn(midiImportDidFinishFnType midiImportDidFinishFn) { _midiImportDidFinishFn = midiImportDidFinishFn; }
};



#endif // MIDIIMPORT_H
