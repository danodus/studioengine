//
//  pianorolleventslistview.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-07-26.
//  Copyright © 2016-2021 Daniel Cliche. All rights reserved.
//

#include "pianorolleventslistview.h"

#include <platform.h>

#include <algorithm>

// ---------------------------------------------------------------------------------------------------------------------
PianoRollEventsListView::PianoRollEventsListView(std::string name, void* owner) : MDStudio::View(name, owner) {
    _ui.loadUI(this, MDStudio::Platform::sharedInstance()->resourcesPath() + "/PianoRollEventsListView.lua");

    using namespace std::placeholders;

    _nbEventsFn = nullptr;
    _eventAtIndexFn = nullptr;
    _didSelectEventsFn = nullptr;

    _trackIndex = -1;
    _isDelegateNotified = true;

    // Create table view
    _tableView = std::shared_ptr<MDStudio::TableView>(
        new MDStudio::TableView("eventsTableView", owner, 14.0f, true, false, true));
    _tableView->setNbColumnsFn(std::bind(&PianoRollEventsListView::tableViewNbColumns, this, _1));
    _tableView->setColumnAtIndexFn(std::bind(&PianoRollEventsListView::tableViewColumnAtIndex, this, _1, _2));
    _tableView->setDidResizeColumnFn(std::bind(&PianoRollEventsListView::tableViewDidResizeColumn, this, _1, _2, _3));
    _tableView->setNbRowsFn(std::bind(&PianoRollEventsListView::tableViewNbRows, this, _1));
    _tableView->setViewForRowFn(std::bind(&PianoRollEventsListView::tableViewViewForRow, this, _1, _2));
    _tableView->setDidSelectRowFn(std::bind(&PianoRollEventsListView::tableViewDidSelectRow, this, _1, _2));
    _tableView->setDidDeselectRowFn(std::bind(&PianoRollEventsListView::tableViewDidDeselectRow, this, _1, _2));
    _tableView->setDidSetFocusStateFn(std::bind(&PianoRollEventsListView::tableViewDidSetFocusState, this, _1, _2));

    // Set it pass though in order to allow the subviews of the selected row to receive events
    _tableView->setIsPassThrough(true);

    auto col1 = std::make_shared<MDStudio::Column>(_ui.findString("typeStr"), 40.0f, true, true, false,
                                                   MDStudio::Column::SortDirection::Descending);
    col1->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    auto col2 = std::make_shared<MDStudio::Column>(_ui.findString("tickStr"), 40.0f, true, true, false,
                                                   MDStudio::Column::SortDirection::Descending);
    col2->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    auto col3 = std::make_shared<MDStudio::Column>(_ui.findString("lenStr"), 40.0f, true, true, false,
                                                   MDStudio::Column::SortDirection::Descending);
    col3->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    auto col4 = std::make_shared<MDStudio::Column>(_ui.findString("channelStr"), 40.0f, true, true, false,
                                                   MDStudio::Column::SortDirection::Descending);
    col4->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    auto col5 = std::make_shared<MDStudio::Column>(_ui.findString("param1Str"), 80.0f, true, true, false,
                                                   MDStudio::Column::SortDirection::Descending);
    col5->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    auto col6 = std::make_shared<MDStudio::Column>(_ui.findString("param2Str"), 80.0f, true, true, false,
                                                   MDStudio::Column::SortDirection::Descending);
    col6->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    auto col7 = std::make_shared<MDStudio::Column>(_ui.findString("param3Str"), 80.0f, true, true, false,
                                                   MDStudio::Column::SortDirection::Descending);
    col7->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());

    _columns = {col1, col2, col3, col4, col5, col6, col7};

    addSubview(_tableView);
}

