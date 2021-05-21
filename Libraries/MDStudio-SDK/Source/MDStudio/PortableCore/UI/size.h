//
//  size.h
//  MDStudio
//
//  Created by Daniel Cliche on 2013-09-09.
//  Copyright (c) 2013-2020 Daniel Cliche. All rights reserved.
//

#ifndef SIZE_H
#define SIZE_H

namespace MDStudio {

struct Size {
    float width, height;
};

Size makeSize(float width, float height);
Size makeZeroSize();

}  // namespace MDStudio

#endif  // SIZE_H
