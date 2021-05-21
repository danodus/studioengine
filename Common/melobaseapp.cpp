//
//  melobaseapp.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2014-07-21.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#include "melobaseapp.h"

#include <font.h>
#include <melobasecore.h>
#include <platform.h>

#include "helpers.h"
#include "midihub.h"

#define SERVER_PORT 8008

// ---------------------------------------------------------------------------------------------------------------------
MelobaseApp::MelobaseApp(std::string resourcesPath, std::string dataPath) : App("Melobase") {
    // Set the application core version
    MDStudio::Platform::sharedInstance()->setAppCoreVersion(MELOBASECORE_VERSION);

    // Load common script
    loadCommonScript();

    // Load the database
    _sequencesDB = new MelobaseCore::SequencesDB(dataPath, &_undoManager);
    _sequencesDB->open(false);

    _studio = new MDStudio::Studio(STUDIO_MAX_CHANNELS, resourcesPath);
    _sequencer = new MDStudio::Sequencer(this, _studio);
    _studioController = new MelobaseCore::StudioController(this, _studio, _sequencer);

    _topView = std::shared_ptr<TopView>(new TopView("topView", _topViewController, _studio, 0.1, 14.0));
    _topView->createResponderChain();
    _topView->createTooltipManager();

    _midiHub = new MIDIHub();

    _server = new MelobaseCore::Server(_sequencesDB);

    _topViewController =
        new TopViewController(_topView, _studioController, _sequencesDB, _server, _midiHub->defaultMIDIInputPortName(),
                              _midiHub->defaultMIDIOutputPortName());

    // Start the mixer
    _studio->setAudioOutputDevice(_topViewController->audioOutputDeviceName(),
                                  _topViewController->audioOutputLatency());
    _studio->startMixer();

    // Dump initial studio states in order to have the views synchronized
    _studio->dumpStates(STUDIO_SOURCE_USER);

    // Start the server
    _server->start(SERVER_PORT);

    // Start MIDI hub updates
    _midiHub->start(_topViewController);
}

// ---------------------------------------------------------------------------------------------------------------------
MelobaseApp::~MelobaseApp() {
    delete _midiHub;

    // Stop the server
    stopServer();

    delete _topViewController;
    _topViewController = nullptr;

    delete _studioController;
    delete _sequencer;
    delete _studio;

    delete _sequencesDB;

    // Unload common script
    unloadCommonScript();
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseApp::stopServer() {
    if (_server) {
        _server->stop();
        delete _server;
        _server = nullptr;
    }
}
