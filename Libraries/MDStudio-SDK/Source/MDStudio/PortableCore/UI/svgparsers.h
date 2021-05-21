//
//  svgparsers.h
//  MDStudio
//
//  Created by Daniel Cliche on 2018-01-16.
//  Copyright (c) 2018-2020 Daniel Cliche. All rights reserved.
//

#ifndef SVGPARSERS_H
#define SVGPARSERS_H

#include "path.h"

namespace MDStudio {

bool parseSVGPolygon(Path* path, const char* s);
bool parseSVGPath(Path* path, const char* s, bool* isOutlineClosed = nullptr);

}  // namespace MDStudio

#endif  // SVGPARSERS_H
