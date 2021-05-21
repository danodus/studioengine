//
//  audioexportviewcontroller.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-08-29.
//  Copyright (c) 2015 Daniel Cliche. All rights reserved.
//

#ifndef AUDIOEXPORTVIEWCONTROLLER_H
#define AUDIOEXPORTVIEWCONTROLLER_H

#include "modalviewcontroller.h"

#include <progressindicator.h>

class AudioExportViewController : public ModalViewController {
    
    std::shared_ptr<MDStudio::ProgressIndicator> _progressIndicator;
    
public:
    AudioExportViewController(MDStudio::View *topView, std::shared_ptr<MDStudio::View> view, std::string uiPath);
    ~AudioExportViewController();
    
    void setProgress(float progress);
};
#endif // AUDIOEXPORTVIEWCONTROLLER_H
