//
//  arrangementview.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-05-19.
//  Copyright © 2016-2019 Daniel Cliche. All rights reserved.
//

#ifndef ARRANGEMENTVIEW_H
#define ARRANGEMENTVIEW_H

#include <scrollview.h>
#include <studio.h>
#include <listview.h>
#include <button.h>
#include <boxview.h>
#include <ui.h>
#include <splitviewh.h>

#include "borderview.h"

class ArrangementView : public MDStudio::View {
    
private:
    
    std::shared_ptr<MDStudio::ScrollView> _tracksScrollView;
    std::shared_ptr<MDStudio::ListView> _tracksListView;
    std::shared_ptr<MDStudio::Button> _addTrackButton;
    std::shared_ptr<MDStudio::Button> _deleteTrackButton;
    std::shared_ptr<MDStudio::Button> _mergeTracksButton;
    std::shared_ptr<MDStudio::Button> _moveTrackUpButton;
    std::shared_ptr<MDStudio::Button> _moveTrackDownButton;
    std::shared_ptr<MDStudio::Button> _showHideFirstTrackButton;
    std::shared_ptr<MDStudio::BoxView> _editionBoxView;
    std::shared_ptr<MDStudio::ScrollView> _trackHeadersScrollView;
    std::shared_ptr<MDStudio::View> _trackHeadersView;
    std::shared_ptr<MDStudio::ListView> _trackHeadersOverlayListView;
    std::shared_ptr<MDStudio::ListView> _trackHeadersListView;
    std::shared_ptr<MDStudio::SplitViewH> _trackHeadersSplitView;
    std::shared_ptr<BorderView> _trackHeadersBorderView;
    
    MDStudio::UI _ui;
    
    double _eventTickWidth;
    float _eventHeight;
    
    std::shared_ptr<MDStudio::Image> _moveTrackUpImage, _moveTrackDownImage;
    
public:	
    
    ArrangementView(std::string name, void *owner, double eventTickWidth, float eventHeight);
    ~ArrangementView();
    
    double eventTickWidth() { return _eventTickWidth; }
    float eventHeight() { return _eventHeight; }
    
    void setFrame(MDStudio::Rect rect) override;
    
    void scrollToCenteredCursor(unsigned int tickPos);
    void scrollToVisibleCursor(unsigned int tickPos);

    std::shared_ptr<MDStudio::ScrollView> tracksScrollView() { return _tracksScrollView; }
    std::shared_ptr<MDStudio::ListView> tracksListView() { return _tracksListView; }
    std::shared_ptr<MDStudio::Button> addTrackButton() { return _addTrackButton; }
    std::shared_ptr<MDStudio::Button> deleteTrackButton() { return _deleteTrackButton; }
    std::shared_ptr<MDStudio::Button> mergeTracksButton() { return _mergeTracksButton; }
    std::shared_ptr<MDStudio::Button> moveTrackUpButton() { return _moveTrackUpButton; }
    std::shared_ptr<MDStudio::Button> moveTrackDownButton() { return _moveTrackDownButton; }
    std::shared_ptr<MDStudio::Button> showHideFirstTrackButton() { return _showHideFirstTrackButton; }
    std::shared_ptr<MDStudio::ScrollView> trackHeadersScrollView() { return _trackHeadersScrollView; }
    std::shared_ptr<MDStudio::ListView> trackHeadersListView() { return _trackHeadersListView; }
    std::shared_ptr<MDStudio::ListView> trackHeadersOverlayListView() { return _trackHeadersOverlayListView; }

};


#endif // ARRANGEMENTVIEW_H
