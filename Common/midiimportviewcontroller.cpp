//
//  midiimportviewcontroller.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-11-27.
//  Copyright © 2015 Daniel Cliche. All rights reserved.
//

#include "midiimportviewcontroller.h"

#include <imageview.h>
#include <ui.h>
#include <platform.h>
#include <boxview.h>

// ---------------------------------------------------------------------------------------------------------------------
MIDIImportViewController::MIDIImportViewController(MDStudio::View *topView, std::shared_ptr<MDStudio::View> view, std::string uiPath) : ModalViewController(topView, view, uiPath)
{
    _progressIndicator = std::dynamic_pointer_cast<MDStudio::ProgressIndicator>(_ui->findView("progressIndicator"));
}

// ---------------------------------------------------------------------------------------------------------------------
MIDIImportViewController::~MIDIImportViewController()
{
}

// ---------------------------------------------------------------------------------------------------------------------
void MIDIImportViewController::setProgress(float progress)
{
    _progressIndicator->setPos(progress);
}