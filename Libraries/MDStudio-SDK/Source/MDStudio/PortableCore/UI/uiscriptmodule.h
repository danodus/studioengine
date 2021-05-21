//
//  uiscriptmodule.h
//  MDStudio
//
//  Created by Daniel Cliche on 2018-04-07.
//  Copyright © 2018-2020 Daniel Cliche. All rights reserved.
//

#ifndef UISCRIPTMODULE_H
#define UISCRIPTMODULE_H

#include <script.h>
#include <view.h>

namespace MDStudio {

class UIScriptModule : public ScriptModule {
    View* _topView;
    Size _contentSize;

   public:
    UIScriptModule(View* topView) : _topView(topView) { _contentSize = makeZeroSize(); }

    View* topView() { return _topView; }

    void init(Script* script) override;

    void setContentSize(Size contentSize) { _contentSize = contentSize; }
    Size contentSize() { return _contentSize; }
};

}  // namespace MDStudio

#endif  // UISCRIPTMODULE_H
