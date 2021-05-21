//
//  levelindicator.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-09-13.
//  Copyright (c) 2014 Daniel Cliche. All rights reserved.
//

#ifndef LEVELINDICATOR_H
#define LEVELINDICATOR_H

#include <functional>

#include "control.h"
#include "imageview.h"

namespace MDStudio {

class LevelIndicator : public Control {
   public:
    typedef std::function<void(LevelIndicator* sender, float level)> levelDidChangeFnType;

   private:
    bool _isCaptured;
    float _level;

    bool handleEvent(const UIEvent* event) override;

    std::shared_ptr<ImageView> _starImageViews[5];

    levelDidChangeFnType _levelDidChangeFn;

   public:
    LevelIndicator(std::string name, void* owner);
    ~LevelIndicator();
    float level() { return _level; }
    void setLevel(float level, bool notifyDelegate = true);
    void setFrame(Rect rect) override;

    void setLevelDidChangeFn(levelDidChangeFnType levelDidChangeFn) { _levelDidChangeFn = levelDidChangeFn; }
};

}  // namespace MDStudio

#endif  // LEVELINDICATOR_H
