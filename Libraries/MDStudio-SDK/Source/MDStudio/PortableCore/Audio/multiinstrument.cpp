//
//  multiinstrument.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-21.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#include "multiinstrument.h"

#include <math.h>

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
MultiInstrument::MultiInstrument() {}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<Instrument> MultiInstrument::instrument(Float32 pitch) {
    std::vector<std::shared_ptr<Instrument>>::iterator it;
    for (it = _instruments.begin(); it != _instruments.end(); it++) {
        if (pitch >= (*it)->lowestPitch() && pitch <= (*it)->highestPitch()) return *it;
    }
    return NULL;
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<Instrument> MultiInstrument::instrument(Float32 pitch, Float32 velocity) {
    velocity = floorf(velocity * 127.0);
    std::vector<std::shared_ptr<Instrument>>::iterator it;
    for (it = _instruments.begin(); it != _instruments.end(); it++) {
        if (pitch >= (*it)->lowestPitch() && pitch <= (*it)->highestPitch() &&
            (velocity >= (*it)->lowestVelocity() && velocity <= (*it)->highestVelocity()))
            return *it;
    }

    return NULL;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<Instrument>> MultiInstrument::instruments(Float32 pitch, Float32 velocity) {
    std::vector<std::shared_ptr<Instrument>> foundInstruments;

    velocity = floorf(velocity * 127.0);
    std::vector<std::shared_ptr<Instrument>>::iterator it;

    for (it = _instruments.begin(); it != _instruments.end(); it++) {
        if (pitch >= (*it)->lowestPitch() && pitch <= (*it)->highestPitch() && velocity >= (*it)->lowestVelocity() &&
            velocity <= (*it)->highestVelocity())
            foundInstruments.push_back(*it);
    }

    return foundInstruments;
}
