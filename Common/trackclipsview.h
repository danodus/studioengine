//
//  trackclipsview.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-09-10.
//  Copyright © 2016-2021 Daniel Cliche. All rights reserved.
//

#ifndef TRACKCLIPSVIEW_H
#define TRACKCLIPSVIEW_H

#include <view.h>
#include <scrollview.h>
#include <melobasecore_sequence.h>
#include <studio.h>
#include <image.h>

#include <functional>
#include <array>

struct TrackClipsEventRect {
    MDStudio::Rect rect;
    std::shared_ptr<MelobaseCore::ChannelEvent> channelEvent;
};

class TrackClipsView : public MDStudio::View
{
public:
    typedef std::function<void(TrackClipsView *sender, int *timeDivision, std::vector<unsigned int> *totalNbTicks, std::vector<unsigned int> *eotTickCounts, bool *areAbsTicks)> SequenceInfosFnType;
    typedef std::function<unsigned int(TrackClipsView *sender, int track)> NbEventsFnType;
    typedef std::function<void(TrackClipsView *sender, int track, int index, std::shared_ptr<MelobaseCore::Event> *event)> EventAtIndexFnType;
    typedef std::function<unsigned int(TrackClipsView* sender)> NbAnnotationsFnType;
    typedef std::function<void(TrackClipsView* sender, int index, std::shared_ptr<MelobaseCore::SequenceAnnotation>* annotation)>        AnnotationAtIndexFnType;
    typedef std::function<void(TrackClipsView *sender, unsigned int cursorTickPos)> DidSetCursorTickPosFnType;
    typedef std::function<void(TrackClipsView *sender)> DidFinishSettingCursorTickPosFnType;
    
private:
    
    void drawFilledRoundRectTopRight(MDStudio::Rect rect, float radius, MDStudio::Color fillColor);
    void drawFilledRoundRectTopLeft(MDStudio::Rect rect, float radius, MDStudio::Color fillColor);
    
    MDStudio::Studio *_studio;
    
    void updateEventRects(int trackIndex, bool areCombined, bool areChannelEventsSkipped);
    
    double _eventTickWidth;
    float _eventHeight;
    unsigned int _cursorTickPos;
    bool _isCaptured;
    std::array<bool, STUDIO_MAX_CHANNELS> _visibleChannels;
    
    // Data source
    SequenceInfosFnType _sequenceInfosFn = nullptr;
    NbEventsFnType _nbEventsFn = nullptr;
    EventAtIndexFnType _eventAtIndexFn = nullptr;
    NbAnnotationsFnType _nbAnnotationsFn = nullptr;
    AnnotationAtIndexFnType _annotationAtIndexFn = nullptr;
    
    DidSetCursorTickPosFnType _didSetCursorTickPosFn = nullptr;
    DidFinishSettingCursorTickPosFnType _didFinishSettingCursorTickPosFn = nullptr;
    
    std::vector<TrackClipsEventRect> _eventRects[STUDIO_MAX_CHANNELS];
    
    MDStudio::ScrollView *_mainScrollView;
    
    bool _isMovingCursor;
    
    int _trackIndex;
    UInt8 _trackChannel;

    std::shared_ptr<MDStudio::Image> _flagImage;
    
    UInt8 rechannelize(UInt8 channel) { return _trackChannel == SEQUENCE_TRACK_MULTI_CHANNEL ? channel : _trackChannel; }
    
    void getMeasureTicks(int timeDivision, unsigned int totalNbTicks, bool areAbsTicks, std::vector<unsigned int> *measureTicks, std::vector<int> *numerators);
    
    void drawAnnotations();
    
public:
    TrackClipsView(std::string name, void *owner, MDStudio::Studio *studio, int trackIndex, UInt8 trackChannel, double eventTickWidth, float eventHeight, MDStudio::ScrollView *mainScrollView = nullptr);
    ~TrackClipsView();
    
    bool handleEvent(const MDStudio::UIEvent *event) override;
    
    void draw() override;
    
    void setEventTickWidth(double eventTickWidth) { _eventTickWidth = eventTickWidth; setDirty(); }
    void setEventHeight(float eventHeight) { _eventHeight = eventHeight; setDirty(); }
    
    double eventTickWidth() { return _eventTickWidth; }
    float eventHeight() { return _eventHeight; }
    
    void setCursorTickPos(unsigned int cursorTickPos, bool isDelegateNotified = true);
    unsigned int cursorTickPos() { return _cursorTickPos; }
    
    bool isCaptured() { return _isCaptured; }
    
    void setVisibleChannels(std::array<bool, STUDIO_MAX_CHANNELS> visibleChannels);
    std::array<bool, STUDIO_MAX_CHANNELS> visibleChannels() { return _visibleChannels; }
    
    void setTrackIndex(int trackIndex);
    void setTrackChannel(UInt8 channel);
    
    void setSequenceInfosFn(SequenceInfosFnType sequenceInfosFn) { _sequenceInfosFn = sequenceInfosFn; }
    void setNbEventsFn(NbEventsFnType nbEventsFn) { _nbEventsFn = nbEventsFn; }
    void setNbAnnotationsFn(NbAnnotationsFnType nbAnnotationsFn) { _nbAnnotationsFn = nbAnnotationsFn; }
    void setAnnotationAtIndexFn(AnnotationAtIndexFnType annotationAtIndexFn) { _annotationAtIndexFn = annotationAtIndexFn; }
    void setEventAtIndexFn(EventAtIndexFnType eventAtIndexFn) { _eventAtIndexFn = eventAtIndexFn; }
    void setDidSetCursorTickPosFn(DidSetCursorTickPosFnType didSetCursorTickPosFn) { _didSetCursorTickPosFn = didSetCursorTickPosFn; }
    void setDidFinishSettingCursorTickPosFn(DidFinishSettingCursorTickPosFnType didFinishSettingCursorTickPosFn) { _didFinishSettingCursorTickPosFn = didFinishSettingCursorTickPosFn; }
};

#endif // TRACKCLIPSVIEW_H

