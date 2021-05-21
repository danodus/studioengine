//
//  responder.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-05-23.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef RESPONDER_H
#define RESPONDER_H

#include <list>

#include "uievent.h"

namespace MDStudio {

class Responder {
    std::list<Responder*>::iterator _it;
    bool _isInChain;

    std::string _name;

   public:
    Responder(const std::string& name);
    virtual ~Responder() = default;

    std::string name() { return _name; }

    virtual bool handleEvent(const UIEvent* event);
    virtual void resignFirstResponder();

    virtual void selectAll();

    void addToChain(std::list<Responder*>::iterator it) {
        _isInChain = true;
        _it = it;
    }
    void removeFromChain() { _isInChain = false; };
    bool isInChain() { return _isInChain; }
    std::list<Responder*>::iterator it() { return _it; }
};

}  // namespace MDStudio

#endif  // RESPONDER_H
