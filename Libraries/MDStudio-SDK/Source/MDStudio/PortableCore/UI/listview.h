//
//  listview.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-26.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <functional>
#include <map>
#include <memory>

#include "control.h"

namespace MDStudio {

class ListView : public Control {
   public:
    typedef std::function<unsigned int(ListView* sender)> NbRowsFnType;
    typedef std::function<std::shared_ptr<View>(ListView* sender, int row)> ViewForRowFnType;
    typedef std::function<void(ListView* sender, int row)> DidSelectRowFnType;
    typedef std::function<void(ListView* sender, int row)> DidDeselectRowFnType;
    typedef std::function<void(ListView* sender, int row)> DidHoverRowFnType;
    typedef std::function<void(ListView* sender, int row)> DidConfirmRowSelectionFnType;
    typedef std::function<void(ListView* sender, bool state)> DidSetFocusStateFnType;
    typedef std::function<bool(ListView* sender, unsigned int key)> DidPressUnhandledKeyFnType;

   private:
    // Data source
    NbRowsFnType _nbRowsFn;
    ViewForRowFnType _viewForRowFn;

    DidSelectRowFnType _didSelectRowFn;
    DidDeselectRowFnType _didDeselectRowFn;
    DidHoverRowFnType _didHoverRowFn;
    DidConfirmRowSelectionFnType _didConfirmRowSelectionFn;
    DidSetFocusStateFnType _didSetFocusStateFn;
    DidPressUnhandledKeyFnType _didPressUnhandledKeyFn;

    int _selectedRow = -1;
    int _selectedRowRef = -1;
    std::vector<int> _selectedRows;
    std::map<int, std::shared_ptr<View>> _items;

    float _rowHeight;

    bool handleEvent(const UIEvent* event) override;

    bool _hasFocus;
    bool _isWaitingMouseUp;
    bool _isDeselectingRow;

    unsigned int _nbRows;
    bool _isMultipleSelectionsAllowed;

    bool _isPassThrough;
    bool _isStatic = false;
    bool _isCapturing = true;

    bool _isReloadWindowPending = false;

    bool isRowSelected(int row);
    void deselectRow(int row);
    void setSelectedRangeRows(int firstRow, int lastRow);

    void layoutList();

    void selectAll() override;

   public:
    ListView(const std::string& name, void* owner, float rowHeight, bool isMultipleSelectionsAllowed = false);
    ~ListView();

    // Data source
    void setNbRowsFn(NbRowsFnType nbRowsFn) { _nbRowsFn = nbRowsFn; }
    void setViewForRowFn(ViewForRowFnType viewForRowFn) { _viewForRowFn = viewForRowFn; }

    void setDidSelectRowFn(DidSelectRowFnType didSelectRowFn) { _didSelectRowFn = didSelectRowFn; }
    void setDidDeselectRowFn(DidSelectRowFnType didDeselectRowFn) { _didDeselectRowFn = didDeselectRowFn; }
    void setDidHoverRowFn(DidHoverRowFnType didHoverRowFn) { _didHoverRowFn = didHoverRowFn; }
    void setDidConfirmRowSelectionFn(DidConfirmRowSelectionFnType didConfirmRowSelectionFn) {
        _didConfirmRowSelectionFn = didConfirmRowSelectionFn;
    }
    void setDidSetFocusStateFn(DidSetFocusStateFnType didSetFocusStateFn) { _didSetFocusStateFn = didSetFocusStateFn; }
    void setDidPressUnhandledKeyFn(DidPressUnhandledKeyFnType didPressUnhandledKey) {
        _didPressUnhandledKeyFn = didPressUnhandledKey;
    }

    void reload();
    void reloadWindow(bool isDelayed, bool isRefreshOnly);

    std::shared_ptr<View> viewAtRow(int row);
    int nbRows() { return _nbRows; }

    float contentHeight();
    Rect viewRectAtRow(int row);

    void setSelectedRow(int row, bool isDelegateNotified = true, bool isExclusive = true);
    int selectedRow() { return _selectedRow; }
    std::vector<int> selectedRows() { return _selectedRows; }

    void setFrame(Rect rect) override;
    void didResolveClippedRect() override;

    void setIsPassThrough(bool isPassThrough) { _isPassThrough = isPassThrough; }
    bool isPassThrough() { return _isPassThrough; }

    void setIsStatic(bool isStatic) { _isStatic = isStatic; }
    bool isStatic() { return _isStatic; }

    void setIsCapturing(bool isCapturing) { _isCapturing = isCapturing; }
    bool isCapturing() { return _isCapturing; }

    bool hasFocus() { return _hasFocus; }

    void captureFocus();
    void releaseFocus();

    void resignFirstResponder() override;
};

}  // namespace MDStudio

#endif  // LISTVIEW_H
