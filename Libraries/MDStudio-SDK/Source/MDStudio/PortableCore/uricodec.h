//
//  uricodec.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-09-26.
//  Copyright (c) 2014 Daniel Cliche. All rights reserved.
//

#ifndef URICODEC_H
#define URICODEC_H

#include <string>

namespace MDStudio {

std::string uriDecode(const std::string& sSrc);
std::string uriEncode(const std::string& sSrc);

}  // namespace MDStudio

#endif  // URICODEC_H
