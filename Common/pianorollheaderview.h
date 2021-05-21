//
//  pianorollheaderview.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2014-09-29.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#ifndef PIANOROLLHEADERVIEW_H
#define PIANOROLLHEADERVIEW_H

#include <font.h>
#include <melobasecore_event.h>
#include <melobasecore_sequence.h>
#include <scrollview.h>
#include <view.h>

#include <functional>

class PianoRollHeaderView : public MDStudio::View {
   public:
    typedef std::function<void(PianoRollHeaderView* sender, int* timeDivision, std::vector<unsigned int>* totalNbTicks,
                               std::vector<unsigned int>* eotTickCounts, bool* areAbsTicks)>
        SequenceInfosFnType;
    typedef std::function<unsigned int(PianoRollHeaderView* sender, int track)> NbEventsFnType;
    typedef std::function<void(PianoRollHeaderView* sender, int track, int index,
                               std::shared_ptr<MelobaseCore::Event>* event)>
        EventAtIndexFnType;
    typedef std::function<unsigned int(PianoRollHeaderView* sender)> NbAnnotationsFnType;
    typedef std::function<void(PianoRollHeaderView* sender, int index,
                               std::shared_ptr<MelobaseCore::SequenceAnnotation>* annotation)>
        AnnotationAtIndexFnType;
    typedef std::function<void(PianoRollHeaderView* sender, unsigned int cursorTickPos)> DidSetCursorTickPosFnType;

   private:
    double _eventTickWidth;

    SequenceInfosFnType _sequenceInfosFn = nullptr;
    NbEventsFnType _nbEventsFn = nullptr;
    EventAtIndexFnType _eventAtIndexFn = nullptr;
    NbAnnotationsFnType _nbAnnotationsFn = nullptr;
    AnnotationAtIndexFnType _annotationAtIndexFn = nullptr;
    DidSetCursorTickPosFnType _didSetCursorTickPosFn = nullptr;

    MDStudio::MultiDPIFont* _font;

    MDStudio::ScrollView* _mainScrollView;

    int _trackIndex;

    bool _isCaptured;
    bool _hasFocus;

    std::shared_ptr<MDStudio::Image> _flagImage;

    void draw() override;
    void setCursorTickPos(unsigned int cursorTickPos);
    void drawMeasures();
    void drawAnnotations();

   public:
    PianoRollHeaderView(std::string name, void* owner, int trackIndex, double eventTickWidth,
                        MDStudio::ScrollView* mainScrollView = nullptr);
    ~PianoRollHeaderView();

    bool handleEvent(const MDStudio::UIEvent* event) override;

    void setEventTickWidth(double eventTickWidth) {
        _eventTickWidth = eventTickWidth;
        setDirty();
    }

    bool isCaptured() { return _isCaptured; }

    void setTrackIndex(int trackIndex);

    void setFocusState(bool state) {
        _hasFocus = state;
        setDirty();
    }

    void setSequenceInfosFn(SequenceInfosFnType sequenceInfosFn) { _sequenceInfosFn = sequenceInfosFn; }
    void setNbEventsFn(NbEventsFnType nbEventsFn) { _nbEventsFn = nbEventsFn; }
    void setEventAtIndexFn(EventAtIndexFnType eventAtIndexFn) { _eventAtIndexFn = eventAtIndexFn; }
    void setNbAnnotationsFn(NbAnnotationsFnType nbAnnotationsFn) { _nbAnnotationsFn = nbAnnotationsFn; }
    void setAnnotationAtIndexFn(AnnotationAtIndexFnType annotationAtIndexFn) {
        _annotationAtIndexFn = annotationAtIndexFn;
    }
    void setDidSetCursorTickPosFn(DidSetCursorTickPosFnType didSetCursorTickPosFn) {
        _didSetCursorTickPosFn = didSetCursorTickPosFn;
    }
};

#endif  // PIANOROLLHEADERVIEW_H
