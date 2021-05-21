//
//  tableview.h
//  MDStudio
//
//  Created by Daniel Cliche on 2017-02-13.
//  Copyright (c) 2017-2021 Daniel Cliche. All rights reserved.
//

#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include "boxview.h"
#include "button.h"
#include "column.h"
#include "control.h"
#include "labelview.h"
#include "listview.h"
#include "scrollview.h"
#include "splitviewmultih.h"

namespace MDStudio {

class TableViewHeaderColumnView : public View {
   public:
    typedef std::function<void(TableViewHeaderColumnView* sender, bool state)> DidSetSortStateFnType;

   private:
    std::shared_ptr<Button> _sortButton;

    DidSetSortStateFnType _didSetSortStateFn;

    void sortButtonClicked(MDStudio::Button* sender);

   public:
    TableViewHeaderColumnView(const std::string& name, void* owner, std::string title, MultiDPIFont* font,
                              bool isSortAvailable, bool isSortEnabled, bool sortState);
    ~TableViewHeaderColumnView();
    void setFrame(Rect rect) override;

    void setDidSetSortStateFn(DidSetSortStateFnType didSetSortStateFn) { _didSetSortStateFn = didSetSortStateFn; }
};

class TableViewHeaderView : public View {
   public:
    typedef std::function<void(TableViewHeaderView* sender, int columnIndex, bool state)> DidSetSortStateFnType;
    typedef std::function<void(TableViewHeaderView* sender, int columnIndex, float width)> DidResizeColumnFnType;

   private:
    std::vector<std::shared_ptr<Column>> _columns;
    std::shared_ptr<BoxView> _boxView;
    std::vector<std::shared_ptr<TableViewHeaderColumnView>> _columnViews;
    std::shared_ptr<SplitViewMultiH> _splitViewMultiH;

    DidSetSortStateFnType _didSetSortStateFn;
    DidResizeColumnFnType _didResizeColumnFn;

    void didSetSortState(TableViewHeaderColumnView* sender, bool state);
    void splitViewMultiHPosChanged(SplitViewMultiH* sender, std::vector<std::pair<float, bool>> pos);

   public:
    TableViewHeaderView(std::string name, void* owner);
    ~TableViewHeaderView();

    void setColumns(std::vector<std::shared_ptr<Column>> columns);
    void setFrame(Rect rect) override;

    void setDidSetSortStateFn(DidSetSortStateFnType didSetSortStateFn) { _didSetSortStateFn = didSetSortStateFn; }
    void setDidResizeColumnFn(DidResizeColumnFnType didResizeColumnFn) { _didResizeColumnFn = didResizeColumnFn; }
};

class TableView : public Control {
   public:
    typedef std::function<unsigned int(TableView* sender)> NbColumnsFnType;
    typedef std::function<std::shared_ptr<Column>(TableView* sender, unsigned int index)> ColumnAtIndexFnType;

    typedef std::function<unsigned int(TableView* sender)> NbRowsFnType;
    typedef std::function<std::shared_ptr<View>(TableView* sender, int row)> ViewForRowFnType;

    typedef std::function<void(TableView* sender, int row)> DidSelectRowFnType;
    typedef std::function<void(TableView* sender, int row)> DidDeselectRowFnType;
    typedef std::function<void(TableView* sender, int row)> DidHoverRowFnType;
    typedef std::function<void(TableView* sender, int row)> DidConfirmRowSelectionFnType;
    typedef std::function<void(TableView* sender, bool state)> DidSetFocusStateFnType;
    typedef std::function<bool(TableView* sender, unsigned int key)> DidPressUnhandledKeyFnType;

    typedef std::function<void(TableView* sender, int columnIndex, bool state)> DidSetSortStateFnType;
    typedef std::function<void(TableView* sender, int columnIndex, float width)> DidResizeColumnFnType;

   private:
    std::shared_ptr<TableViewHeaderView> _headerView;
    std::shared_ptr<ListView> _listView;
    std::shared_ptr<ScrollView> _scrollView;

