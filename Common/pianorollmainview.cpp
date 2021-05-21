//
//  pianorollmainview.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-09-28.
//  Copyright © 2016-2018 Daniel Cliche. All rights reserved.
//

#include "pianorollmainview.h"

#include "helpers.h"

#include <platform.h>

#define PIANO_ROLL_MAIN_VIEW_KEYBOARD_WIDTH      (80.0f + 2.0f)
#define PIANO_ROLL_MAIN_VIEW_HEADER_HEIGHT       40.0f



// ---------------------------------------------------------------------------------------------------------------------
PianoRollMainView::PianoRollMainView(std::string name, void *owner, MDStudio::Studio *studio, double eventTickWidth, float eventHeight) : View(name, owner)
{
    _ui.loadUI(this, MDStudio::Platform::sharedInstance()->resourcesPath() + "/PianoRollMainView.lua");
    
    _upperZoneImage = std::make_shared<MDStudio::Image>("UpperZone@2x.png");
    _lowerZoneImage = std::make_shared<MDStudio::Image>("LowerZone@2x.png");

    _keyboard = std::shared_ptr<KeyboardV>(new KeyboardV("keyboard", owner));
    _keyboardChannelBoxView = std::make_shared<MDStudio::BoxView>("keyboardChannelBoxView", owner);
    MDStudio::Color color = channelColors[0];
    color.red *= 0.75f;
    color.green *= 0.75f;
    color.blue *= 0.75f;
    _keyboardChannelBoxView->setBorderColor(color);
    _keyboardChannelBoxView->setFillColor(color);
    
    std::vector<Any> zoneSelectionItems = {_lowerZoneImage, _upperZoneImage};
    _zoneSelectionSegmentedControl = std::make_shared<MDStudio::SegmentedControl>("zoneSelectionSegmentedControl", owner, zoneSelectionItems);
    _zoneSelectionSegmentedControl->setSelectedSegment(1);
    _currentVelocityLabelView = std::shared_ptr<MDStudio::LabelView>(new MDStudio::LabelView("currentVelocityLabelView", owner, _ui.findString("currentVelocityStr")));
    _currentVelocityLabelView->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    _currentVelocitySlider = std::shared_ptr<MDStudio::Slider>(new MDStudio::Slider("currentVelocitySlider", owner, 0, 127, 64));
    
    _pianoRollEventsView = std::shared_ptr<PianoRollEventsView>(new PianoRollEventsView("pianoRollEventsView", owner, studio, 0, SEQUENCE_TRACK_MULTI_CHANNEL, eventTickWidth, eventHeight, false));
    _pianoRollEventsScrollBar = std::shared_ptr<MDStudio::ScrollBar>(new MDStudio::ScrollBar("pianoRollEventsScrollBar", owner, 0, 0, 0, 0));
    _pianoRollEventsScrollView = std::shared_ptr<MDStudio::ScrollView>(new MDStudio::ScrollView("pianoRollEventsScrollView", owner, _pianoRollEventsView, false, _pianoRollEventsScrollBar));
    _pianoRollHeaderView = std::shared_ptr<PianoRollHeaderView>(new PianoRollHeaderView("pianoRollHeaderView", owner, 0, eventTickWidth, _pianoRollEventsScrollView.get()));
    _pianoRollControllerEventsView = std::shared_ptr<PianoRollEventsView>(new PianoRollEventsView("pianoRollControllerEventsView", owner, studio, 0, SEQUENCE_TRACK_MULTI_CHANNEL, eventTickWidth, 1.0f, true, _pianoRollEventsScrollView.get()));
    _pianoRollControllerEventsView->setIsVisible(false);
    
    _pianoRollMetaEventView = std::make_shared<PianoRollEventsView>("pianoRollMetaEventView", owner, studio, 0, SEQUENCE_TRACK_MULTI_CHANNEL, eventTickWidth, 1.0f, true, _pianoRollEventsScrollView.get());
    
    std::vector<std::string> controllerStrings = {
        _ui.findString("controllerControlChangeStr"),
        _ui.findString("controllerProgramChangesStr"),
        _ui.findString("controllerPitchBendStr"),
        _ui.findString("controllerKeyAftertouchStr"),
        _ui.findString("controllerChannelAftertouchStr"),
        _ui.findString("controllerSysexStr"),
        _ui.findString("controllerMetaTypeStr")
    };
    
    std::vector<std::string> controllerTooltipStrings = {
        _ui.findString("controllerControlChangeTooltipStr"),
        _ui.findString("controllerProgramChangesTooltipStr"),
        _ui.findString("controllerPitchBendTooltipStr"),
        _ui.findString("controllerKeyAftertouchTooltipStr"),
        _ui.findString("controllerChannelAftertouchTooltipStr"),
        _ui.findString("controllerSysexTooltipStr"),
        _ui.findString("controllerMetaTypeTooltipStr")
    };
    
    _controllerSegmentedControlV = std::shared_ptr<SegmentedControlV>(new SegmentedControlV("controllerSegmentedControlV", owner, controllerStrings, controllerTooltipStrings));
    _controllerSegmentedControlV->setIsVisible(false);
    
    _controllerEventsRulerView = make_shared<RulerView>("controllerEventsRulerView", this, 10.0f);
    _controllerEventsRulerView->setIsVisible(false);
    
    _controlChangeComboBox = make_shared<MDStudio::ComboBox>("controlChangeComboBox", this, 12.0f);
    _controlChangeComboBox->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    _controlChangeComboBox->setListPosition(MDStudio::ComboBox::AbovePosition);
    _controlChangeComboBox->setIsHorizScrollBarVisible(false);
    _controlChangeComboBox->setIsVertScrollBarVisible(true);
    _controlChangeComboBox->setIsVisible(false);

    _metaTypeComboBox = make_shared<MDStudio::ComboBox>("metaTypeComboBox", this, 12.0f);
    _metaTypeComboBox->setFont(MDStudio::SystemFonts::sharedInstance()->semiboldFontSmall());
    _metaTypeComboBox->setListPosition(MDStudio::ComboBox::AbovePosition);
    _metaTypeComboBox->setIsHorizScrollBarVisible(false);
    _metaTypeComboBox->setIsVertScrollBarVisible(true);
    _metaTypeComboBox->setIsVisible(false);

    addSubview(_keyboard);
    addSubview(_keyboardChannelBoxView);
    addSubview(_currentVelocityLabelView);
    addSubview(_currentVelocitySlider);
    addSubview(_zoneSelectionSegmentedControl);
    addSubview(_pianoRollHeaderView);
    addSubview(_pianoRollEventsScrollBar);
    addSubview(_pianoRollEventsScrollView);
    addSubview(_pianoRollControllerEventsView);
    addSubview(_pianoRollMetaEventView);
    addSubview(_controllerSegmentedControlV);
    addSubview(_controllerEventsRulerView);
    addSubview(_controlChangeComboBox);
    addSubview(_metaTypeComboBox);
}

