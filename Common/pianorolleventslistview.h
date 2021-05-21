//
//  pianorolleventslistview.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-07-26.
//  Copyright © 2016-2019 Daniel Cliche. All rights reserved.
//

#ifndef PIANOROLLEVENTSLISTVIEW_H
#define PIANOROLLEVENTSLISTVIEW_H

#include <view.h>
#include <labelview.h>
#include <scrollview.h>
#include <tableview.h>
#include <melobasecore_event.h>
#include <ui.h>

#include "eventlistitemview.h"

class PianoRollEventsListView : public MDStudio::View {
    
public:
    typedef std::function<unsigned int(const PianoRollEventsListView *sender, int track)> NbEventsFnType;
    typedef std::function<void(const PianoRollEventsListView *sender, int track, int index, std::shared_ptr<MelobaseCore::Event> *event, bool *isChannelEditionAvail)> EventAtIndexFnType;
    typedef std::function<void(PianoRollEventsListView *sender, std::vector<std::shared_ptr<MelobaseCore::Event>> events)> DidSelectEventsFnType;
    typedef std::function<void(PianoRollEventsListView *sender, std::shared_ptr<MelobaseCore::Event> event, UInt32 tickCount, UInt32 length, UInt8 channel, SInt32 param1, SInt32 param2, SInt32 param3, std::vector<UInt8> data)> EventDidChangeFnType;
    
private:
    
    MDStudio::UI _ui;
    
    std::shared_ptr<MDStudio::TableView> _tableView;
    
    NbEventsFnType _nbEventsFn;
    EventAtIndexFnType _eventAtIndexFn;
    DidSelectEventsFnType _didSelectEventsFn;
    EventDidChangeFnType _eventDidChangeFn;
    
    int _trackIndex;
    bool _isDelegateNotified;
    
    std::vector<std::shared_ptr<MDStudio::Column>> _columns;
    
    unsigned int tableViewNbColumns(MDStudio::TableView *sender);
    std::shared_ptr<MDStudio::Column> tableViewColumnAtIndex(MDStudio::TableView *sender, int index);
    void tableViewDidResizeColumn(MDStudio::TableView *sender, int columnIndex, float width);
    unsigned int tableViewNbRows(MDStudio::TableView *sender);
    std::shared_ptr<MDStudio::View> tableViewViewForRow(MDStudio::TableView *sender, int row);
    void tableViewDidSelectRow(MDStudio::TableView *sender, int row);
    void tableViewDidDeselectRow(MDStudio::TableView *sender, int row);
    void tableViewDidSetFocusState(MDStudio::TableView *sender, bool state);
    
    void eventListItemViewEventDidChange(EventListItemView *sender, UInt32 tickCount, UInt32 length, UInt8 channel, SInt32 param1, SInt32 param2, SInt32 param3, std::vector<UInt8> data);

    std::vector<std::shared_ptr<MelobaseCore::Event>> selectedEvents() const;

public:
    
    PianoRollEventsListView(std::string name, void *owner);
    ~PianoRollEventsListView();
    
    void setFrame(MDStudio::Rect aRect) override;
    
    void setTrackIndex(int trackIndex) { _trackIndex = trackIndex; }
    void reload();
    
    void selectEvents(std::vector<std::shared_ptr<MelobaseCore::Event>> events, bool isDelegateNotified = true);
    
    std::shared_ptr<MDStudio::TableView> tableView() { return _tableView; }
    
    void setNbEventsFn(NbEventsFnType nbEventsFn) { _nbEventsFn = nbEventsFn; }
    void setEventAtIndexFn(EventAtIndexFnType eventAtIndexFn) { _eventAtIndexFn = eventAtIndexFn; }
    void setDidSelectEventsFn(DidSelectEventsFnType didSelectEventsFn) { _didSelectEventsFn = didSelectEventsFn; }
    void setEventDidChangeFn(EventDidChangeFnType eventDidChangeFn) { _eventDidChangeFn = eventDidChangeFn; }
};

#endif // PIANOROLLEVENTSLISTVIEW_H