// ---------------------------------------------------------------------------------------------------------------------
PianoRollEventsListView::~PianoRollEventsListView() {}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsListView::setFrame(MDStudio::Rect aRect) {
    View::setFrame(aRect);

    _tableView->setFrame(bounds());
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsListView::reload() { _tableView->reload(); }

// ---------------------------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<MelobaseCore::Event>> PianoRollEventsListView::selectedEvents() const {
    std::vector<std::shared_ptr<MelobaseCore::Event>> selectedEvents;
    auto selectedRows = _tableView->selectedRows();
    for (int r : selectedRows) {
        std::shared_ptr<MelobaseCore::Event> event;
        bool isChannelEditionAvail = false;
        _eventAtIndexFn(this, _trackIndex, r, &event, &isChannelEditionAvail);
        selectedEvents.push_back(event);
    }
    return selectedEvents;
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int PianoRollEventsListView::tableViewNbColumns(MDStudio::TableView* sender) {
    return static_cast<unsigned int>(_columns.size());
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MDStudio::Column> PianoRollEventsListView::tableViewColumnAtIndex(MDStudio::TableView* sender,
                                                                                  int index) {
    return _columns.at(index);
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int PianoRollEventsListView::tableViewNbRows(MDStudio::TableView* sender) {
    return _nbEventsFn(this, _trackIndex);
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MDStudio::View> PianoRollEventsListView::tableViewViewForRow(MDStudio::TableView* sender, int row) {
    using namespace std::placeholders;

    std::shared_ptr<MelobaseCore::Event> event;
    bool isChannelEditionAvail = false;
    _eventAtIndexFn(this, _trackIndex, row, &event, &isChannelEditionAvail);

    std::vector<float> columnWidths;
    for (auto column : _columns) columnWidths.push_back(column->width());

    auto listItemView =
        std::make_shared<EventListItemView>("eventListItemView", this, columnWidths, event, isChannelEditionAvail);
    listItemView->setFocusState(sender->hasFocus());

    auto selectedRows = sender->selectedRows();
    if (std::find(selectedRows.begin(), selectedRows.end(), row) != selectedRows.end()) {
        listItemView->setIsHighlighted(true);
        listItemView->setEventDidChangeFn(
            std::bind(&PianoRollEventsListView::eventListItemViewEventDidChange, this, _1, _2, _3, _4, _5, _6, _7, _8));
    }

    return listItemView;
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsListView::tableViewDidResizeColumn(MDStudio::TableView* sender, int columnIndex, float width) {
    std::vector<float> columnWidths;
    for (auto column : _columns) columnWidths.push_back(column->width());

    for (auto rowView : sender->visibleRowViews()) {
        auto listItemView = std::dynamic_pointer_cast<EventListItemView>(rowView);
        listItemView->setColumnWidths(columnWidths);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsListView::tableViewDidSelectRow(MDStudio::TableView* sender, int row) {
    using namespace std::placeholders;

    auto view = sender->viewAtRow(row);
    if (view) {
        auto eventListItemView = std::dynamic_pointer_cast<EventListItemView>(view);
        eventListItemView->setIsHighlighted(true);

        eventListItemView->setEventDidChangeFn(
            std::bind(&PianoRollEventsListView::eventListItemViewEventDidChange, this, _1, _2, _3, _4, _5, _6, _7, _8));
    }

    _tableView->scrollToVisibleRectV(sender->viewRectAtRow(row));

    if (_isDelegateNotified && _didSelectEventsFn) _didSelectEventsFn(this, selectedEvents());
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsListView::tableViewDidDeselectRow(MDStudio::TableView* sender, int row) {
    auto view = sender->viewAtRow(row);
    if (view) {
        auto eventListItemView = std::dynamic_pointer_cast<EventListItemView>(view);
        eventListItemView->setIsHighlighted(false);
        eventListItemView->setEventDidChangeFn(nullptr);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsListView::tableViewDidSetFocusState(MDStudio::TableView* sender, bool state) {
    for (auto view : sender->visibleRowViews()) {
        auto eventListItemView = std::dynamic_pointer_cast<EventListItemView>(view);
        eventListItemView->setFocusState(state);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsListView::selectEvents(std::vector<std::shared_ptr<MelobaseCore::Event>> events,
                                           bool isDelegateNotified) {
    std::vector<int> indices;

    int nbEvents = _nbEventsFn(this, _trackIndex);
    for (int i = 0; i < nbEvents; ++i) {
        std::shared_ptr<MelobaseCore::Event> event;
        bool isChannelEditionAvail = false;
        _eventAtIndexFn(this, _trackIndex, i, &event, &isChannelEditionAvail);
        if (std::find(events.begin(), events.end(), event) != events.end()) indices.push_back(i);
    }

    _isDelegateNotified = false;

    _tableView->setSelectedRow(-1, true);

    for (auto i : indices) _tableView->setSelectedRow(i, true, false);

    _isDelegateNotified = true;

    if (isDelegateNotified && _didSelectEventsFn) _didSelectEventsFn(this, selectedEvents());
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollEventsListView::eventListItemViewEventDidChange(EventListItemView* sender, UInt32 tickCount,
                                                              UInt32 length, UInt8 channel, SInt32 param1,
                                                              SInt32 param2, SInt32 param3, std::vector<UInt8> data) {
    // The sender is an item view therefore we must invoke
    auto event = sender->event();
    MDStudio::Platform::sharedInstance()->invoke(
        [=] { _eventDidChangeFn(this, event, tickCount, length, channel, param1, param2, param3, data); });
}
