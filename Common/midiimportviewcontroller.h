//
//  midiimportviewcontroller.hpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-11-27.
//  Copyright © 2015 Daniel Cliche. All rights reserved.
//

#ifndef MIDIIMPORTVIEWCONTROLLER_H
#define MIDIIMPORTVIEWCONTROLLER_H

#include "modalviewcontroller.h"

#include <progressindicator.h>

class MIDIImportViewController : public ModalViewController {
    
    std::shared_ptr<MDStudio::ProgressIndicator> _progressIndicator;
    
public:
    MIDIImportViewController(MDStudio::View *topView, std::shared_ptr<MDStudio::View> view, std::string uiPath);
    ~MIDIImportViewController();
    
    void setProgress(float progress);
};
#endif // MIDIIMPORTVIEWCONTROLLER_H
