//
//  control.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-05-23.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef CONTROL_H
#define CONTROL_H

#include <iostream>

#include "view.h"

namespace MDStudio {
class Control : public View {
   public:
    Control(const std::string& name, void* owner);
};
}  // namespace MDStudio

#endif  // CONTROL_H
