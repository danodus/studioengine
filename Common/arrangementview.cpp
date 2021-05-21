//
//  arrangementview.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-05-19.
//  Copyright © 2016-2019 Daniel Cliche. All rights reserved.
//

#include "arrangementview.h"

#include <platform.h>

// ---------------------------------------------------------------------------------------------------------------------
ArrangementView::ArrangementView(std::string name, void *owner, double eventTickWidth, float eventHeight) : MDStudio::View(name, owner), _eventTickWidth(eventTickWidth), _eventHeight(eventHeight)
{
    _ui.loadUI(this, MDStudio::Platform::sharedInstance()->resourcesPath() + "/ArrangementView.lua");
    
    _moveTrackUpImage = std::make_shared<MDStudio::Image>("MoveTrackUp@2x.png");
    _moveTrackDownImage = std::make_shared<MDStudio::Image>("MoveTrackDown@2x.png");
    
    _editionBoxView = std::make_shared<MDStudio::BoxView>("editionBoxView", this);
    _editionBoxView->setFillColor(MDStudio::veryDimGrayColor);
    _editionBoxView->setBorderColor(MDStudio::veryDimGrayColor);
    addSubview(_editionBoxView);
    
    _addTrackButton = std::make_shared<MDStudio::Button>("addTrackButton", this, "+", nullptr);
    addSubview(_addTrackButton);
    
    _deleteTrackButton = std::make_shared<MDStudio::Button>("deleteTrackButton", this, "-", nullptr);
    addSubview(_deleteTrackButton);

    _mergeTracksButton = std::make_shared<MDStudio::Button>("mergeTracksButton", this, _ui.findString("mergeTracksStr"), nullptr);
    addSubview(_mergeTracksButton);

    _moveTrackUpButton = std::make_shared<MDStudio::Button>("moveTrackUpButton", this, "", _moveTrackUpImage);
    addSubview(_moveTrackUpButton);
    
    _moveTrackDownButton = std::make_shared<MDStudio::Button>("moveTrackDownButton", this, "", _moveTrackDownImage);
    addSubview(_moveTrackDownButton);
    
    _showHideFirstTrackButton = std::make_shared<MDStudio::Button>("showHideFirstTrackButton", this, _ui.findString("firstTrackStr"), nullptr);
    _showHideFirstTrackButton->setType(MDStudio::Button::CheckBoxButtonType);
    addSubview(_showHideFirstTrackButton);
    
    _tracksListView = std::make_shared<MDStudio::ListView>("tracksListView", this, 100.0f, true);
    _tracksListView->setIsPassThrough(true);
    _tracksScrollView = std::make_shared<MDStudio::ScrollView>("tracksScrollView", this, _tracksListView, true);
    
    _trackHeadersOverlayListView = std::make_shared<MDStudio::ListView>("trackHeadersOverlayListView", this, 100.0f);
    _trackHeadersOverlayListView->setIsStatic(true);
    
    _trackHeadersListView = std::make_shared<MDStudio::ListView>("trackHeadersListView", this, 100.0f, true);
    _trackHeadersListView->setIsPassThrough(true);
    _trackHeadersListView->setIsCapturing(false);

    _trackHeadersView = std::make_shared<MDStudio::View>("trackHeadersView", this);
    _trackHeadersView->addSubview(_trackHeadersListView);
    _trackHeadersView->addSubview(_trackHeadersOverlayListView);
    _trackHeadersView->setLayoutFn([=](MDStudio::View *sender, MDStudio::Rect frame) {
        _trackHeadersListView->setFrame(sender->bounds());
        _trackHeadersOverlayListView->setFrame(sender->bounds());
    });

    _trackHeadersScrollView = std::make_shared<MDStudio::ScrollView>("trackHeadersScrollView", this, _trackHeadersView, true);
    _trackHeadersScrollView->setIsHorizScrollBarVisible(false);
    _trackHeadersScrollView->setIsVertScrollBarVisible(false);
    
    _trackHeadersBorderView = std::make_shared<BorderView>("trackHeadersBorderView", this, _trackHeadersScrollView, 0.0f, 0.0f, SCROLL_VIEW_SCROLL_BAR_THICKNESS, 0.0f);
    
    _trackHeadersSplitView = std::make_shared<MDStudio::SplitViewH>("trackHeadersSplitView", this, _trackHeadersBorderView, _tracksScrollView, 150.0f);
    addSubview(_trackHeadersSplitView);
    
    _trackHeadersSplitView->setPosChangedFn([=](MDStudio::SplitViewH *sender, float pos){
        _trackHeadersScrollView->setContentSize(MDStudio::makeSize(_trackHeadersScrollView->rect().size.width, _tracksScrollView->contentView()->rect().size.height));
    });
}

// ---------------------------------------------------------------------------------------------------------------------
ArrangementView::~ArrangementView()
{
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementView::setFrame(MDStudio::Rect aRect)
{
    View::setFrame(aRect);
    
    MDStudio::Rect r = MDStudio::makeRect(0.0f, bounds().size.height - 20.0f, bounds().size.width, 20.0f);
    _editionBoxView->setFrame(r);
    
    r.size.width = 20.0f;
    _addTrackButton->setFrame(r);
    r.origin.x += r.size.width - 1;
    _deleteTrackButton->setFrame(r);
    r.origin.x += r.size.width;

    r.origin.x += 10.0f;
    r.size.width = 150.0f;
    _mergeTracksButton->setFrame(r);
    r.origin.x += r.size.width;

    r.origin.x += 10.0f;
    r.size.width = 20.0f;
    _moveTrackUpButton->setFrame(r);
    r.origin.x += r.size.width - 1;
    _moveTrackDownButton->setFrame(r);
    r.origin.x += r.size.width;
    
    r.origin.x += 10.0f;
    r.size.width = 150.0f;
    _showHideFirstTrackButton->setFrame(r);
    
    r = bounds();
    r.size.height -= 20.0f;
    _trackHeadersSplitView->setFrame(r);
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementView::scrollToCenteredCursor(unsigned int tickPos)
{
    float pos = _eventTickWidth * tickPos;
    MDStudio::Rect contentRect = _tracksListView->rect();
    float w = (_tracksScrollView->posMaxH() - _tracksScrollView->posMinH());
    _tracksScrollView->scrollToVisibleRect(MDStudio::makeRect(contentRect.origin.x + pos - w / 2.0f, contentRect.origin.y + _tracksScrollView->posMinV(), w, _tracksScrollView->posMaxV() - _tracksScrollView->posMinV()));
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementView::scrollToVisibleCursor(unsigned int tickPos)
{
    float pos = _eventTickWidth * tickPos;
    MDStudio::Rect contentRect = _tracksListView->rect();
    _tracksScrollView->scrollToVisibleRect(MDStudio::makeRect(contentRect.origin.x + pos, contentRect.origin.y + _tracksScrollView->posMinV(), 1, _tracksScrollView->posMaxV() - _tracksScrollView->posMinV()));
}