// ---------------------------------------------------------------------------------------------------------------------
PianoRollMainView::~PianoRollMainView()
{
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollMainView::scrollToCenteredCursor()
{
    float pos = _pianoRollEventsView->eventTickWidth() * _pianoRollEventsView->cursorTickPos();
    MDStudio::Rect pianoRollContentRect = _pianoRollEventsView->rect();
    float w = (_pianoRollEventsScrollView->posMaxH() - _pianoRollEventsScrollView->posMinH());
    _pianoRollEventsScrollView->scrollToVisibleRect(MDStudio::makeRect(pianoRollContentRect.origin.x + pos - w / 2.0f, pianoRollContentRect.origin.y + _pianoRollEventsScrollView->posMinV(), w, _pianoRollEventsScrollView->posMaxV() - _pianoRollEventsScrollView->posMinV()));
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollMainView::scrollToVisibleCursor()
{
    float pos = _pianoRollEventsView->eventTickWidth() * _pianoRollEventsView->cursorTickPos();
    MDStudio::Rect pianoRollContentRect = _pianoRollEventsView->rect();
    _pianoRollEventsScrollView->scrollToVisibleRect(MDStudio::makeRect(pianoRollContentRect.origin.x + pos, pianoRollContentRect.origin.y + _pianoRollEventsScrollView->posMinV(), 1, _pianoRollEventsScrollView->posMaxV() - _pianoRollEventsScrollView->posMinV()));
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollMainView::scrollToVisibleRect(MDStudio::Rect r, bool isHorizOnly)
{
    MDStudio::Rect pianoRollContentRect = _pianoRollEventsView->rect();
    MDStudio::Rect visibleRect = MDStudio::makeRect(pianoRollContentRect.origin.x + r.origin.x, pianoRollContentRect.origin.y + r.origin.y, r.size.width, r.size.height);
    if (isHorizOnly) {
        _pianoRollEventsScrollView->scrollToVisibleRectH(visibleRect);
    } else {
        _pianoRollEventsScrollView->scrollToVisibleRect(visibleRect);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollMainView::setFrame(MDStudio::Rect aRect)
{
    float pianoRollViewControllerHeight = _pianoRollControllerEventsView->isVisible() ? PIANO_ROLL_MAIN_VIEW_CONTROLLER_HEIGHT : 0.0f;
    
    View::setFrame(aRect);
    
    MDStudio::Rect r = MDStudio::makeRect(2.0f, SCROLL_VIEW_SCROLL_BAR_THICKNESS + pianoRollViewControllerHeight, PIANO_ROLL_MAIN_VIEW_KEYBOARD_WIDTH - 2.0f, rect().size.height - PIANO_ROLL_MAIN_VIEW_HEADER_HEIGHT - SCROLL_VIEW_SCROLL_BAR_THICKNESS - pianoRollViewControllerHeight);
    _keyboard->setFrame(r);
    r.origin.x = 0.0f;
    r.size.width = 2.0f;
    _keyboardChannelBoxView->setFrame(r);
    
    r.origin.y = r.origin.y + r.size.height;
    r.size.height = 20.0f;
    r.size.width = 20.0f;
    _currentVelocityLabelView->setFrame(r);
    
    r.origin.x += r.size.width + 4.0f;
    r.size.width = 55.0f;
    _currentVelocitySlider->setFrame(makeInsetRect(r, 0.0f, 4.0f));
    
    r.origin.y += r.size.height - 2;
    r.origin.x = 0.0f;
    r.size.width = PIANO_ROLL_MAIN_VIEW_KEYBOARD_WIDTH;
    _zoneSelectionSegmentedControl->setFrame(makeInsetRect(r, 2.0f, 0.0f));
    
    r = MDStudio::makeRect(PIANO_ROLL_MAIN_VIEW_KEYBOARD_WIDTH, pianoRollViewControllerHeight + SCROLL_VIEW_SCROLL_BAR_THICKNESS, rect().size.width - PIANO_ROLL_MAIN_VIEW_KEYBOARD_WIDTH, rect().size.height - PIANO_ROLL_MAIN_VIEW_HEADER_HEIGHT - pianoRollViewControllerHeight - SCROLL_VIEW_SCROLL_BAR_THICKNESS);
    _pianoRollEventsScrollView->setFrame(r);
    
    r.origin.y = bounds().size.height - PIANO_ROLL_MAIN_VIEW_HEADER_HEIGHT + 15.0f;
    r.size.height = PIANO_ROLL_MAIN_VIEW_HEADER_HEIGHT - 15.0f;
    _pianoRollHeaderView->setFrame(r);
    
    r.origin.y -= 15.0f;
    r.size.height = 15.0f;
    _pianoRollMetaEventView->setFrame(r);
    
    r = MDStudio::makeRect(0.0f, SCROLL_VIEW_SCROLL_BAR_THICKNESS, PIANO_ROLL_MAIN_VIEW_KEYBOARD_WIDTH - 50.0f, PIANO_ROLL_MAIN_VIEW_CONTROLLER_HEIGHT);
    _controllerSegmentedControlV->setFrame(makeCenteredRectInRect(r, PIANO_ROLL_MAIN_VIEW_KEYBOARD_WIDTH - 50.0f, 130.0f));
    
    r = MDStudio::makeRect(PIANO_ROLL_MAIN_VIEW_KEYBOARD_WIDTH - 50.0f, SCROLL_VIEW_SCROLL_BAR_THICKNESS, 50.0f, PIANO_ROLL_MAIN_VIEW_CONTROLLER_HEIGHT);
    _controllerEventsRulerView->setFrame(r);
    
    r = MDStudio::makeRect(PIANO_ROLL_MAIN_VIEW_KEYBOARD_WIDTH, SCROLL_VIEW_SCROLL_BAR_THICKNESS, rect().size.width - PIANO_ROLL_MAIN_VIEW_KEYBOARD_WIDTH - SCROLL_VIEW_SCROLL_BAR_THICKNESS, pianoRollViewControllerHeight);
    _pianoRollControllerEventsView->setFrame(r);
    
    if (!_controlChangeComboBox->isVisible() && !_metaTypeComboBox->isVisible()) {
        r = MDStudio::makeRect(PIANO_ROLL_MAIN_VIEW_KEYBOARD_WIDTH, 0.0f, rect().size.width - PIANO_ROLL_MAIN_VIEW_KEYBOARD_WIDTH - SCROLL_VIEW_SCROLL_BAR_THICKNESS, SCROLL_VIEW_SCROLL_BAR_THICKNESS);
    } else {
        r = MDStudio::makeRect(150.0f, 0.0f, rect().size.width - 150.0f - SCROLL_VIEW_SCROLL_BAR_THICKNESS, SCROLL_VIEW_SCROLL_BAR_THICKNESS);
    }
    _pianoRollEventsScrollBar->setFrame(r);
    
    r = MDStudio::makeRect(0.0f, 0.0f, 150.0f, SCROLL_VIEW_SCROLL_BAR_THICKNESS);
    _controlChangeComboBox->setFrame(r);
    _metaTypeComboBox->setFrame(r);
}

