//
//  foldersview.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-10-30.
//  Copyright © 2015-2018 Daniel Cliche. All rights reserved.
//

#include "foldersview.h"
#include "platform.h"

// ---------------------------------------------------------------------------------------------------------------------
FoldersView::FoldersView(std::string name, void *owner) : View(name, owner)
{
    _ui.loadUI(this, MDStudio::Platform::sharedInstance()->resourcesPath() + "/FoldersView.lua");
    
    _newFolderImage = std::make_shared<MDStudio::Image>("NewFolder@2x.png");
    
    _controlsView = std::make_shared<MDStudio::View>("ControlsView", this);
    _newSubfolderButton = std::make_shared<MDStudio::Button>("NewSubfolderButton", this, "", _newFolderImage);
    _recursiveButton = std::make_shared<MDStudio::Button>("RecursiveButton", this, _ui.findString("recursive"));
    _recursiveButton->setType(MDStudio::Button::CheckBoxButtonType);
    
    _controlsView->addSubview(_newSubfolderButton);
    _controlsView->addSubview(_recursiveButton);
    
    addSubview(_controlsView);
    
    _treeView = std::make_shared<MDStudio::TreeView>("TreeView", this, MDStudio::makeSize(200.0f, 20.0f));
    // Set it pass though in order to allow the subviews of the selected row to receive events
    _treeView->setIsPassThrough(true);
    _treeScrollView = std::make_shared<MDStudio::ScrollView>("TreeScrollView", this, _treeView, true);
    
    addSubview(_treeScrollView);
}

// ---------------------------------------------------------------------------------------------------------------------
FoldersView::~FoldersView()
{
}

// ---------------------------------------------------------------------------------------------------------------------
void FoldersView::setFrame(MDStudio::Rect frame)
{
    View::setFrame(frame);
    
    MDStudio::Rect r = MDStudio::makeRect(0.0f, rect().size.height - 20.0f, rect().size.width, 20.0f);
    _controlsView->setFrame(r);
    
    r = MDStudio::makeRect(0.0f, 0.0f, 90.0f, 20.0f);
    _recursiveButton->setFrame(r);

    r.origin.x += r.size.width + 8.0f;
    r.size.width = 20.0f;
    _newSubfolderButton->setFrame(r);
    
    
    r = MDStudio::makeRect(0.0f, 0.0f, rect().size.width, rect().size.height - 20.0f);
    _treeScrollView->setFrame(r);
}

