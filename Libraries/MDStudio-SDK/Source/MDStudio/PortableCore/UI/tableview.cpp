//
//  tableview.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2017-02-13.
//  Copyright (c) 2017-2021 Daniel Cliche. All rights reserved.
//

#include "tableview.h"

#include <platform.h>

#include <algorithm>
#include <cassert>

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
TableViewHeaderColumnView::TableViewHeaderColumnView(const std::string& name, void* owner, std::string title,
                                                     MultiDPIFont* font, bool isSortAvailable, bool isSortEnabled,
                                                     bool sortState)
    : View(name, owner) {
    using namespace std::placeholders;

    _didSetSortStateFn = nullptr;

    _sortButton = std::make_shared<Button>("sortButton", this, title);
    _sortButton->setFont(font);
    _sortButton->setType(Button::SortButtonType);
    _sortButton->setState(sortState);
    _sortButton->setIsEnabled(isSortAvailable);
    _sortButton->setIsSortEnabled(isSortEnabled);
    addSubview(_sortButton);

    _sortButton->setClickedFn(std::bind(&TableViewHeaderColumnView::sortButtonClicked, this, _1));
}

// ---------------------------------------------------------------------------------------------------------------------
TableViewHeaderColumnView::~TableViewHeaderColumnView() { removeSubview(_sortButton); }

// ---------------------------------------------------------------------------------------------------------------------
void TableViewHeaderColumnView::setFrame(Rect rect) {
    View::setFrame(rect);

    _sortButton->setFrame(bounds());
}

// ---------------------------------------------------------------------------------------------------------------------
void TableViewHeaderColumnView::sortButtonClicked(MDStudio::Button* sender) {
    bool state = sender->state();
    if (sender->isSortEnabled()) {
        state = !state;
        sender->setState(state, false);
    }

    if (_didSetSortStateFn) _didSetSortStateFn(this, state);
}

// ---------------------------------------------------------------------------------------------------------------------
TableViewHeaderView::TableViewHeaderView(std::string name, void* owner) : View(name, owner) {
    _didSetSortStateFn = nullptr;
    _didResizeColumnFn = nullptr;

    _boxView = std::make_shared<BoxView>("tableViewHeaderBoxView", this);
    _boxView->setFillColor(grayColor);
    _boxView->setBorderColor(grayColor);
    addSubview(_boxView);
}

// ---------------------------------------------------------------------------------------------------------------------
TableViewHeaderView::~TableViewHeaderView() {
    if (_splitViewMultiH) removeSubview(_splitViewMultiH);
    removeSubview(_boxView);
}

// ---------------------------------------------------------------------------------------------------------------------
void TableViewHeaderView::setColumns(std::vector<std::shared_ptr<Column>> columns) {
    using namespace std::placeholders;

    if (_splitViewMultiH) {
        removeSubview(_splitViewMultiH);
        _splitViewMultiH = nullptr;
    }

    _columnViews.clear();

    _columns = columns;

    std::vector<std::pair<float, bool>> pos;
    std::vector<std::shared_ptr<View>> panes;

    int columnIndex = 0;
    for (auto column : columns) {
        auto columnView = std::make_shared<TableViewHeaderColumnView>(
            "tableViewHeaderColumnView" + std::to_string(columnIndex), this, column->title(), column->font(),
            column->isSortAvailable(), column->isSortEnabled(),
            column->sortDirection() == Column::Ascending ? true : false);
        columnView->setDidSetSortStateFn(std::bind(&TableViewHeaderView::didSetSortState, this, _1, _2));
        _columnViews.push_back(columnView);
        pos.push_back(std::make_pair(column->width(), column->isResizable()));
        panes.push_back(columnView);
        ++columnIndex;
    }

    _splitViewMultiH = std::make_shared<SplitViewMultiH>("tableViewHeaderSplitViewMultiH", this, panes, pos);
    _splitViewMultiH->setPosChangedFn(std::bind(&TableViewHeaderView::splitViewMultiHPosChanged, this, _1, _2));
    addSubview(_splitViewMultiH);

    setFrame(frame());
}

