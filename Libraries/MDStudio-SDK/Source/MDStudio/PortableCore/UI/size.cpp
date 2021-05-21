//
//  size.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2013-09-09.
//  Copyright (c) 2013-2020 Daniel Cliche. All rights reserved.
//

#include "size.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
Size MDStudio::makeSize(float width, float height) {
    Size size;
    size.width = width;
    size.height = height;
    return size;
}

// ---------------------------------------------------------------------------------------------------------------------
Size MDStudio::makeZeroSize() { return {0.0f, 0.0f}; }