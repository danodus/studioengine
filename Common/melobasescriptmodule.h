//
//  melobasescriptmodule.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2019-09-16.
//  Copyright © 2019 Daniel Cliche. All rights reserved.
//

#ifndef MELOBASESCRIPTMODULE_H
#define MELOBASESCRIPTMODULE_H

#include <script.h>

#include <sequencesdb.h>
#include <sequenceeditor.h>

class MelobaseScriptModule : public MDStudio::ScriptModule {
    
    MelobaseCore::SequencesDB *_sequencesDB;
    MelobaseCore::SequenceEditor *_sequenceEditor;
    
public:
    MelobaseScriptModule(MelobaseCore::SequencesDB *sequencesDB, MelobaseCore::SequenceEditor *sequenceEditor);
    void init(MDStudio::Script *script) override;
    
    MelobaseCore::SequencesDB *sequencesDB() { return _sequencesDB; }
    MelobaseCore::SequenceEditor *sequenceEditor() { return _sequenceEditor; }
};


#endif // MELOBASESCRIPTMODULE_H
