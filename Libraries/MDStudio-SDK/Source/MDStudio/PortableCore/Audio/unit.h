//
//  unit.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-21.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef UNIT_H
#define UNIT_H

#include <atomic>

#include "../types.h"

namespace MDStudio {

class Unit {
    std::atomic<bool> _isRunning;

   public:
    Unit();
    virtual ~Unit() = default;

    virtual int renderInput(UInt32 inNumberFrames, GraphSampleType* ioData[2], UInt32 stride = 1) = 0;

    void setIsRunning(bool isRunning) { _isRunning = isRunning; }
    bool isRunning() { return _isRunning; }
};

}  // namespace MDStudio

#endif  // UNIT_H
