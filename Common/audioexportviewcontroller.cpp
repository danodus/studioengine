//
//  audioexportviewcontroller.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-08-29.
//  Copyright (c) 2015 Daniel Cliche. All rights reserved.
//

#include "audioexportviewcontroller.h"

#include <imageview.h>
#include <ui.h>
#include <platform.h>
#include <boxview.h>

// ---------------------------------------------------------------------------------------------------------------------
AudioExportViewController::AudioExportViewController(MDStudio::View *topView, std::shared_ptr<MDStudio::View> view, std::string uiPath) : ModalViewController(topView, view, uiPath)
{
    _progressIndicator = std::dynamic_pointer_cast<MDStudio::ProgressIndicator>(_ui->findView("progressIndicator"));
}

// ---------------------------------------------------------------------------------------------------------------------
AudioExportViewController::~AudioExportViewController()
{
}

// ---------------------------------------------------------------------------------------------------------------------
void AudioExportViewController::setProgress(float progress)
{
    _progressIndicator->setPos(progress);
}