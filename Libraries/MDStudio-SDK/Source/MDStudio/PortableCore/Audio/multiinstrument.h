//
//  multiinstrument.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-21.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef MULTIINSTRUMENT_H
#define MULTIINSTRUMENT_H

#include <memory>
#include <vector>

#include "instrument.h"
#include "types.h"

namespace MDStudio {

class MultiInstrument {
    std::vector<std::shared_ptr<Instrument>> _instruments;

   public:
    MultiInstrument();

    std::shared_ptr<Instrument> instrument(Float32 pitch);
    std::shared_ptr<Instrument> instrument(Float32 pitch, Float32 velocity);

    std::vector<std::shared_ptr<Instrument>>::iterator instrumentsBegin() { return _instruments.begin(); }
    std::vector<std::shared_ptr<Instrument>>::iterator instrumentsEnd() { return _instruments.end(); }
    void addInstrument(std::shared_ptr<Instrument> instrument) { _instruments.push_back(instrument); }
    std::vector<std::shared_ptr<Instrument>> instruments(Float32 pitch, Float32 velocity);
};

}  // namespace MDStudio

#endif  // MULTIINSTRUMENT_H
