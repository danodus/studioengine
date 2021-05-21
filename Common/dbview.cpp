//
//  dbview.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-10-30.
//  Copyright © 2015 Daniel Cliche. All rights reserved.
//

#include "dbview.h"

// ---------------------------------------------------------------------------------------------------------------------
DBView::DBView(std::string name, void *owner) : MDStudio::View(name, owner)
{
    _foldersView = std::make_shared<FoldersView>("FoldersView", this);
    _sequencesView = std::make_shared<SequencesView>("SequencesView", this);
    
    _splitView = std::make_shared<MDStudio::SplitViewH>("SplitView", this, _foldersView, _sequencesView, 200.0f);
    
    addSubview(_splitView);
}

// ---------------------------------------------------------------------------------------------------------------------
void DBView::setFrame(MDStudio::Rect rect)
{
    View::setFrame(rect);
    
    _splitView->setFrame(bounds());
}
