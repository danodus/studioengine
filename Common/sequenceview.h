//
//  sequenceview.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-05-19.
//  Copyright © 2016 Daniel Cliche. All rights reserved.
//

#ifndef SEQUENCEVIEW_H
#define SEQUENCEVIEW_H

#include <view.h>
#include <splitviewv.h>
#include <studio.h>
#include "pianorollview.h"
#include "arrangementview.h"

class SequenceView : public MDStudio::View {

    std::shared_ptr<PianoRollView> _pianoRollView;
    std::shared_ptr<ArrangementView> _arrangementView;
    std::shared_ptr<MDStudio::SplitViewV> _vertSplitView;

public:
    
    SequenceView(std::string name, void *owner, MDStudio::Studio *studio, double eventTickWidth, float eventHeight);

    void setFrame(MDStudio::Rect rect) override;
    
    std::shared_ptr<PianoRollView> pianoRollView() { return _pianoRollView; }
    std::shared_ptr<ArrangementView> arrangementView() { return _arrangementView; }
    std::shared_ptr<MDStudio::SplitViewV> vertSplitView() { return _vertSplitView; }
};

#endif // SEQUENCEVIEW_H
