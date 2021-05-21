//
//  instrumentmanager.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-21.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef INSTRUMENTMANAGER_H
#define INSTRUMENTMANAGER_H

#include <memory>
#include <string>
#include <vector>

#include "multiinstrument.h"

namespace MDStudio {

class Preset {
   public:
    int _bank;
    int _number;
    std::string _name;
};

class InstrumentManager {
    std::string _audioPath;
    std::vector<std::shared_ptr<Sample>> _loadedSamples;

   public:
    InstrumentManager(const std::string& audioPath);

    std::shared_ptr<MultiInstrument> loadSF2MultiInstrument(const std::string& name, int presetBank, int preset);
    void unloadMultiInstrument(std::shared_ptr<MultiInstrument> multiInstrument);

    std::vector<Preset> getSF2Presets(const std::string& name);
};

}  // namespace MDStudio

#endif  // INSTRUMENTMANAGER_H
