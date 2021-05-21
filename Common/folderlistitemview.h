//
//  folderlistitemview.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-10-31.
//  Copyright © 2015 Daniel Cliche. All rights reserved.
//

#ifndef FOLDERLISTITEMVIEW_H
#define FOLDERLISTITEMVIEW_H

#include <view.h>
#include <labelview.h>
#include <textfield.h>
#include <boxview.h>
#include <melobasecore_sequence.h>

#include <string>
#include <memory>

class FolderListItemView : public MDStudio::View
{
public:
    typedef std::function<void(FolderListItemView *sender, std::shared_ptr<MelobaseCore::Sequence> sequence, bool isLast)> DidReceiveSequenceFnType;
    typedef std::function<void(FolderListItemView *sender, std::shared_ptr<MelobaseCore::SequencesFolder> folder, bool isLast)> DidReceiveFolderFnType;
    
private:
    bool _isHighlighted;
    bool _hasFocus;
    
    bool _isSubviewsConfigured;
    
    int _row;
    
    std::shared_ptr<MDStudio::BoxView> _boxView;
    std::shared_ptr<MDStudio::TextField> _nameTextField;
    
    void setFrame(MDStudio::Rect rect) override;
    
    void configureSubviews() override;
    
    bool isAcceptingDrop(std::shared_ptr<View> draggingView) override;
    bool didReceiveDrop(std::shared_ptr<View> draggingView, bool isLast) override;
    bool isSendingDrop() override;
    
    DidReceiveSequenceFnType _didReceiveSequenceFn;
    DidReceiveFolderFnType _didReceiveFolderFn;
    
    std::shared_ptr<MelobaseCore::SequencesFolder> _folder;
    
public:
    FolderListItemView(std::string name, void *owner, std::shared_ptr<MelobaseCore::SequencesFolder> folder, bool isNameEditable);
    ~FolderListItemView();
    
    void setIsHighlighted(bool isHighlighted);
    void setFocusState(bool focusState);
    
    std::shared_ptr<MDStudio::TextField> nameTextField() { return _nameTextField; }
    
    void startNameEdition();
    
    std::shared_ptr<MelobaseCore::SequencesFolder> folder() { configureSubviews(); return _folder; }
    
    void setDidReceiveSequenceFn(DidReceiveSequenceFnType didReceiveSequenceFn) { _didReceiveSequenceFn = didReceiveSequenceFn; }
    void setDidReceiveFolderFn(DidReceiveFolderFnType didReceiveFolderFn) { _didReceiveFolderFn = didReceiveFolderFn; }
};


#endif // FOLDERLISTITEMVIEW_H
