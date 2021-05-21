//
//  server.h
//  MelobaseCore
//
//  Created by Daniel Cliche on 2014-06-15.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#ifndef SERVER_H
#define SERVER_H

#if defined(_WIN32) || defined(_LINUX)

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#include "mdns.h"
#include "mdnsd.h"
#endif

#include "mongoose.h"
#include "sequencesdb.h"

namespace MelobaseCore {

class Server {
   public:
    typedef std::function<void(Server* sender)> DidBeginRequestHandlingFnType;
    typedef std::function<void(Server* sender)> DidEndRequestHandlingFnType;
    typedef std::function<void(Server* sender)> DidRequestSaveFnType;

   private:
    struct mdnsd* _svr;
    struct mg_context* _mgContext;
    struct mg_callbacks _mgCallbacks;

    SequencesDB* _sequencesDB;

    DidBeginRequestHandlingFnType _didBeginRequestHandlingFn = nullptr;
    DidEndRequestHandlingFnType _didEndRequestHandlingFn = nullptr;
    DidRequestSaveFnType _didRequestSaveFn = nullptr;

   public:
    Server(SequencesDB* sequencesDB);
    bool start(int port);
    void stop();

    SequencesDB* sequencesDB() { return _sequencesDB; }

    // Internal use only
    void notifyDidBeginRequestHandling();
    void notifyDidEndRequestHandling();
    void notifyDidRequestSave();

    void setDidBeginRequestHandlingFn(DidBeginRequestHandlingFnType didBeginRequestHandlingFn) {
        _didBeginRequestHandlingFn = didBeginRequestHandlingFn;
    }
    void setDidEndRequestHandlingFn(DidEndRequestHandlingFnType didEndRequestHandlingFn) {
        _didEndRequestHandlingFn = didEndRequestHandlingFn;
    }
    void setDidRequestSaveFn(DidRequestSaveFnType didRequestSaveFn) { _didRequestSaveFn = didRequestSaveFn; }
};

}  // namespace MelobaseCore

#endif  // SERVER_H
