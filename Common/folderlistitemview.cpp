//
//  folderlistitemview.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-10-31.
//  Copyright © 2015-2018 Daniel Cliche. All rights reserved.
//

#include "folderlistitemview.h"
#include "helpers.h"

#include <draw.h>
#include <platform.h>
#include "sequencelistitemview.h"

#include <sstream> // stringstream
#include <iomanip>

// ---------------------------------------------------------------------------------------------------------------------
FolderListItemView::FolderListItemView(std::string name, void *owner, std::shared_ptr<MelobaseCore::SequencesFolder> folder, bool isNameEditable) : View(name, owner), _folder(folder)
{
    _didReceiveSequenceFn = nullptr;
    _didReceiveFolderFn = nullptr;
    
    _isHighlighted = false;
    _hasFocus = false;
    
    _isSubviewsConfigured = false;
    
    _boxView = std::shared_ptr<MDStudio::BoxView>(new MDStudio::BoxView("boxView", owner));
    _boxView->setBorderColor(MDStudio::blackColor);
    _nameTextField = std::shared_ptr<MDStudio::TextField>(new MDStudio::TextField("nameTextField", owner));
    _nameTextField->setIsEnabled(isNameEditable);
    
    addSubview(_boxView);
    addSubview(_nameTextField);
}

// ---------------------------------------------------------------------------------------------------------------------
FolderListItemView::~FolderListItemView()
{
    removeSubview(_boxView);
    removeSubview(_nameTextField);
}

// ---------------------------------------------------------------------------------------------------------------------
void FolderListItemView::setFrame(MDStudio::Rect aRect)
{
    View::setFrame(aRect);
    _boxView->setFrame(bounds());
    _nameTextField->setFrame(bounds());
}

// ---------------------------------------------------------------------------------------------------------------------
void FolderListItemView::setIsHighlighted(bool isHighlighted)
{
    _isHighlighted = isHighlighted;
    _boxView->setFillColor(_isHighlighted ? (_hasFocus ? MDStudio::blueColor : MDStudio::veryDimGrayColor) : MDStudio::blackColor);
}

// ---------------------------------------------------------------------------------------------------------------------
void FolderListItemView::setFocusState(bool focusState)
{
    _hasFocus = focusState;
    _boxView->setFillColor(_isHighlighted ? (_hasFocus ? MDStudio::blueColor : MDStudio::veryDimGrayColor) : MDStudio::blackColor);
}

// ---------------------------------------------------------------------------------------------------------------------
void FolderListItemView::configureSubviews()
{
    if (_isSubviewsConfigured)
        return;
    
    _nameTextField->setText(_folder ? localizedFolderName(_folder.get()) : "<NULL>", false);
    
    _isSubviewsConfigured = true;
    
}

// ---------------------------------------------------------------------------------------------------------------------
void FolderListItemView::startNameEdition()
{
    _nameTextField->startEdition();
}

// ---------------------------------------------------------------------------------------------------------------------
bool FolderListItemView::isAcceptingDrop(std::shared_ptr<View> draggingView)
{
    // Check if the dragging folder is a parent of this folder. If so, we do not accept the drop operation.
    if (std::dynamic_pointer_cast<FolderListItemView>(draggingView)) {
        auto draggingFolderListItemView = std::dynamic_pointer_cast<FolderListItemView>(draggingView);
        auto draggingFolder = draggingFolderListItemView->folder();
        
        // We do not drop on itself
        if (draggingFolder == _folder)
            return false;
        
        auto parentFolder = _folder->parent;
        while (parentFolder) {
            if (parentFolder == draggingFolder)
                return false;
            parentFolder = parentFolder->parent;
        }
    }
    
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool FolderListItemView::didReceiveDrop(std::shared_ptr<View> draggingView, bool isLast)
{
    if (std::dynamic_pointer_cast<SequenceListItemView>(draggingView)) {
        MDStudio::Platform::sharedInstance()->invoke([=] {
            auto sequenceListItemView = std::dynamic_pointer_cast<SequenceListItemView>(draggingView);
            
            if (_didReceiveSequenceFn)
                _didReceiveSequenceFn(this, sequenceListItemView->sequence(), isLast);
        });
    } else if (std::dynamic_pointer_cast<FolderListItemView>(draggingView)) {
        MDStudio::Platform::sharedInstance()->invoke([=] {
            auto folderListItemView = std::dynamic_pointer_cast<FolderListItemView>(draggingView);
            
            if (_didReceiveFolderFn)
                _didReceiveFolderFn(this, folderListItemView->folder(), isLast);
        });
    }
    
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool FolderListItemView::isSendingDrop()
{
    return _isHighlighted && _hasFocus;
}
