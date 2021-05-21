//
//  modalviewcontroller.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-01-23.
//  Copyright (c) 2015-2020 Daniel Cliche. All rights reserved.
//

#include "modalviewcontroller.h"

#include <draw.h>
#include <platform.h>

// ---------------------------------------------------------------------------------------------------------------------
ModalViewController::ModalViewController(MDStudio::View* topView, std::shared_ptr<MDStudio::View> view,
                                         std::string uiPath)
    : _topView(topView), _view(view) {
    using namespace std::placeholders;

    _willAppearFn = nullptr;
    _didDisappearFn = nullptr;
    _didReceiveResultFn = nullptr;

    _ui = std::shared_ptr<MDStudio::UI>(new MDStudio::UI());
    _ui->loadUI(_view.get(), uiPath);

    std::shared_ptr<MDStudio::Button> okButton = std::dynamic_pointer_cast<MDStudio::Button>(_ui->findView("okButton"));
    if (okButton) {
        okButton->setType(MDStudio::Button::OKButtonType);
        okButton->setClickedFn(std::bind(&ModalViewController::okButtonClicked, this, _1));
    }

    std::shared_ptr<MDStudio::Button> cancelButton =
        std::dynamic_pointer_cast<MDStudio::Button>(_ui->findView("cancelButton"));
    if (cancelButton) {
        cancelButton->setType(MDStudio::Button::CancelButtonType);
        cancelButton->setClickedFn(std::bind(&ModalViewController::cancelButtonClicked, this, _1));
    }

    view->setHandleEventFn([](MDStudio::View* sender, const MDStudio::UIEvent* event) {
        if (MDStudio::isMouseEvent(event)) {
            if (!MDStudio::isPointInRect(event->pt, sender->resolvedClippedRect())) {
                return true;
            }
        }

        return false;
    });

    view->subviews().at(0)->setHandleEventFn([](MDStudio::View* sender, const MDStudio::UIEvent* event) {
        if (MDStudio::isMouseEvent(event)) {
            if (MDStudio::isPointInRect(event->pt, sender->resolvedClippedRect())) {
                return true;
            }
        }

        return false;
    });
}

// ---------------------------------------------------------------------------------------------------------------------
void ModalViewController::dismiss() {
    MDStudio::Platform::sharedInstance()->invoke([=] {
        //_topView->topView()->setModalView(nullptr);
        _view->responderChain()->releaseResponder(_view.get());
        _topView->removeSubview(_view);
        _topView->setLayoutFn(nullptr);
        _topView->setDirty();

        if (_didDisappearFn) _didDisappearFn(this);
    });
}

// ---------------------------------------------------------------------------------------------------------------------
void ModalViewController::showModal() {
    if (_willAppearFn) _willAppearFn(this);

    _topView->addSubview(_view);
    _topView->setLayoutFn([=](MDStudio::View* sender, MDStudio::Rect frame) {
        if (_ui->contentSize().width != 0 || _ui->contentSize().height != 0)
            _view->setFrame(MDStudio::makeCenteredRectInRect(_topView->bounds(), _ui->contentSize().width,
                                                             _ui->contentSize().height));
    });

    //_topView->topView()->setModalView(_view.get());
    _topView->updateResponderChain();
    _topView->setFrame(_topView->frame());

    // Key window to main
    MDStudio::Platform::sharedInstance()->makeKeyWindow(nullptr);

    _view->responderChain()->captureResponder(_view.get());
}

// ---------------------------------------------------------------------------------------------------------------------
void ModalViewController::okButtonClicked(MDStudio::Button* sender) {
    dismiss();

    if (_didReceiveResultFn) _didReceiveResultFn(this, OKResult);
}

// ---------------------------------------------------------------------------------------------------------------------
void ModalViewController::cancelButtonClicked(MDStudio::Button* sender) {
    dismiss();

    if (_didReceiveResultFn) _didReceiveResultFn(this, CancelResult);
}