// ---------------------------------------------------------------------------------------------------------------------
void TableViewHeaderView::setFrame(Rect rect) {
    View::setFrame(rect);

    float totalColumnsWidth = 0.0f;
    for (auto column : _columns) totalColumnsWidth += column->width();

    auto r = makeRect(0.0f, 0.0f, totalColumnsWidth + SPLIT_VIEW_MULTI_H_SPLITTER_WIDTH / 2.0f, rect.size.height);

    _boxView->setFrame(r);
    if (_splitViewMultiH) _splitViewMultiH->setFrame(r);
}

// ---------------------------------------------------------------------------------------------------------------------
void TableViewHeaderView::didSetSortState(TableViewHeaderColumnView* sender, bool state) {
    if (_didSetSortStateFn) {
        // Find the index of the column
        auto it = std::find_if(
            _columnViews.begin(), _columnViews.end(),
            [sender](std::shared_ptr<TableViewHeaderColumnView> columnView) { return sender == columnView.get(); });
        assert(it != _columnViews.end());
        size_t index = it - _columnViews.begin();
        MDStudio::Platform::sharedInstance()->invoke([=] { _didSetSortStateFn(this, (int)index, state); });
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TableViewHeaderView::splitViewMultiHPosChanged(SplitViewMultiH* sender, std::vector<std::pair<float, bool>> pos) {
    int columnIndex = 0;
    for (auto column : _columns) {
        auto width = pos.at(static_cast<size_t>(columnIndex)).first;
        if (column->width() != width) {
            column->setWidth(width);
            if (_didResizeColumnFn) {
                _didResizeColumnFn(this, columnIndex, width);
            }
        }
        ++columnIndex;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
TableView::TableView(std::string name, void* owner, float rowHeight, bool isMultipleSelectionsAllowed,
                     bool isRowSelectionConfirmationAvailable, bool isWindowed)
    : _isWindowed(isWindowed), Control(name, owner) {
    using namespace std::placeholders;

    _nbColumnsFn = nullptr;
    _columnAtIndexFn = nullptr;
    _nbRowsFn = nullptr;
    _viewForRowFn = nullptr;

    _didSelectRowFn = nullptr;
    _didDeselectRowFn = nullptr;
    _didHoverRowFn = nullptr;
    _didConfirmRowSelectionFn = nullptr;
    _didSetFocusStateFn = nullptr;
    _didPressUnhandledKeyFn = nullptr;

    _didSetSortStateFn = nullptr;
    _didResizeColumnFn = nullptr;

    _headerHeight = rowHeight;

    _headerView = std::make_shared<TableViewHeaderView>("HeaderView", this);
    _headerView->setDidSetSortStateFn(std::bind(&TableView::tableHeaderViewDidSetSortState, this, _1, _2, _3));
    _headerView->setDidResizeColumnFn(std::bind(&TableView::tableHeaderViewDidResizeColumn, this, _1, _2, _3));

    _listView = std::make_shared<ListView>("ListView", this, rowHeight, isMultipleSelectionsAllowed);

    _listView->setNbRowsFn(std::bind(&TableView::listViewNbRows, this, _1));
    _listView->setViewForRowFn(std::bind(&TableView::listViewViewForRow, this, _1, _2));
    _listView->setDidSelectRowFn(std::bind(&TableView::listViewDidSelectRow, this, _1, _2));
    _listView->setDidDeselectRowFn(std::bind(&TableView::listViewDidDeselectRow, this, _1, _2));
    _listView->setDidHoverRowFn(std::bind(&TableView::listViewDidHoverRow, this, _1, _2));
    if (isRowSelectionConfirmationAvailable)
        _listView->setDidConfirmRowSelectionFn(std::bind(&TableView::listViewDidConfirmRowSelection, this, _1, _2));
    _listView->setDidSetFocusStateFn(std::bind(&TableView::listViewDidSetFocusState, this, _1, _2));
    _listView->setDidPressUnhandledKeyFn(std::bind(&TableView::listViewDidPressUnhandledKey, this, _1, _2));

    _scrollView = std::shared_ptr<ScrollView>(new ScrollView("ScrollView", owner, _listView, true));

    addSubview(_headerView);
    addSubview(_scrollView);
}

// ---------------------------------------------------------------------------------------------------------------------
TableView::~TableView() {
    removeSubview(_headerView);
    removeSubview(_scrollView);
}

// ---------------------------------------------------------------------------------------------------------------------
void TableView::reload() {
    using namespace std::placeholders;

    // Request the list of columns

    if (!_nbColumnsFn || !_columnAtIndexFn) return;

    _columns.clear();

    float totalColumnsWidth = 0.0f;
    unsigned int nbColumns = _nbColumnsFn(this);

    for (unsigned int columnIndex = 0; columnIndex < nbColumns; ++columnIndex) {
        auto column = _columnAtIndexFn(this, columnIndex);
        _columns.push_back(column);
        totalColumnsWidth += column->width();
    }

    _headerView->setColumns(_columns);

    _listView->setSelectedRow(-1);

    if (_isWindowed) {
        _listView->reloadWindow(true, false);
    } else {
        _listView->reload();
    }

    _scrollView->setContentSize(MDStudio::makeSize(totalColumnsWidth, _listView->contentHeight()));

    _scrollView->setPosChangedFn(std::bind(&TableView::scrollViewPosChanged, this, _1, _2));
}

// ---------------------------------------------------------------------------------------------------------------------
Size TableView::contentSize() {
    float totalWidth = 0.0f;
    for (auto column : _columns) totalWidth += column->width();

    return makeSize(totalWidth, _listView->contentHeight());
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int TableView::listViewNbRows(ListView* sender) {
    if (_nbRowsFn) return _nbRowsFn(this);
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<View> TableView::listViewViewForRow(ListView* sender, int row) {
    if (_viewForRowFn) return _viewForRowFn(this, row);
    return nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------
void TableView::listViewDidSelectRow(ListView* sender, int row) {
    if (_didSelectRowFn) _didSelectRowFn(this, row);
}

// ---------------------------------------------------------------------------------------------------------------------
void TableView::listViewDidDeselectRow(ListView* sender, int row) {
    if (_didDeselectRowFn) _didDeselectRowFn(this, row);
}

// ---------------------------------------------------------------------------------------------------------------------
void TableView::listViewDidHoverRow(ListView* sender, int row) {
    if (_didHoverRowFn) _didHoverRowFn(this, row);
}

// ---------------------------------------------------------------------------------------------------------------------
void TableView::listViewDidConfirmRowSelection(ListView* sender, int row) {
    if (_didConfirmRowSelectionFn) _didConfirmRowSelectionFn(this, row);
}

// ---------------------------------------------------------------------------------------------------------------------
void TableView::listViewDidSetFocusState(ListView* sender, bool state) {
    if (_didSetFocusStateFn) _didSetFocusStateFn(this, state);
}

// ---------------------------------------------------------------------------------------------------------------------
bool TableView::listViewDidPressUnhandledKey(ListView* sender, unsigned int key) {
    if (_didPressUnhandledKeyFn) return _didPressUnhandledKeyFn(this, key);
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
void TableView::scrollViewPosChanged(MDStudio::ScrollView* sender, MDStudio::Point pos) {
    // Set the horizontal offset for the list header
    MDStudio::Point offset = _headerView->offset();
    offset.x = pos.x;
    _headerView->setOffset(offset);

    if (_isWindowed) _listView->reloadWindow(false, true);
}

// ---------------------------------------------------------------------------------------------------------------------
void TableView::tableHeaderViewDidSetSortState(TableViewHeaderView* sender, int columnIndex, bool state) {
    if (_didSetSortStateFn) _didSetSortStateFn(this, columnIndex, state);
}

// ---------------------------------------------------------------------------------------------------------------------
void TableView::tableHeaderViewDidResizeColumn(TableViewHeaderView* sender, int columnIndex, float width) {
    _columns.at(columnIndex)->setWidth(width);
    _headerView->setFrame(_headerView->frame());
    _scrollView->setContentSize(contentSize());

    if (_didResizeColumnFn) _didResizeColumnFn(this, columnIndex, width);
}

// ---------------------------------------------------------------------------------------------------------------------
void TableView::setFrame(Rect aRect) {
    Control::setFrame(aRect);

    Rect r = makeRect(0.0f, bounds().size.height - _headerHeight, bounds().size.width, _headerHeight);
    _headerView->setFrame(r);

    r.size.height = bounds().size.height - _headerHeight;
    r.origin.y = 0.0f;

    _scrollView->setFrame(r);
}

// ---------------------------------------------------------------------------------------------------------------------
Rect TableView::viewRectAtRow(int row)
{
    return _listView->viewRectAtRow(row);
}
