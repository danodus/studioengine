//
//  audioscriptmodule.h
//  MDStudio
//
//  Created by Daniel Cliche on 2018-04-07.
//  Copyright © 2018-2020 Daniel Cliche. All rights reserved.
//

#ifndef AUDIOSCRIPTMODULE_H
#define AUDIOSCRIPTMODULE_H

#include <script.h>

namespace MDStudio {

class AudioScriptModule : public ScriptModule {
   public:
    void init(Script* script) override;
};

}  // namespace MDStudio

#endif  // AUDIOSCRIPTMODULE_H
