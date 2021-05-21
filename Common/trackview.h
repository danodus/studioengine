//
//  trackview.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-05-19.
//  Copyright © 2016 Daniel Cliche. All rights reserved.
//

#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <view.h>
#include <studio.h>
#include <boxview.h>
#include "trackclipsview.h"

class TrackView : public MDStudio::View {
    
    std::shared_ptr<TrackClipsView> _trackClipsView;
    std::shared_ptr<MDStudio::BoxView> _boxView;
    
    bool _isHighlighted;
    bool _hasFocus;
    
public:
    
    TrackView(std::string name, void *owner, MDStudio::Studio *studio, int trackIndex, UInt8 trackChannel, double eventTickWidth, float eventHeight);
    ~TrackView();
    
    void setFrame(MDStudio::Rect rect) override;
    
    std::shared_ptr<TrackClipsView> trackClipsView() { return _trackClipsView; }
    
    void setIsHighlighted(bool isHighlighted);
    void setFocusState(bool focusState);
};


#endif // TRACKVIEW_H
