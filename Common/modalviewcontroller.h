//
//  modalviewcontroller.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-01-23.
//  Copyright (c) 2015 Daniel Cliche. All rights reserved.
//

#ifndef MODALVIEWCONTROLLER_H
#define MODALVIEWCONTROLLER_H

#include <view.h>
#include <ui.h>
#include <button.h>

#include <memory>

class ModalViewController {
    
public:
    
    typedef enum {CancelResult, OKResult} ResultEnumType;
    
    typedef std::function<void(ModalViewController *sender)> WillAppearFnType;
    typedef std::function<void(ModalViewController *sender)> DidDisappearFnType;
    typedef std::function<void(ModalViewController *sender, ResultEnumType result)> DidReceiveResultFnType;
    
protected:
    
    std::shared_ptr<MDStudio::UI> _ui;
    MDStudio::View *_topView;
    std::shared_ptr<MDStudio::View> _view;
    
    void okButtonClicked(MDStudio::Button *sender);
    void cancelButtonClicked(MDStudio::Button *sender);

    WillAppearFnType _willAppearFn;
    DidDisappearFnType _didDisappearFn;
    DidReceiveResultFnType _didReceiveResultFn;
    
public:
    
    ModalViewController(MDStudio::View *topView, std::shared_ptr<MDStudio::View> view, std::string uiPath);
    virtual void showModal();
    virtual void dismiss();
    
    void setWillAppear(WillAppearFnType willAppearFn) { _willAppearFn = willAppearFn; }
    void setDidDisappear(DidDisappearFnType didDisappearFn) { _didDisappearFn = didDisappearFn; }
    void setDidReceiveResult(DidReceiveResultFnType didReceiveResultFn) { _didReceiveResultFn = didReceiveResultFn; }
};


#endif // MODALVIEWCONTROLLER_H
