//
//  aboutviewcontroller.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-02-26.
//  Copyright (c) 2015 Daniel Cliche. All rights reserved.
//

#include "aboutviewcontroller.h"

#include <imageview.h>
#include <ui.h>
#include <platform.h>
#include <boxview.h>
#include <labelview.h>

// ---------------------------------------------------------------------------------------------------------------------
AboutViewController::AboutViewController(MDStudio::View *topView, std::shared_ptr<MDStudio::View> view, std::string uiPath) : ModalViewController(topView, view, uiPath)
{
    using namespace std::placeholders;
    
    _acknowledgmementButtonClickedFn = nullptr;
    
    std::shared_ptr<MDStudio::ImageView> imageView = std::dynamic_pointer_cast<MDStudio::ImageView>(_ui->findView("iconImageView"));
    imageView->setIsStretched(true);
    
    std::shared_ptr<MDStudio::Button> acknowledgementsButton = std::dynamic_pointer_cast<MDStudio::Button>(_ui->findView("acknowledgementsButton"));
    acknowledgementsButton->setClickedFn(std::bind(&AboutViewController::acknowledgementsButtonClicked, this, _1));
}

// ---------------------------------------------------------------------------------------------------------------------
AboutViewController::~AboutViewController()
{
}

// ---------------------------------------------------------------------------------------------------------------------
void AboutViewController::acknowledgementsButtonClicked(MDStudio::Button *sender)
{
    if (_acknowledgmementButtonClickedFn)
        _acknowledgmementButtonClickedFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void AboutViewController::showModal()
{
    std::string licenseName = MDStudio::Platform::sharedInstance()->licenseName();
    if (!licenseName.empty()) {
        std::string licensedToStr = _ui->findString("licensedToStr") + " ";
        licensedToStr += licenseName;
        std::shared_ptr<MDStudio::LabelView> licensedToLabel = std::dynamic_pointer_cast<MDStudio::LabelView>(_ui->findView("licensedToLabel"));
        licensedToLabel->setTitle(licensedToStr);
    }
    
    ModalViewController::showModal();
}
