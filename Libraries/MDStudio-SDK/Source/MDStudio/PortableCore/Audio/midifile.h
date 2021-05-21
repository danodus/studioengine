//
//  midifile.h
//  MDStudio
//
//  Created by Daniel Cliche on 10-10-22.
//  Copyright 2010-2020 Daniel Cliche. All rights reserved.
//

#ifndef MIDIFILE_H
#define MIDIFILE_H

#include <memory>
#include <vector>

#include "event.h"
#include "sequence.h"

namespace MDStudio {

bool writeMIDIFile(std::string path, std::shared_ptr<Sequence> sequence);
std::shared_ptr<Sequence> readMIDIFile(const std::string& path);

std::vector<UInt8> midiFileDataFromSequence(std::shared_ptr<Sequence> sequence);

}  // namespace MDStudio

#endif  // MIDIFILE_H