    std::vector<std::shared_ptr<Column>> _columns;
    float _headerHeight;
    bool _isWindowed;

    // Data source
    NbColumnsFnType _nbColumnsFn;
    ColumnAtIndexFnType _columnAtIndexFn;
    NbRowsFnType _nbRowsFn;
    ViewForRowFnType _viewForRowFn;

    DidSelectRowFnType _didSelectRowFn;
    DidDeselectRowFnType _didDeselectRowFn;
    DidHoverRowFnType _didHoverRowFn;
    DidConfirmRowSelectionFnType _didConfirmRowSelectionFn;
    DidSetFocusStateFnType _didSetFocusStateFn;
    DidPressUnhandledKeyFnType _didPressUnhandledKeyFn;

    DidSetSortStateFnType _didSetSortStateFn;
    DidResizeColumnFnType _didResizeColumnFn;

    Size contentSize();

    unsigned int listViewNbRows(ListView* sender);
    std::shared_ptr<View> listViewViewForRow(ListView* sender, int row);
    void listViewDidSelectRow(ListView* sender, int row);
    void listViewDidDeselectRow(ListView* sender, int row);
    void listViewDidHoverRow(ListView* sender, int row);
    void listViewDidConfirmRowSelection(ListView* sender, int row);
    void listViewDidSetFocusState(ListView* sender, bool state);
    bool listViewDidPressUnhandledKey(ListView* sender, unsigned int key);

    void scrollViewPosChanged(MDStudio::ScrollView* sender, MDStudio::Point pos);

    void tableHeaderViewDidSetSortState(TableViewHeaderView* sender, int columnIndex, bool state);
    void tableHeaderViewDidResizeColumn(TableViewHeaderView* sender, int columnIndex, float width);

   public:
    TableView(std::string name, void* owner, float rowHeight, bool isMultipleSelectionsAllowed = false,
              bool isRowSelectionConfirmationAvailable = true, bool isWindowed = false);
    ~TableView();

    void setFrame(Rect rect) override;

    void reload();

    void setIsPassThrough(bool isPassThrough) { _listView->setIsPassThrough(isPassThrough); }
    bool isPassThrough() { return _listView->isPassThrough(); }

    void captureFocus() { _listView->captureFocus(); }
    bool hasFocus() { return _listView->hasFocus(); }

    void setSelectedRow(int row, bool isDelegateNotified = true, bool isExclusive = true) {
        _listView->setSelectedRow(row, isDelegateNotified, isExclusive);
    }

    int selectedRow() { return _listView->selectedRow(); }
    std::vector<int> selectedRows() { return _listView->selectedRows(); }

    void scrollToVisibleRectV(Rect rect) { _scrollView->scrollToVisibleRectV(rect); }

    const std::vector<std::shared_ptr<View>>& visibleRowViews() { return _listView->subviews(); }
    std::shared_ptr<View> viewAtRow(int row) { return _listView->viewAtRow(row); }
    int nbRows() { return _listView->nbRows(); }

    void setPosInvY(Point posInvY) { _scrollView->setPosInvY(posInvY); }
    Point posInvY() { return _scrollView->posInvY(); }

    Rect viewRectAtRow(int row);

    // Data source
    void setNbColumnsFn(NbColumnsFnType nbColumnsFn) { _nbColumnsFn = nbColumnsFn; }
    void setColumnAtIndexFn(ColumnAtIndexFnType columnAtIndexFn) { _columnAtIndexFn = columnAtIndexFn; }
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

    void setDidSetSortStateFn(DidSetSortStateFnType didSetSortStateFn) { _didSetSortStateFn = didSetSortStateFn; }
    void setDidResizeColumnFn(DidResizeColumnFnType didResizeColumnFn) { _didResizeColumnFn = didResizeColumnFn; }
};
}  // namespace MDStudio

#endif  // TABLEVIEW_H
