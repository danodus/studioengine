//
//  dbview.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-10-30.
//  Copyright © 2015-2018 Daniel Cliche. All rights reserved.
//

#ifndef DBVIEW_H
#define DBVIEW_H

#include <view.h>
#include "foldersview.h"
#include "sequencesview.h"
#include <splitviewh.h>

class DBView : public MDStudio::View {
    
    std::shared_ptr<FoldersView> _foldersView;
    std::shared_ptr<SequencesView> _sequencesView;
    std::shared_ptr<MDStudio::SplitViewH> _splitView;

public:
    
    DBView(std::string name, void *owner);
    
    void setFrame(MDStudio::Rect rect) override;
    
    std::shared_ptr<FoldersView> foldersView() { return _foldersView; }
    std::shared_ptr<SequencesView> sequencesView() { return _sequencesView; }
    std::shared_ptr<MDStudio::SplitViewH> splitView() { return _splitView; }
};


#endif // DBVIEW_H
