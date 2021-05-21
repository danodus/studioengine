//
//  aboutviewcontroller.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-02-26.
//  Copyright (c) 2015 Daniel Cliche. All rights reserved.
//

#ifndef ABOUT_VIEW_CONTROLLER_H
#define ABOUT_VIEW_CONTROLLER_H

#include "modalviewcontroller.h"

class AboutViewController : public ModalViewController {

    typedef std::function<void(AboutViewController *sender)> AcknowledgementButtonClickedFnType;

private:

    AcknowledgementButtonClickedFnType _acknowledgmementButtonClickedFn;
    
    void acknowledgementsButtonClicked(MDStudio::Button *sender);

public:
    AboutViewController(MDStudio::View *topView, std::shared_ptr<MDStudio::View> view, std::string uiPath);
    ~AboutViewController();
    
    void showModal() override;
    
    void setAcknowledgementButtonClickedFn(AcknowledgementButtonClickedFnType acknowledgmementButtonClickedFn) { _acknowledgmementButtonClickedFn = acknowledgmementButtonClickedFn; }
};


#endif // ABOUT_VIEW_CONTOLLER_H
