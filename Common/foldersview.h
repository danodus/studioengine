//
//  foldersview.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-10-30.
//  Copyright © 2015-2017 Daniel Cliche. All rights reserved.
//

#ifndef FOLDERSVIEW_H
#define FOLDERSVIEW_H

#include <ui.h>
#include <view.h>
#include <scrollview.h>
#include <treeview.h>
#include <button.h>

class FoldersView : public MDStudio::View
{
    MDStudio::UI _ui;
    
    std::shared_ptr<MDStudio::View> _controlsView;
    std::shared_ptr<MDStudio::Button> _newSubfolderButton;
    std::shared_ptr<MDStudio::Button> _recursiveButton;
    
    std::shared_ptr<MDStudio::ScrollView> _treeScrollView;
    std::shared_ptr<MDStudio::TreeView> _treeView;
    
    std::shared_ptr<MDStudio::Image> _newFolderImage;

public:
    
    FoldersView(std::string name, void *owner);
    ~FoldersView();
    
    void setFrame(MDStudio::Rect frame) override;
    
    std::shared_ptr<MDStudio::TreeView> treeView() { return _treeView; }
    std::shared_ptr<MDStudio::ScrollView> treeScrollView() { return _treeScrollView; }
    
    std::shared_ptr<MDStudio::Button> newSubfolderButton() { return _newSubfolderButton; }
    std::shared_ptr<MDStudio::Button> recursiveButton() { return _recursiveButton; }
};


#endif /* FOLDERSVIEW_H */
