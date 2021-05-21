//
//  responder.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-05-23.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#include "responder.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
Responder::Responder(const std::string& name) : _name(name) { _isInChain = false; }

// ---------------------------------------------------------------------------------------------------------------------
bool Responder::handleEvent(const UIEvent* event) { return false; }

// ---------------------------------------------------------------------------------------------------------------------
void Responder::resignFirstResponder() {}

// ---------------------------------------------------------------------------------------------------------------------
void Responder::selectAll() {}
