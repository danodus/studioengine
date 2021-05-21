//
//  melobasecorescriptmodule.h
//  MelobaseCore
//
//  Created by Daniel Cliche on 2019-09-16.
//  Copyright © 2019 Daniel Cliche. All rights reserved.
//

#ifndef MELOBASECORESCRIPTMODULE_H
#define MELOBASECORESCRIPTMODULE_H

#include <script.h>

namespace MelobaseCore {
    
    class MelobaseCoreScriptModule : public MDStudio::ScriptModule {
        
    public:
        void init(MDStudio::Script *script) override;
    };
}

#endif // MELOBASECORESCRIPTMODULE_H
