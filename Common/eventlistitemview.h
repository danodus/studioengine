//
//  eventlistitemview.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-07-27.
//  Copyright © 2016 Daniel Cliche. All rights reserved.
//

#ifndef EVENTLISTITEMVIEW_H
#define EVENTLISTITEMVIEW_H

#include <view.h>
#include <boxview.h>
#include <labelview.h>
#include <textfield.h>
#include <melobasecore_event.h>

class EventListItemView : public MDStudio::View {
    
public:
    
    typedef std::function<void(EventListItemView *sender, UInt32 tickCount, UInt32 length, UInt8 channel, SInt32 param1, SInt32 param2, SInt32 param3, std::vector<UInt8> data)> EventDidChangeFnType;
    
private:
    
    bool _isHighlighted;
    bool _hasFocus;
    
    std::shared_ptr<MDStudio::BoxView> _boxView;
    std::shared_ptr<MDStudio::LabelView> _typeLabelView;
    std::shared_ptr<MDStudio::TextField> _tickTextField;
    std::shared_ptr<MDStudio::TextField> _lengthTextField;
    std::shared_ptr<MDStudio::TextField> _channelTextField;
    std::shared_ptr<MDStudio::LabelView> _p1LabelView;
    std::shared_ptr<MDStudio::TextField> _p1TextField;
    std::shared_ptr<MDStudio::LabelView> _p2LabelView;
    std::shared_ptr<MDStudio::TextField> _p2TextField;
    std::shared_ptr<MDStudio::LabelView> _p3LabelView;
    std::shared_ptr<MDStudio::TextField> _p3TextField;
    
    std::shared_ptr<MelobaseCore::ChannelEvent> _channelEvent;
    
    std::vector<float> _columnWidths;
    
    EventDidChangeFnType _eventDidChangeFn;
    
    void textFieldTextDidChange(MDStudio::TextField *sender, std::string text);

public:
    
    void setFrame(MDStudio::Rect rect) override;
    
    EventListItemView(std::string name, void *owner, std::vector<float> columnWidths,  std::shared_ptr<MelobaseCore::Event> event, bool isChannelEditionAvail);
    ~EventListItemView();
    
    std::shared_ptr<MelobaseCore::Event> event() { return _channelEvent; }
    
    void setColumnWidths(std::vector<float> columnWidths) { _columnWidths = columnWidths; setFrame(frame()); }
    void setIsHighlighted(bool isHighlighted);
    void setFocusState(bool focusState);
    
    void setEventDidChangeFn(EventDidChangeFnType eventDidChangeFn) { _eventDidChangeFn = eventDidChangeFn; }
};

#endif // EVENTLISTITEMVIEW_H

