//
//  treeview.h
//  MDStudio
//
//  Created by Daniel Cliche on 2015-10-01.
//  Copyright (c) 2015-2020 Daniel Cliche. All rights reserved.
//

#ifndef TREEVIEW_H
#define TREEVIEW_H

#include "button.h"
#include "control.h"

namespace MDStudio {

class TreeView : public Control {
   public:
    typedef std::function<unsigned int(TreeView* sender, std::vector<int> indexPath)> NbRowsFnType;
    typedef std::function<std::shared_ptr<View>(TreeView* sender, std::vector<int> indexPath, bool* isExpanded)>
        ViewForIndexPathFnType;
    typedef std::function<void(TreeView* sender)> DidPerformLayoutFnType;
    typedef std::function<void(TreeView* sender, std::vector<int> indexPath, bool isExpanded)>
        DidChangeRowExpandedStateFnType;
    typedef std::function<void(TreeView* sender, std::vector<int> indexPath)> DidSelectRowFnType;
    typedef std::function<void(TreeView* sender, std::vector<int> indexPath)> DidDeselectRowFnType;
    typedef std::function<void(TreeView* sender, std::vector<int> indexPath)> DidHoverRowFnType;
    typedef std::function<void(TreeView* sender, std::vector<int> indexPath)> DidConfirmRowSelectionFnType;
    typedef std::function<void(TreeView* sender, bool state)> DidSetFocusStateFnType;

   private:
    Size _itemSize;
    std::vector<unsigned int> _nbRows;
    Size _contentSize;
    bool _hasFocus;
    bool _isWaitingMouseUp;
    std::vector<int> _selectedRow;

    std::vector<std::pair<std::vector<int>, std::shared_ptr<View>>> _items;
    std::vector<std::pair<std::vector<int>, std::shared_ptr<Button>>> _collapseButtons;

    bool _isPassThrough;

    NbRowsFnType _nbRowsFn;
    ViewForIndexPathFnType _viewForIndexPathFn;
    DidPerformLayoutFnType _didPerformLayoutFn;
    DidChangeRowExpandedStateFnType _didChangeRowExpandedStateFn;
    DidSelectRowFnType _didSelectRowFn;
    DidDeselectRowFnType _didDeselectRowFn;
    DidHoverRowFnType _didHoverRowFn;
    DidConfirmRowSelectionFnType _didConfirmRowSelectionFn;
    DidSetFocusStateFnType _didSetFocusStateFn;

    void addBranch(std::vector<int> indexPath);
    void layoutBranch(float& y, unsigned int& viewIndex, std::vector<int> indexPath, bool isVisible);
    void collapseButtonStateDidChange(Button* sender, bool state);

    bool handleEvent(const UIEvent* event) override;

   public:
    TreeView(const std::string& name, void* owner, Size itemSize);
    ~TreeView();

    void setFrame(Rect rect) override;

    Size contentSize() { return _contentSize; }

    void reload();
    void layoutTree(bool isDelegateNotified = true);

    std::shared_ptr<View> viewAtIndexPath(std::vector<int> indexPath);
    std::vector<std::pair<std::vector<int>, std::shared_ptr<View>>> items() { return _items; }

    std::vector<int> selectedRow() { return _selectedRow; }
    void setSelectedRow(std::vector<int> row, bool isDelegateNotified = true);
    void exposeRow(std::vector<int> row, bool isDelegateNotified = true);

    bool hasFocus() { return _hasFocus; }

    void captureFocus();
    void releaseFocus();

    void setIsPassThrough(bool isPassThrough) { _isPassThrough = isPassThrough; }
    bool passThrough() { return _isPassThrough; }

    void setNbRowsFn(NbRowsFnType nbRowsFn) { _nbRowsFn = nbRowsFn; }
    void setViewForIndexPathFn(ViewForIndexPathFnType viewForIndexPathFn) { _viewForIndexPathFn = viewForIndexPathFn; }
    void setDidPerformLayoutFn(DidPerformLayoutFnType didPerformLayoutFn) { _didPerformLayoutFn = didPerformLayoutFn; }
    void setDidChangeRowExpandedState(DidChangeRowExpandedStateFnType didChangeRowExpandedStateFn) {
        _didChangeRowExpandedStateFn = didChangeRowExpandedStateFn;
    }
    void setDidSelectRowFn(DidSelectRowFnType didSelectRowFn) { _didSelectRowFn = didSelectRowFn; }
    void setDidDeselectRowFn(DidSelectRowFnType didDeselectRowFn) { _didDeselectRowFn = didDeselectRowFn; }
    void setDidHoverRowFn(DidHoverRowFnType didHoverRowFn) { _didHoverRowFn = didHoverRowFn; }
    void setDidConfirmRowSelectionFn(DidConfirmRowSelectionFnType didConfirmRowSelectionFn) {
        _didConfirmRowSelectionFn = didConfirmRowSelectionFn;
    }
    void setDidSetFocusStateFn(DidSetFocusStateFnType didSetFocusStateFn) { _didSetFocusStateFn = didSetFocusStateFn; }
};

}  // namespace MDStudio

#endif  // TREEVIEW_H
