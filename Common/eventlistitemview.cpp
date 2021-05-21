//
//  eventlistitemview.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-07-27.
//  Copyright © 2016-2019 Daniel Cliche. All rights reserved.
//

#include "eventlistitemview.h"

#include <math.h>
#include <algorithm>
#include <cctype>

#include "helpers.h"

static const char *eventTypeStrings[][4] = {
    {"-", "", "", ""},
    {"NOTE", "Pitch: ", "Vel.On: ", "Vel.Off: "},
    {"-", "", "", ""},
    {"PC", "Prog.: ", "", ""},
    {"VOL", "Val.: ", "", ""},
    {"PAN", "Val.: ", "", ""},
    {"S", "Val.: ", "", ""},
    {"PB", "Val.: ", "", ""},
    {"M", "Val.: ", "", ""},
    {"KAT", "Pitch: ", "Val.: ", ""},
    {"CAT", "Val.: ", "", ""},
    {"CC", "Ctrl.:", "Val.: ", ""},
    {"SYSEX", "Data(H):", "", ""}
    
};

static const char *metaEventTypeStrings[][4] = {
    {"T", "Val.: ", "", ""},
    {"TS", "Num.: ", "Den.: ", ""},
    {"EOT", "", "", ""}
};

// ---------------------------------------------------------------------------------------------------------------------
EventListItemView::EventListItemView(std::string name, void *owner, std::vector<float> columnWidths, std::shared_ptr<MelobaseCore::Event> event, bool isChannelEditionAvail) : _columnWidths(columnWidths), MDStudio::View(name, owner)
{
    using namespace std::placeholders;
    
    _eventDidChangeFn = nullptr;
    
    _isHighlighted = false;
    _hasFocus = false;
    
    _channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(event);
    
    _boxView = std::shared_ptr<MDStudio::BoxView>(new MDStudio::BoxView("boxView", owner));
    _boxView->setBorderColor(MDStudio::blackColor);
    addSubview(_boxView);
    std::string eventTypeString = "?";
    std::string p1String = "";
    std::string p2String = "";
    std::string p3String = "";
    
    if (_channelEvent->type() <= 12) {
        eventTypeString = eventTypeStrings[_channelEvent->type()][0];
        p1String = eventTypeStrings[_channelEvent->type()][1];
        p2String = eventTypeStrings[_channelEvent->type()][2];
        p3String = eventTypeStrings[_channelEvent->type()][3];
    } else if (_channelEvent->type() >= 0xF0 && _channelEvent->type() <= 0xF2) {
        eventTypeString = metaEventTypeStrings[_channelEvent->type() & 0xF][0];
        p1String = metaEventTypeStrings[_channelEvent->type() & 0xF][1];
        p2String = metaEventTypeStrings[_channelEvent->type() & 0xF][2];
        p3String = metaEventTypeStrings[_channelEvent->type() & 0xF][3];
    } else if (_channelEvent->type() == CHANNEL_EVENT_TYPE_META_GENERIC) {
        eventTypeString = "META";
        p1String = "Type: ";
        p2String = (_channelEvent->param1() >= 1 && _channelEvent->param1() <= 7) ? "Data: " : "Data(H):";
        p3String = "";
    }
    _typeLabelView = std::make_shared<MDStudio::LabelView>("typeLabelView", this, eventTypeString);
    _typeLabelView->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    addSubview(_typeLabelView);
    _tickTextField = std::make_shared<MDStudio::TextField>("tickTextField", this);
    _tickTextField->setText(std::to_string(_channelEvent->tickCount()));
    _tickTextField->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    _tickTextField->setTextDidChangeFn(std::bind(&EventListItemView::textFieldTextDidChange, this, _1, _2));
    addSubview(_tickTextField);
    _lengthTextField = std::make_shared<MDStudio::TextField>("lengthTextField", this);
    _lengthTextField->setText(std::to_string(_channelEvent->length()));
    _lengthTextField->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    _lengthTextField->setIsVisible(_channelEvent->type() == CHANNEL_EVENT_TYPE_NOTE);
    _lengthTextField->setTextDidChangeFn(std::bind(&EventListItemView::textFieldTextDidChange, this, _1, _2));
    addSubview(_lengthTextField);
    _channelTextField = std::make_shared<MDStudio::TextField>("channelTextField", this);
    _channelTextField->setText(std::to_string(_channelEvent->channel() + 1));
    _channelTextField->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    _channelTextField->setIsVisible(_channelEvent->type() < 0xF0);
    _channelTextField->setTextDidChangeFn(std::bind(&EventListItemView::textFieldTextDidChange, this, _1, _2));
    _channelTextField->setIsEnabled(isChannelEditionAvail);
    addSubview(_channelTextField);
    _p1LabelView = std::make_shared<MDStudio::LabelView>("p1LabelView", this, p1String);
    _p1LabelView->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    _p1LabelView->setTextAlign(MDStudio::LabelView::RightTextAlign);
    _p1LabelView->setIsVisible(!p1String.empty());
    addSubview(_p1LabelView);
    _p1TextField = std::make_shared<MDStudio::TextField>("p1TextField", this);
    // Special case for tempo
    if (_channelEvent->type() == CHANNEL_EVENT_TYPE_META_SET_TEMPO) {
        _p1TextField->setText(std::to_string(static_cast<SInt32>(60000000.0f / (float)(_channelEvent->param1()))));
    } else if (_channelEvent->type() == CHANNEL_EVENT_TYPE_PITCH_BEND) {
        _p1TextField->setText(std::to_string(_channelEvent->param1() - 8192));
    } else if (_channelEvent->type() == CHANNEL_EVENT_TYPE_SYSTEM_EXCLUSIVE) {
        std::string s;
        size_t len = _channelEvent->data().size();
        if (len > 1) {
            for (size_t i = 0; i < len - 1; ++i) {
                s += toStringHex(_channelEvent->data()[i]);
                if (i < len - 2)
                    s += " ";
            }
        }
        _p1TextField->setText(s);
    } else {
        _p1TextField->setText(std::to_string(_channelEvent->param1()));
    }
    _p1TextField->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    _p1TextField->setIsVisible(!p1String.empty());
    _p1TextField->setTextDidChangeFn(std::bind(&EventListItemView::textFieldTextDidChange, this, _1, _2));
    addSubview(_p1TextField);
    _p2LabelView = std::make_shared<MDStudio::LabelView>("p2LabelView", this, p2String);
    _p2LabelView->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    _p2LabelView->setTextAlign(MDStudio::LabelView::RightTextAlign);
    _p2LabelView->setIsVisible(!p2String.empty());
    addSubview(_p2LabelView);
    _p2TextField = std::make_shared<MDStudio::TextField>("p2TextField", this);
    if (_channelEvent->type() == CHANNEL_EVENT_TYPE_META_GENERIC) {
        std::string s;
        if (_channelEvent->param1() >= 1 && _channelEvent->param1() <= 7) {
            for (auto c : _channelEvent->data())
                s += c;
        } else {
            size_t len = _channelEvent->data().size();
            for (size_t i = 0; i < len; ++i) {
                s += toStringHex(_channelEvent->data()[i]);
                if (i < len - 1)
                    s += " ";
            }
        }
        _p2TextField->setText(s);
    } else {
        _p2TextField->setText(std::to_string(_channelEvent->param2()));
    }
    _p2TextField->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    _p2TextField->setIsVisible(!p2String.empty());
    _p2TextField->setTextDidChangeFn(std::bind(&EventListItemView::textFieldTextDidChange, this, _1, _2));
    addSubview(_p2TextField);
    _p3LabelView = std::make_shared<MDStudio::LabelView>("p3LabelView", this, p3String);
    _p3LabelView->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    _p3LabelView->setTextAlign(MDStudio::LabelView::RightTextAlign);
    _p3LabelView->setIsVisible(!p3String.empty());
    addSubview(_p3LabelView);
    _p3TextField = std::make_shared<MDStudio::TextField>("p3TextField", this);
    _p3TextField->setText(std::to_string(_channelEvent->param3()));
    _p3TextField->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    _p3TextField->setIsVisible(!p3String.empty());
    _p3TextField->setTextDidChangeFn(std::bind(&EventListItemView::textFieldTextDidChange, this, _1, _2));
    addSubview(_p3TextField);
}

