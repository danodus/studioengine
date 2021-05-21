//
//  pasteboard.h
//  MDStudio
//
//  Created by Daniel Cliche on 2015-06-30.
//  Copyright (c) 2015-2019 Daniel Cliche. All rights reserved.
//

#ifndef PASTEBOARD_H
#define PASTEBOARD_H

#include "any.h"

namespace MDStudio {

class Pasteboard {
    Any _content;

   public:
    static Pasteboard* sharedInstance();

    void setContent(Any content);
    void clear();

    bool isContentAvailable();
    Any content();
};

}  // namespace MDStudio

#endif  // PASTEBOARD_H
