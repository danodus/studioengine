//
//  progressindicator.h
//  MDStudio
//
//  Created by Daniel Cliche on 2015-08-29.
//  Copyright (c) 2015-2020 Daniel Cliche. All rights reserved.
//

#ifndef PROGRESSINDICATOR_H
#define PROGRESSINDICATOR_H

#include "view.h"

namespace MDStudio {
class ProgressIndicator : public View {
    float _max, _pos;

    void draw() override;

   public:
    ProgressIndicator(const std::string& name, void* owner, float max);
    void setPos(float pos);
};
}  // namespace MDStudio

#endif  // PROGRESSINDICATOR_H
