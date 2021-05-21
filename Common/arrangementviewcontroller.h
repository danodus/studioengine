//
//  arrangementviewcontroller.hpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-05-19.
//  Copyright (c) 2016-2021 Daniel Cliche. All rights reserved.
//

#ifndef ARRANGEMENTVIEWCONTROLLER_H
#define ARRANGEMENTVIEWCONTROLLER_H

#include <sequence.h>
#include <sequenceeditor.h>
#include <studio.h>
#include <studiocontroller.h>

#include "arrangementview.h"
#include "pianorollviewcontroller.h"
#include "trackclipsview.h"

class ArrangementViewController {
   public:
    typedef std::function<void(ArrangementViewController* sender)> WillModifySequenceFnType;
    typedef std::function<void(ArrangementViewController* sender, unsigned int cursorTickPos)>
        DidSetCursorTickPosFnType;

   private:
    MelobaseCore::StudioController* _studioController;
    MelobaseCore::SequenceEditor* _sequenceEditor;

    PianoRollViewController* _pianoRollViewController;

    int _timeDivision;
    std::vector<unsigned int> _totalTickCounts, _eotTickCounts;

    int _previousNbTracks;

    bool _isFirstTrackShown;

    std::shared_ptr<MelobaseCore::Sequence> _sequence;
    std::shared_ptr<ArrangementView> _arrangementView;

    WillModifySequenceFnType _willModifySequenceFn;
    DidSetCursorTickPosFnType _didSetCursorTickPosFn;

    MDStudio::Point _lastPianoRollPos;
    int _lastEditionModeIndex;

    bool _isDelegateNotified;

    void sequenceInfos(TrackClipsView* sender, int* timeDivision, std::vector<unsigned int>* totalNbTicks,
                       std::vector<unsigned int>* eotTickCounts, bool* areAbsTicks);
    int nbEvents(TrackClipsView* sender, int track);
    void eventAtIndex(TrackClipsView* sender, int track, int index, std::shared_ptr<MelobaseCore::Event>* event);
    int nbAnnotations(TrackClipsView* sender);
    void annotationAtIndex(TrackClipsView* sender, int index, std::shared_ptr<MelobaseCore::SequenceAnnotation>* annotation);
    void didSetCursorTickPos(TrackClipsView* sender, unsigned int cursorTickPos);

    unsigned int tracksListViewNbRows(MDStudio::ListView* sender);
    std::shared_ptr<MDStudio::View> tracksListViewViewForRow(MDStudio::ListView* sender, int row);
    void tracksListViewDidSelectRow(MDStudio::ListView* sender, int row);
    void tracksListViewDidDeselectRow(MDStudio::ListView* sender, int row);
    void tracksListViewDidSetFocusState(MDStudio::ListView* sender, bool state);
    bool tracksListViewDidPressUnhandledKey(MDStudio::ListView* sender, unsigned int key);

    unsigned int trackHeadersListViewNbRows(MDStudio::ListView* sender);
    std::shared_ptr<MDStudio::View> trackHeadersListViewViewForRow(MDStudio::ListView* sender, int row);
    void trackHeadersListViewDidSelectRow(MDStudio::ListView* sender, int row);
    void trackHeadersListViewDidDeselectRow(MDStudio::ListView* sender, int row);
    void trackHeadersListViewDidSetFocusState(MDStudio::ListView* sender, bool state);

    void addTrackClicked(MDStudio::Button* sender);
    void deleteTrackClicked(MDStudio::Button* sender);
    void mergeTracksClicked(MDStudio::Button* sender);
    void moveTrackUpClicked(MDStudio::Button* sender);
    void moveTrackDownClicked(MDStudio::Button* sender);
    void showHideFirstTrackStateDidChange(MDStudio::Button* sender, bool state);

    void tracksScrollViewPosChanged(MDStudio::ScrollView* sender, MDStudio::Point pos);
    void trackHeadersScrollViewPosChanged(MDStudio::ScrollView* sender, MDStudio::Point pos);

    void trackHeaderNameTextFieldNameDidChange(MDStudio::TextField* sender, std::string text);
    void trackHeaderArmedStateDidChange(MDStudio::Button* sender, bool state);
    void trackHeaderMuteStateDidChange(MDStudio::Button* sender, bool state);
    void trackHeaderSoloStateDidChange(MDStudio::Button* sender, bool state);

    unsigned int trackHeaderChannelNbRows(MDStudio::ComboBox* sender);
    std::shared_ptr<MDStudio::View> trackHeaderChannelViewForRow(MDStudio::ComboBox* sender, int row);
    void trackHeaderChannelDidSelectRow(MDStudio::ComboBox* sender, int row);
    void trackHeaderChannelDidHoverRow(MDStudio::ComboBox* sender, int row);
    void trackHeaderChannelDidConfirmRowSelection(MDStudio::ComboBox* sender, int row);

    bool removeSelectedTracks();

   public:
    ArrangementViewController(std::shared_ptr<ArrangementView> arrangementView,
                              MelobaseCore::StudioController* studioController,
                              MelobaseCore::SequenceEditor* sequenceEditor,
                              PianoRollViewController* pianoRollViewController);

    void setSequence(std::shared_ptr<MelobaseCore::Sequence> sequence);
    void clearTrackSelection();
    void selectTrack(int trackIndex);

    void updateTracks();
    void updateTrackControls();
    void setCursorTickPos(unsigned int tickPos, bool isScrollingToCenteredCursor, bool isScrollingToVisibleCursor);
    void handleEventRepeat();

    bool deleteSelectedTracks();
    bool copySelectedTracks();
    bool cutSelectedTracks();
    bool pasteTracks();

    void setWillModifySequenceFn(WillModifySequenceFnType willModifySequenceFn) {
        _willModifySequenceFn = willModifySequenceFn;
    }
    void setDidSetCursorTickPosFn(DidSetCursorTickPosFnType didSetCursorTickPosFn) {
        _didSetCursorTickPosFn = didSetCursorTickPosFn;
    }
};

#endif  // ARRANGEMENTVIEWCONTROLLER_H
