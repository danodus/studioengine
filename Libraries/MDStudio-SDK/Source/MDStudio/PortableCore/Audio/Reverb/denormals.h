//
//  denormals.h
//  MDStudio
//
//  Created by Daniel Cliche on 11-05-24.
//  Copyright 2010-2020 Daniel Cliche. All rights reserved.
//

// Based on public domain code written by Jezar at Dreampoint, June 2000, http://www.dreampoint.co.uk
// Based on IS_DENORMAL macro by Jon Watte

#ifndef DENORMALS_H
#define DENORMALS_H

// Macro for killing denormalled numbers
#define undenormalise(sample) \
    if (((*(unsigned int*)&sample) & 0x7f800000) == 0) sample = 0.0f

#endif  // DENORMALS_H