// ---------------------------------------------------------------------------------------------------------------------
EventListItemView::~EventListItemView()
{
    removeSubview(_boxView);
    removeSubview(_typeLabelView);
    removeSubview(_tickTextField);
    removeSubview(_lengthTextField);
    removeSubview(_channelTextField);
    removeSubview(_p1LabelView);
    removeSubview(_p1TextField);
    removeSubview(_p2LabelView);
    removeSubview(_p2TextField);
    removeSubview(_p3LabelView);
    removeSubview(_p3TextField);
}

// ---------------------------------------------------------------------------------------------------------------------
void EventListItemView::setFrame(MDStudio::Rect rect)
{
    View::setFrame(rect);
    MDStudio::Rect r = bounds();
    _boxView->setFrame(r);
    r.size.width = _columnWidths.at(0);
    _typeLabelView->setFrame(r);
    r.origin.x += r.size.width;
    r.size.width = _columnWidths.at(1);
    _tickTextField->setFrame(r);
    r.origin.x += r.size.width;
    r.size.width = _columnWidths.at(2);
    _lengthTextField->setFrame(r);
    r.origin.x += r.size.width;
    r.size.width = _columnWidths.at(3);
    _channelTextField->setFrame(r);
    r.origin.x += r.size.width;
    r.size.width = std::min(40.0f, _columnWidths.at(4));
    _p1LabelView->setFrame(r);
    r.origin.x += r.size.width;
    r.size.width = _columnWidths.at(4) - r.size.width;
    _p1TextField->setFrame(r);
    r.origin.x += r.size.width;
    r.size.width = std::min(40.0f, _columnWidths.at(5));
    _p2LabelView->setFrame(r);
    r.origin.x += r.size.width;
    r.size.width = _columnWidths.at(5) - r.size.width;
    _p2TextField->setFrame(r);
    r.origin.x += r.size.width;
    r.size.width = std::min(40.0f, _columnWidths.at(6));
    _p3LabelView->setFrame(r);
    r.origin.x += r.size.width;
    r.size.width = _columnWidths.at(6) - r.size.width;
    _p3TextField->setFrame(r);
}

