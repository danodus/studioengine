//
//  event.h
//  MDStudio
//
//  Created by Daniel Cliche on 10-08-12.
//  Copyright 2010-2020 Daniel Cliche. All rights reserved.
//

#include "event.h"

// ---------------------------------------------------------------------------------------------------------------------
MDStudio::Event MDStudio::makeEvent(UInt8 type, UInt8 channel, UInt32 tickCount, SInt32 param1, SInt32 param2,
                                    std::vector<UInt8> data) {
    Event event;
    event.type = type;
    event.channel = channel;
    event.tickCount = tickCount;
    event.param1 = param1;
    event.param2 = param2;
    event.data = data;

    return event;
}
