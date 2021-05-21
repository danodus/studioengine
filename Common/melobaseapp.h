//
//  melobaseapp.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2014-07-21.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#ifndef MELOBASEAPP_H
#define MELOBASEAPP_H

#include <metronome.h>
#include <sequencesdb.h>
#include <server.h>
#include <studiocontroller.h>
#include <undomanager.h>

#include "app.h"
#include "midihub.h"
#include "sequencer.h"
#include "topview.h"
#include "topviewcontroller.h"

class MelobaseApp : public MDStudio::App {
    MDStudio::UndoManager _undoManager;
    std::shared_ptr<TopView> _topView;
    TopViewController* _topViewController;
    MDStudio::Studio* _studio;
    MelobaseCore::SequencesDB* _sequencesDB;
    MDStudio::Sequencer* _sequencer;
    MelobaseCore::StudioController* _studioController;
    MelobaseCore::Server* _server;
    MIDIHub* _midiHub;

   public:
    MelobaseApp(std::string resourcesPath, std::string dataPath);
    ~MelobaseApp();
    
    void stopServer();

    std::shared_ptr<MDStudio::View> topView() override { return _topView; }

    TopViewController* topViewController() { return _topViewController; }
};

#endif  // MELOBASEAPP_H