// ---------------------------------------------------------------------------------------------------------------------
void EventListItemView::setIsHighlighted(bool isHighlighted)
{
    _isHighlighted = isHighlighted;
    _boxView->setFillColor(_isHighlighted ? (_hasFocus ? MDStudio::blueColor : MDStudio::veryDimGrayColor) : MDStudio::blackColor);
}

// ---------------------------------------------------------------------------------------------------------------------
void EventListItemView::setFocusState(bool focusState)
{
    _hasFocus = focusState;
    _boxView->setFillColor(_isHighlighted ? (_hasFocus ? MDStudio::blueColor : MDStudio::veryDimGrayColor) : MDStudio::blackColor);
}

// ---------------------------------------------------------------------------------------------------------------------
void EventListItemView::textFieldTextDidChange(MDStudio::TextField *sender, std::string text)
{
    if  ((_channelEvent->type() == CHANNEL_EVENT_TYPE_SYSTEM_EXCLUSIVE && (sender == _p1TextField.get())) ||
         (_channelEvent->type() == CHANNEL_EVENT_TYPE_META_GENERIC && (sender == _p2TextField.get()))) {
        
        // Parse data
        std::vector<UInt8> data;
        
        if (_channelEvent->type() == CHANNEL_EVENT_TYPE_META_GENERIC && _channelEvent->param1() >= 1 && _channelEvent->param1() <= 7) {
            auto s = MDStudio::UString(text);
            for (auto c16 : *(s.str16())) {
                data.push_back(c16 & 0x7f);
            }
        } else {
            data = parseHexString(text);
            if (_channelEvent->type() == CHANNEL_EVENT_TYPE_SYSTEM_EXCLUSIVE) {
                // If the user did not provide the terminal F7, add it
                if ((data.size() == 0) || (data.back() != 0xF7))
                    data.push_back(0xF7);
            }
        }
        
        if (_eventDidChangeFn)
            _eventDidChangeFn(this, _channelEvent->tickCount(), _channelEvent->length(), _channelEvent->channel(), _channelEvent->param1(), _channelEvent->param2(), _channelEvent->param3(), data);
        
    } else {
    
        bool isValid = false;
        long value = 0;
        
        // Special case for tempo
        if (_channelEvent->type() == CHANNEL_EVENT_TYPE_META_SET_TEMPO && (sender == _p1TextField.get())) {
            isValid = isNumber(sender->text());
            if (isValid) {
                value = std::stol(sender->text());
                if (value == 0)
                    value = 120;
                value = 60000000.0f / value;
            } else {
                value = 60000000.0f / 120.0f;
            }
        } else if (_channelEvent->type() == CHANNEL_EVENT_TYPE_PITCH_BEND && (sender == _p1TextField.get())) {
            isValid = isNumber(sender->text(), true);
            if (isValid) {
                value = std::stol(sender->text());
                if (value < -8192)
                    value = -8192;
                else if (value > 8191)
                    value = 8191;
                value += 8192;
            } else {
                value = 8192;
            }
        } else {
            isValid = isNumber(sender->text());
            if (isValid)
                value = std::stol(sender->text());
        }

        UInt32 tickCount = (isValid && (sender == _tickTextField.get())) ? (UInt32)(value >= 0 ? value : 0) : _channelEvent->tickCount();
        UInt32 length = (isValid && (sender == _lengthTextField.get())) ? (UInt32)(value > 0 ? value : 1) : _channelEvent->length();
        UInt8 channel = (isValid && (sender == _channelTextField.get())) ? (UInt32)((value >= 1 && value <= 16) ? value - 1 : _channelEvent->channel()) : _channelEvent->channel();
        SInt32 param1 = (isValid && (sender == _p1TextField.get())) ? (SInt32)value : _channelEvent->param1();
        SInt32 param2 = (isValid && (sender == _p2TextField.get())) ? (SInt32)value : _channelEvent->param2();
        SInt32 param3 = (isValid && (sender == _p3TextField.get())) ? (SInt32)value : _channelEvent->param3();
        std::vector<UInt8> data = _channelEvent->data();
        
        if (_eventDidChangeFn)
            _eventDidChangeFn(this, tickCount, length, channel, param1, param2, param3, data);
    }
}
