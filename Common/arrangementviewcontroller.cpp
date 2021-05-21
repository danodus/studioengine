//
//  arrangementviewcontroller.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-05-19.
//  Copyright (c) 2016-2021 Daniel Cliche. All rights reserved.
//

#include "arrangementviewcontroller.h"

#include <math.h>
#include <pasteboard.h>
#include <responderchain.h>

#include <algorithm>

#include "helpers.h"
#include "trackheaderview.h"
#include "trackview.h"

// ---------------------------------------------------------------------------------------------------------------------
ArrangementViewController::ArrangementViewController(std::shared_ptr<ArrangementView> arrangementView,
                                                     MelobaseCore::StudioController* studioController,
                                                     MelobaseCore::SequenceEditor* sequenceEditor,
                                                     PianoRollViewController* pianoRollViewController)
    : _arrangementView(arrangementView),
      _studioController(studioController),
      _sequenceEditor(sequenceEditor),
      _pianoRollViewController(pianoRollViewController) {
    using namespace std::placeholders;

    _willModifySequenceFn = nullptr;
    _didSetCursorTickPosFn = nullptr;

    _timeDivision = 480;
    _previousNbTracks = -1;
    _isFirstTrackShown = false;

    _lastPianoRollPos = MDStudio::makePoint(-1.0f, -1.0f);
    _lastEditionModeIndex = -1;

    _isDelegateNotified = true;

    arrangementView->tracksListView()->setNbRowsFn(
        std::bind(&ArrangementViewController::tracksListViewNbRows, this, _1));
    arrangementView->tracksListView()->setViewForRowFn(
        std::bind(&ArrangementViewController::tracksListViewViewForRow, this, _1, _2));
    arrangementView->tracksListView()->setDidSelectRowFn(
        std::bind(&ArrangementViewController::tracksListViewDidSelectRow, this, _1, _2));
    arrangementView->tracksListView()->setDidDeselectRowFn(
        std::bind(&ArrangementViewController::tracksListViewDidDeselectRow, this, _1, _2));
    arrangementView->tracksListView()->setDidSetFocusStateFn(
        std::bind(&ArrangementViewController::tracksListViewDidSetFocusState, this, _1, _2));

    arrangementView->trackHeadersListView()->setNbRowsFn(
        std::bind(&ArrangementViewController::trackHeadersListViewNbRows, this, _1));
    arrangementView->trackHeadersListView()->setViewForRowFn(
        std::bind(&ArrangementViewController::trackHeadersListViewViewForRow, this, _1, _2));
    arrangementView->trackHeadersListView()->setDidSelectRowFn(
        std::bind(&ArrangementViewController::trackHeadersListViewDidSelectRow, this, _1, _2));
    arrangementView->trackHeadersListView()->setDidDeselectRowFn(
        std::bind(&ArrangementViewController::trackHeadersListViewDidDeselectRow, this, _1, _2));
    arrangementView->trackHeadersListView()->setDidSetFocusStateFn(
        std::bind(&ArrangementViewController::trackHeadersListViewDidSetFocusState, this, _1, _2));

    arrangementView->trackHeadersOverlayListView()->setNbRowsFn(
        std::bind(&ArrangementViewController::trackHeadersListViewNbRows, this, _1));
    arrangementView->trackHeadersOverlayListView()->setViewForRowFn(
        std::bind(&ArrangementViewController::trackHeadersListViewViewForRow, this, _1, _2));

    arrangementView->tracksListView()->setDidPressUnhandledKeyFn(
        std::bind(&ArrangementViewController::tracksListViewDidPressUnhandledKey, this, _1, _2));
    arrangementView->trackHeadersListView()->setDidPressUnhandledKeyFn(
        std::bind(&ArrangementViewController::tracksListViewDidPressUnhandledKey, this, _1, _2));

    arrangementView->tracksScrollView()->setPosChangedFn(
        std::bind(&ArrangementViewController::tracksScrollViewPosChanged, this, _1, _2));
    arrangementView->trackHeadersScrollView()->setPosChangedFn(
        std::bind(&ArrangementViewController::trackHeadersScrollViewPosChanged, this, _1, _2));

    arrangementView->addTrackButton()->setClickedFn(std::bind(&ArrangementViewController::addTrackClicked, this, _1));
    arrangementView->deleteTrackButton()->setClickedFn(
        std::bind(&ArrangementViewController::deleteTrackClicked, this, _1));
    arrangementView->mergeTracksButton()->setClickedFn(
        std::bind(&ArrangementViewController::mergeTracksClicked, this, _1));
    arrangementView->moveTrackUpButton()->setClickedFn(
        std::bind(&ArrangementViewController::moveTrackUpClicked, this, _1));
    arrangementView->moveTrackDownButton()->setClickedFn(
        std::bind(&ArrangementViewController::moveTrackDownClicked, this, _1));
    arrangementView->showHideFirstTrackButton()->setStateDidChangeFn(
        std::bind(&ArrangementViewController::showHideFirstTrackStateDidChange, this, _1, _2));

    arrangementView->addTrackButton()->setIsEnabled(false);
    arrangementView->deleteTrackButton()->setIsEnabled(false);

    arrangementView->moveTrackUpButton()->setIsEnabled(false);
    arrangementView->moveTrackDownButton()->setIsEnabled(false);
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::sequenceInfos(TrackClipsView* sender, int* timeDivision,
                                              std::vector<unsigned int>* totalNbTicks,
                                              std::vector<unsigned int>* eotTickCounts, bool* areAbsTicks) {
    *timeDivision = _timeDivision;
    *totalNbTicks = _totalTickCounts;
    *eotTickCounts = _eotTickCounts;
    *areAbsTicks = _studioController->status() != MelobaseCore::StudioController::StudioControllerStatusRecording;
}

// ---------------------------------------------------------------------------------------------------------------------
int ArrangementViewController::nbEvents(TrackClipsView* sender, int track) {
    if (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusRecording) {
        return static_cast<int>(_studioController->sequencer()->sequence()->data.tracks[track].events.size());
    } else {
        if (_sequence) return static_cast<int>(_sequence->data.tracks[track]->clips[0]->events.size());
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
int ArrangementViewController::nbAnnotations(TrackClipsView* sender) {
    if (_sequence) return static_cast<int>(_sequence->annotations.size());
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::eventAtIndex(TrackClipsView* sender, int track, int index,
                                             std::shared_ptr<MelobaseCore::Event>* event) {
    if (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusRecording) {
        MDStudio::Event e = _studioController->sequencer()->sequence()->data.tracks[track].events[index];
        *event = std::make_shared<MelobaseCore::ChannelEvent>(e.type, e.channel, e.tickCount, 0, e.param1, e.param2, 0);
    } else {
        auto e = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(
            _sequence->data.tracks[track]->clips[0]->events[index]);
        std::shared_ptr<MelobaseCore::ChannelEvent> retEvent = std::make_shared<MelobaseCore::ChannelEvent>(
            e->type(), e->channel(), e->tickCount(), e->length(), e->param1(), e->param2(), e->param3());
        *event = retEvent;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::annotationAtIndex(TrackClipsView* sender, int index,
                                                  std::shared_ptr<MelobaseCore::SequenceAnnotation>* annotation) {
    *annotation = _sequence->annotations.at(index);
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::didSetCursorTickPos(TrackClipsView* sender, unsigned int cursorTickPos) {
    if (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusRecording) {
        _studioController->stopRecordingAndMetronome();
    } else {
        _studioController->stop();
    }

    if (cursorTickPos != _studioController->studio()->metronome()->tick())
        _studioController->studio()->moveMetronomeToTick(cursorTickPos);

    setCursorTickPos(cursorTickPos, false, true);

    if (_didSetCursorTickPosFn) _didSetCursorTickPosFn(this, cursorTickPos);
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::updateTracks() {
    int trackIndex = 0;
    if (_sequence) {
        _timeDivision = roundf(60.0f / (_sequence->data.tickPeriod * 125.0f));

        _eotTickCounts.resize(_sequence->data.tracks.size());
        _totalTickCounts.resize(_sequence->data.tracks.size());

        for (auto track : _sequence->data.tracks) {
            _totalTickCounts[trackIndex] = 0;
            bool isEOTFound = false;
            _eotTickCounts[trackIndex] = 0;

            if ((_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusRecording) &&
                (_studioController->isTrackArmed(trackIndex))) {
                _totalTickCounts[trackIndex] = _studioController->studio()->metronome()->tick();
            } else {
                for (auto event : track->clips[0]->events) {
                    auto channelEvent = std::dynamic_pointer_cast<MelobaseCore::ChannelEvent>(event);
                    _totalTickCounts[trackIndex] =
                        (channelEvent->tickCount() + channelEvent->length() > _totalTickCounts[trackIndex])
                            ? (channelEvent->tickCount() + channelEvent->length())
                            : _totalTickCounts[trackIndex];
                    if (channelEvent->type() == CHANNEL_EVENT_TYPE_META_END_OF_TRACK) {
                        _eotTickCounts[trackIndex] = channelEvent->tickCount();
                        isEOTFound = true;
                    }
                }
            }

            if (!isEOTFound) _eotTickCounts[trackIndex] = _totalTickCounts[trackIndex];

            ++trackIndex;
        }
    } else {
        _timeDivision = 480;
        _totalTickCounts.clear();
        _eotTickCounts.clear();
    }

    unsigned int maxTotalTickCount = 0;
    for (auto totalTickCount : _totalTickCounts)
        if (totalTickCount > maxTotalTickCount) maxTotalTickCount = totalTickCount;

    if (_sequence) {
        if (_previousNbTracks != _sequence->data.tracks.size()) {
            _arrangementView->tracksListView()->reload();
            _arrangementView->trackHeadersListView()->reload();
            _arrangementView->trackHeadersOverlayListView()->reload();
            _previousNbTracks = (int)_sequence->data.tracks.size();
        } else {
            // Update the track infos
            int trackIndex = _isFirstTrackShown ? 0 : 1;
            for (auto view : _arrangementView->trackHeadersOverlayListView()->subviews()) {
                std::shared_ptr<TrackHeaderView> trackHeaderView = (std::static_pointer_cast<TrackHeaderView>)(view);
                trackHeaderView->nameTextField()->setText(_sequence->data.tracks[trackIndex]->name, false);
                trackHeaderView->channelComboBox()->setSelectedRow(_sequence->data.tracks[trackIndex]->channel ==
                                                                           SEQUENCE_TRACK_MULTI_CHANNEL
                                                                       ? STUDIO_MAX_CHANNELS
                                                                       : _sequence->data.tracks[trackIndex]->channel);

                MDStudio::Color channelColor = MDStudio::grayColor;
                if (_sequence->data.tracks[trackIndex]->channel != SEQUENCE_TRACK_MULTI_CHANNEL) {
                    channelColor = channelColors[_sequence->data.tracks[trackIndex]->channel];
                    channelColor.red *= 0.75f;
                    channelColor.green *= 0.75f;
                    channelColor.blue *= 0.75f;
                }

                trackHeaderView->channelBoxView()->setBorderColor(channelColor);
                trackHeaderView->channelBoxView()->setFillColor(channelColor);

                if (_studioController->status() == MelobaseCore::StudioController::StudioControllerStatusRecording) {
                    trackHeaderView->nameTextField()->setIsEnabled(false);
                    trackHeaderView->armedButton()->setIsEnabled(false);
                    trackHeaderView->channelComboBox()->setIsEnabled(false);
                } else {
                    trackHeaderView->nameTextField()->setIsEnabled(true);
                    trackHeaderView->armedButton()->setIsEnabled(true);
                    trackHeaderView->channelComboBox()->setIsEnabled(true);
                }

                ++trackIndex;
            }
        }
    }

    _arrangementView->tracksScrollView()->setContentSize(
        MDStudio::makeSize(maxTotalTickCount * _arrangementView->eventTickWidth() + pianoRollCursorWidth,
                           (trackIndex - (!_isFirstTrackShown ? 1 : 0)) * 100.0f));
    _arrangementView->setDirty();
    _arrangementView->tracksScrollView()->contentView()->setDirty();

    _arrangementView->trackHeadersScrollView()->setContentSize(
        MDStudio::makeSize(_arrangementView->trackHeadersScrollView()->rect().size.width,
                           (trackIndex - (!_isFirstTrackShown ? 1 : 0)) * 100.0f));
    _arrangementView->trackHeadersScrollView()->contentView()->setDirty();
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::updateTrackControls() {
    if (!_sequence || _studioController->status() == MelobaseCore::StudioController::StudioControllerStatusPlaying ||
        _studioController->status() == MelobaseCore::StudioController::StudioControllerStatusRecording) {
        _arrangementView->addTrackButton()->setIsEnabled(false);
        _arrangementView->deleteTrackButton()->setIsEnabled(false);
        _arrangementView->mergeTracksButton()->setIsEnabled(false);
        _arrangementView->moveTrackUpButton()->setIsEnabled(false);
        _arrangementView->moveTrackDownButton()->setIsEnabled(false);
    } else {
        _arrangementView->addTrackButton()->setIsEnabled(true);
        _arrangementView->deleteTrackButton()->setIsEnabled(true);
        auto selectedRows = _arrangementView->tracksListView()->selectedRows();
        _arrangementView->mergeTracksButton()->setIsEnabled(
            (selectedRows.size() > 1) &&
            (!_isFirstTrackShown || (std::find(selectedRows.begin(), selectedRows.end(), 0) == selectedRows.end())));
        if (_arrangementView->tracksListView()->selectedRows().empty() ||
            _arrangementView->tracksListView()->selectedRows().size() > 1) {
            _arrangementView->moveTrackUpButton()->setIsEnabled(false);
            _arrangementView->moveTrackDownButton()->setIsEnabled(false);
        } else {
            _arrangementView->moveTrackUpButton()->setIsEnabled(_arrangementView->tracksListView()->selectedRow() >
                                                                (_isFirstTrackShown ? 1 : 0));
            if (_isFirstTrackShown && (_arrangementView->tracksListView()->selectedRow() == 0)) {
                _arrangementView->moveTrackDownButton()->setIsEnabled(false);
            } else {
                _arrangementView->moveTrackDownButton()->setIsEnabled(
                    _arrangementView->tracksListView()->selectedRow() <
                    (_sequence->data.tracks.size() - (_isFirstTrackShown ? 0 : 1)) - 1);
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::setSequence(std::shared_ptr<MelobaseCore::Sequence> sequence) {
    _lastPianoRollPos = MDStudio::makePoint(-1.0f, -1.0f);
    _lastEditionModeIndex = -1;
    _sequence = sequence;
    _sequenceEditor->setSequence(sequence);
    _previousNbTracks = -1;
    _arrangementView->tracksListView()->reload();
    _arrangementView->trackHeadersListView()->reload();
    _arrangementView->trackHeadersOverlayListView()->reload();
    updateTracks();
    updateTrackControls();
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int ArrangementViewController::tracksListViewNbRows(MDStudio::ListView* sender) {
    if (!_sequence || _sequence->data.tracks.empty()) return 0;

    return static_cast<unsigned int>(_sequence->data.tracks.size() - (_isFirstTrackShown ? 0 : 1));
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MDStudio::View> ArrangementViewController::tracksListViewViewForRow(MDStudio::ListView* sender,
                                                                                    int row) {
    using namespace std::placeholders;

    auto track = _sequence->data.tracks[row + (_isFirstTrackShown ? 0 : 1)];
    std::shared_ptr<TrackView> trackView = std::make_shared<TrackView>(
        "trackView", this, _studioController->studio(), row + (_isFirstTrackShown ? 0 : 1), track->channel,
        _arrangementView->eventTickWidth(), _arrangementView->eventHeight());
    trackView->trackClipsView()->setSequenceInfosFn(
        std::bind(&ArrangementViewController::sequenceInfos, this, _1, _2, _3, _4, _5));
    trackView->trackClipsView()->setNbEventsFn(std::bind(&ArrangementViewController::nbEvents, this, _1, _2));
    trackView->trackClipsView()->setEventAtIndexFn(
        std::bind(&ArrangementViewController::eventAtIndex, this, _1, _2, _3, _4));
    trackView->trackClipsView()->setNbAnnotationsFn(std::bind(&ArrangementViewController::nbAnnotations, this, _1));
    trackView->trackClipsView()->setAnnotationAtIndexFn(
        std::bind(&ArrangementViewController::annotationAtIndex, this, _1, _2, _3));
    trackView->trackClipsView()->setDidSetCursorTickPosFn(
        std::bind(&ArrangementViewController::didSetCursorTickPos, this, _1, _2));

    trackView->trackClipsView()->setCursorTickPos(_studioController->studio()->metronome()->tick(), false);

    return trackView;
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::tracksListViewDidSelectRow(MDStudio::ListView* sender, int row) {
    std::shared_ptr<MDStudio::View> view = sender->subviews()[row];
    std::shared_ptr<TrackView> trackView = (std::static_pointer_cast<TrackView>)(view);
    trackView->setIsHighlighted(true);

    std::shared_ptr<MDStudio::ListItemView> trackHeaderView =
        (std::static_pointer_cast<MDStudio::ListItemView>)(_arrangementView->trackHeadersListView()->subviews()[row]);
    trackHeaderView->setIsHighlighted(true);

    if (_isDelegateNotified) {
        _arrangementView->trackHeadersListView()->setSelectedRow(-1, false);
        for (auto selectedRow : sender->selectedRows())
            _arrangementView->trackHeadersListView()->setSelectedRow(selectedRow, false, false);
    }

    if (sender->selectedRows().size() > 1) {
        _pianoRollViewController->setTrackIndex(-1);
    } else {
        _pianoRollViewController->setTrackIndex(row + (_isFirstTrackShown ? 0 : 1));
    }

    // Restore the last piano roll position
    if (_lastPianoRollPos.x >= 0.0f) {
        _pianoRollViewController->setPianoRollEventsScrollViewPos(_lastPianoRollPos);
        _lastPianoRollPos = MDStudio::makePoint(-1.0f, -1.0f);
    }

    // Restore the edition mode
    if (_lastEditionModeIndex >= 0) {
        _pianoRollViewController->setEditionModeIndex(_lastEditionModeIndex);
        _lastEditionModeIndex = -1;
    }

    // Scroll to visible row
    if (_isDelegateNotified) _arrangementView->tracksScrollView()->scrollToVisibleRectV(view->rect());

    updateTrackControls();
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::tracksListViewDidDeselectRow(MDStudio::ListView* sender, int row) {
    std::shared_ptr<MDStudio::View> view = sender->subviews()[row];
    std::shared_ptr<TrackView> trackView = (std::static_pointer_cast<TrackView>)(view);
    trackView->setIsHighlighted(false);

    std::shared_ptr<MDStudio::ListItemView> trackHeaderView =
        (std::static_pointer_cast<MDStudio::ListItemView>)(_arrangementView->trackHeadersListView()->subviews()[row]);
    trackHeaderView->setIsHighlighted(false);

    if (_isDelegateNotified) {
        _arrangementView->trackHeadersListView()->setSelectedRow(-1, false);
        for (auto selectedRow : sender->selectedRows())
            _arrangementView->trackHeadersListView()->setSelectedRow(selectedRow, false, false);
    }

    _lastPianoRollPos = _pianoRollViewController->pianoRollEventsScrollViewPos();
    _lastEditionModeIndex = _pianoRollViewController->editionModeIndex();

    _pianoRollViewController->setTrackIndex(-1);
    _arrangementView->deleteTrackButton()->setIsEnabled(false);

    updateTrackControls();
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::tracksListViewDidSetFocusState(MDStudio::ListView* sender, bool state) {
    for (unsigned int i = 0; i < sender->subviews().size(); ++i) {
        std::shared_ptr<MDStudio::View> view = sender->subviews()[i];
        std::shared_ptr<TrackView> trackView = (std::static_pointer_cast<TrackView>)(view);
        trackView->setFocusState(state);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool ArrangementViewController::tracksListViewDidPressUnhandledKey(MDStudio::ListView* sender, unsigned int key) {
    if (key == KEY_DELETE) {
        removeSelectedTracks();
        return true;
    } else if (key == KEY_LEFT) {
        _pianoRollViewController->goToPreviousMeasure();
        return true;
    } else if (key == KEY_RIGHT) {
        _pianoRollViewController->goToNextMeasure();
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int ArrangementViewController::trackHeadersListViewNbRows(MDStudio::ListView* sender) {
    if (!_sequence || _sequence->data.tracks.empty()) return 0;

    return static_cast<unsigned int>(_sequence->data.tracks.size() - (_isFirstTrackShown ? 0 : 1));
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MDStudio::View> ArrangementViewController::trackHeadersListViewViewForRow(MDStudio::ListView* sender,
                                                                                          int row) {
    if (sender != _arrangementView->trackHeadersOverlayListView().get()) {
        auto trackHeaderView = std::make_shared<MDStudio::ListItemView>("trackHeaderView", this, "");
        trackHeaderView->setFocusState(sender->hasFocus());
        return trackHeaderView;
    }

    using namespace std::placeholders;

    std::vector<int> armedTrackIndices = _studioController->sequencer()->armedTrackIndices();
    std::vector<int> muteTrackIndices = _studioController->sequencer()->muteTrackIndices();
    int soloTrackIndex = _studioController->sequencer()->soloTrackIndex();

    auto track = _sequence->data.tracks[row + (_isFirstTrackShown ? 0 : 1)];
    std::shared_ptr<TrackHeaderView> trackHeaderView =
        std::make_shared<TrackHeaderView>("trackHeaderView", this, _sequence.get(), row + (_isFirstTrackShown ? 0 : 1));

    // We do not display the controls for the first track
    trackHeaderView->setAreControlsVisible((_isFirstTrackShown && (row == 0)) ? false : true);

    // Allow the first track to be armed
    trackHeaderView->armedButton()->setIsVisible(true);

    trackHeaderView->armedButton()->setState(std::find(armedTrackIndices.begin(), armedTrackIndices.end(),
                                                       row + (_isFirstTrackShown ? 0 : 1)) != armedTrackIndices.end());
    trackHeaderView->muteButton()->setState(std::find(muteTrackIndices.begin(), muteTrackIndices.end(),
                                                      row + (_isFirstTrackShown ? 0 : 1)) != muteTrackIndices.end());
    trackHeaderView->soloButton()->setState(row + (_isFirstTrackShown ? 0 : 1) == soloTrackIndex);
    trackHeaderView->nameTextField()->setTextDidChangeFn(
        std::bind(&ArrangementViewController::trackHeaderNameTextFieldNameDidChange, this, _1, _2));
    trackHeaderView->armedButton()->setStateDidChangeFn(
        std::bind(&ArrangementViewController::trackHeaderArmedStateDidChange, this, _1, _2));
    trackHeaderView->muteButton()->setStateDidChangeFn(
        std::bind(&ArrangementViewController::trackHeaderMuteStateDidChange, this, _1, _2));
    trackHeaderView->soloButton()->setStateDidChangeFn(
        std::bind(&ArrangementViewController::trackHeaderSoloStateDidChange, this, _1, _2));
    trackHeaderView->channelComboBox()->setNbRowsFn(
        std::bind(&ArrangementViewController::trackHeaderChannelNbRows, this, _1));
    trackHeaderView->channelComboBox()->setViewForRowFn(
        std::bind(&ArrangementViewController::trackHeaderChannelViewForRow, this, _1, _2));
    trackHeaderView->channelComboBox()->setDidSelectRowFn(
        std::bind(&ArrangementViewController::trackHeaderChannelDidSelectRow, this, _1, _2));
    trackHeaderView->channelComboBox()->setDidHoverRowFn(
        std::bind(&ArrangementViewController::trackHeaderChannelDidHoverRow, this, _1, _2));
    trackHeaderView->channelComboBox()->setDidConfirmRowSelectionFn(
        std::bind(&ArrangementViewController::trackHeaderChannelDidConfirmRowSelection, this, _1, _2));
    trackHeaderView->channelComboBox()->reload();

    auto channelRow = track->channel == SEQUENCE_TRACK_MULTI_CHANNEL ? STUDIO_MAX_CHANNELS : track->channel;
    trackHeaderView->channelComboBox()->setSelectedRow(channelRow, false);

    std::string title = (channelRow < STUDIO_MAX_CHANNELS) ? std::to_string(channelRow + 1) : "MULTI";
    trackHeaderView->channelComboBox()->setTitle(title);

    MDStudio::Color channelColor = MDStudio::grayColor;
    if (track->channel != SEQUENCE_TRACK_MULTI_CHANNEL) {
        channelColor = channelColors[track->channel];
        channelColor.red *= 0.75f;
        channelColor.green *= 0.75f;
        channelColor.blue *= 0.75f;
    }

    trackHeaderView->channelBoxView()->setBorderColor(channelColor);
    trackHeaderView->channelBoxView()->setFillColor(channelColor);

    return trackHeaderView;
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::trackHeadersListViewDidSelectRow(MDStudio::ListView* sender, int row) {
    std::shared_ptr<MDStudio::View> view = sender->subviews()[row];

    _isDelegateNotified = false;
    _arrangementView->tracksListView()->setSelectedRow(-1);
    for (auto selectedRow : sender->selectedRows())
        _arrangementView->tracksListView()->setSelectedRow(selectedRow, true, false);
    _isDelegateNotified = true;

    // Scroll to visible row
    _arrangementView->tracksScrollView()->scrollToVisibleRectV(view->rect());
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::trackHeadersListViewDidDeselectRow(MDStudio::ListView* sender, int row) {
    _isDelegateNotified = false;
    _arrangementView->tracksListView()->setSelectedRow(-1);
    for (auto selectedRow : sender->selectedRows())
        _arrangementView->tracksListView()->setSelectedRow(selectedRow, true, false);
    _isDelegateNotified = true;
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::trackHeadersListViewDidSetFocusState(MDStudio::ListView* sender, bool state) {
    for (unsigned int i = 0; i < sender->subviews().size(); ++i) {
        std::shared_ptr<MDStudio::View> view = sender->subviews()[i];
        std::shared_ptr<MDStudio::ListItemView> trackHeaderView =
            (std::static_pointer_cast<MDStudio::ListItemView>)(view);
        trackHeaderView->setFocusState(state);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::addTrackClicked(MDStudio::Button* sender) {
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    int row = _arrangementView->tracksListView()->selectedRow();
    auto track = std::make_shared<MelobaseCore::Track>();

    // Find a free channel for the track and assign it if possible
    for (int trackChannel = 0; trackChannel < STUDIO_MAX_CHANNELS; ++trackChannel) {
        // Do not automatically select a drum kit channel
        if (trackChannel == 9) continue;

        // Find if the track has this channel. If not, we use it.
        bool isInUse = false;
        bool isFirst = !_isFirstTrackShown;
        for (auto& t : _sequence->data.tracks) {
            if (isFirst) {
                isFirst = false;
                continue;
            }
            if (t->channel == trackChannel) {
                isInUse = true;
                break;
            }
        }

        // If not currently in use, we do so
        if (!isInUse) {
            track->channel = trackChannel;
            break;
        }
    }

    track->clips[0]->events.push_back(
        std::make_shared<MelobaseCore::ChannelEvent>(CHANNEL_EVENT_TYPE_META_END_OF_TRACK, 0, 10 * 4 * 480, 0, -1, -1));

    _sequenceEditor->addTrack(track,
                              row >= 0 ? row + 1 + (_isFirstTrackShown ? 0 : 1) : (int)_sequence->data.tracks.size());

    MDStudio::Platform::sharedInstance()->invoke([=] {
        if (_arrangementView->trackHeadersListView()->isInChain())
            _arrangementView->trackHeadersListView()->captureFocus();
        _arrangementView->trackHeadersListView()->setSelectedRow(
            row >= 0 ? row + 1 : (int)_sequence->data.tracks.size() - (_isFirstTrackShown ? 1 : 2));
    });
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::deleteTrackClicked(MDStudio::Button* sender) { removeSelectedTracks(); }

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::mergeTracksClicked(MDStudio::Button* sender) {
    std::shared_ptr<MelobaseCore::Track> track;
    std::vector<std::shared_ptr<MelobaseCore::Track>> tracks;
    for (int i = 0; i < _arrangementView->trackHeadersListView()->nbRows(); ++i) {
        auto listItemView =
            std::dynamic_pointer_cast<MDStudio::ListItemView>(_arrangementView->trackHeadersListView()->viewAtRow(i));
        if (listItemView->isHighlighted()) {
            auto trackHeaderView = std::dynamic_pointer_cast<TrackHeaderView>(
                _arrangementView->trackHeadersOverlayListView()->viewAtRow(i));
            if (track) {
                tracks.push_back(_sequence->data.tracks[trackHeaderView->trackIndex()]);
            } else {
                track = _sequence->data.tracks[trackHeaderView->trackIndex()];
            }
        }
    }

    if (tracks.size() > 0) _sequenceEditor->mergeTracks(track, tracks);
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::moveTrackUpClicked(MDStudio::Button* sender) {
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    if (_arrangementView->tracksListView()->selectedRow() >= 0) {
        int trackToMoveIndex = _arrangementView->tracksListView()->selectedRow() + (_isFirstTrackShown ? 0 : 1);

        if (trackToMoveIndex > 1) {
            auto trackToMove = _sequence->data.tracks[trackToMoveIndex];
            _sequenceEditor->undoManager()->beginGroup();
            _sequenceEditor->removeTrack(trackToMove);
            _sequenceEditor->addTrack(trackToMove, trackToMoveIndex - 1);
            _sequenceEditor->undoManager()->endGroup();

            // Select the track
            _arrangementView->tracksListView()->setSelectedRow(trackToMoveIndex - 1 - (_isFirstTrackShown ? 0 : 1));
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::moveTrackDownClicked(MDStudio::Button* sender) {
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    if (_arrangementView->tracksListView()->selectedRow() >= 0) {
        int trackToMoveIndex = _arrangementView->tracksListView()->selectedRow() + (_isFirstTrackShown ? 0 : 1);

        if ((trackToMoveIndex > 0) && (trackToMoveIndex < _sequence->data.tracks.size() - 1)) {
            auto trackToMove = _sequence->data.tracks[trackToMoveIndex];
            _sequenceEditor->undoManager()->beginGroup();
            _sequenceEditor->removeTrack(trackToMove);
            _sequenceEditor->addTrack(trackToMove, trackToMoveIndex + 1);
            _sequenceEditor->undoManager()->endGroup();

            // Select the track
            _arrangementView->tracksListView()->setSelectedRow(trackToMoveIndex + 1 - (_isFirstTrackShown ? 0 : 1));
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::showHideFirstTrackStateDidChange(MDStudio::Button* sender, bool state) {
    _isFirstTrackShown = state;
    _previousNbTracks = -1;
    updateTracks();
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::setCursorTickPos(unsigned int tickPos, bool isScrollingToCenteredCursor,
                                                 bool isScrollingToVisibleCursor) {
    for (auto view : _arrangementView->tracksListView()->subviews()) {
        std::shared_ptr<TrackView> trackView = (std::static_pointer_cast<TrackView>)(view);
        trackView->trackClipsView()->setCursorTickPos(tickPos, false);
    }

    if (isScrollingToCenteredCursor && !_arrangementView->tracksScrollView()->isScrollingH()) {
        _arrangementView->scrollToCenteredCursor(tickPos);
    } else if (isScrollingToVisibleCursor) {
        _arrangementView->scrollToVisibleCursor(tickPos);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::handleEventRepeat() {
    for (auto view : _arrangementView->tracksListView()->subviews()) {
        std::shared_ptr<TrackView> trackView = (std::static_pointer_cast<TrackView>)(view);

        if (trackView->trackClipsView()->isCaptured()) {
            MDStudio::UIEvent lastEvent = _arrangementView->responderChain()->lastEvent();
            if (lastEvent.type == MDStudio::MOUSE_MOVED_UIEVENT) trackView->trackClipsView()->handleEvent(&lastEvent);

            // We should have only one captured track, so we can break
            break;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::tracksScrollViewPosChanged(MDStudio::ScrollView* sender, MDStudio::Point pos) {
    if (sender->isResizingContent()) return;

    using namespace std::placeholders;
    MDStudio::Point p = _arrangementView->trackHeadersScrollView()->pos();
    p.y = -pos.y;

    _arrangementView->tracksScrollView()->setPosChangedFn(nullptr);
    _arrangementView->trackHeadersScrollView()->setPos(p);
    _arrangementView->tracksScrollView()->setPosChangedFn(
        std::bind(&ArrangementViewController::tracksScrollViewPosChanged, this, _1, _2));
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::trackHeadersScrollViewPosChanged(MDStudio::ScrollView* sender, MDStudio::Point pos) {
    if (sender->isResizingContent()) return;

    using namespace std::placeholders;

    MDStudio::Point p = _arrangementView->tracksScrollView()->pos();
    p.y = -pos.y;

    _arrangementView->trackHeadersScrollView()->setPosChangedFn(nullptr);
    _arrangementView->tracksScrollView()->setPos(p);
    _arrangementView->trackHeadersScrollView()->setPosChangedFn(
        std::bind(&ArrangementViewController::trackHeadersScrollViewPosChanged, this, _1, _2));
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::trackHeaderNameTextFieldNameDidChange(MDStudio::TextField* sender, std::string text) {
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    int trackIndex = _isFirstTrackShown ? 0 : 1;
    for (auto view : _arrangementView->trackHeadersOverlayListView()->subviews()) {
        std::shared_ptr<TrackHeaderView> trackHeaderView = (std::static_pointer_cast<TrackHeaderView>)(view);
        if (trackHeaderView->nameTextField().get() == sender) break;
        ++trackIndex;
    }
    _sequenceEditor->setTrackName(_sequenceEditor->sequence()->data.tracks[trackIndex], text);

    MDStudio::Platform::sharedInstance()->invoke([=] {
        if (_arrangementView->trackHeadersListView()->isInChain())
            _arrangementView->trackHeadersListView()->captureFocus();
    });
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::trackHeaderArmedStateDidChange(MDStudio::Button* sender, bool state) {
    std::vector<int> armedTrackIndices;

    int trackIndex = _isFirstTrackShown ? 0 : 1;
    for (auto view : _arrangementView->trackHeadersOverlayListView()->subviews()) {
        std::shared_ptr<TrackHeaderView> trackHeaderView = (std::static_pointer_cast<TrackHeaderView>)(view);
        if (trackHeaderView->armedButton()->state()) armedTrackIndices.push_back(trackIndex);
        ++trackIndex;
    }

    _studioController->sequencer()->setArmedTrackIndices(armedTrackIndices);
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::trackHeaderMuteStateDidChange(MDStudio::Button* sender, bool state) {
    std::vector<int> muteTrackIndices;

    int trackIndex = _isFirstTrackShown ? 0 : 1;
    for (auto view : _arrangementView->trackHeadersOverlayListView()->subviews()) {
        std::shared_ptr<TrackHeaderView> trackHeaderView = (std::static_pointer_cast<TrackHeaderView>)(view);
        if (trackHeaderView->muteButton()->state()) muteTrackIndices.push_back(trackIndex);
        ++trackIndex;
    }

    _studioController->sequencer()->setMuteTrackIndices(muteTrackIndices);
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::trackHeaderSoloStateDidChange(MDStudio::Button* sender, bool state) {
    // Clear all the solo states except the sender
    for (auto view : _arrangementView->trackHeadersOverlayListView()->subviews()) {
        std::shared_ptr<TrackHeaderView> trackHeaderView = (std::static_pointer_cast<TrackHeaderView>)(view);
        if (trackHeaderView->soloButton().get() != sender) trackHeaderView->soloButton()->setState(false, false);
    }

    int trackIndex = -1;

    if (state) {
        trackIndex = _isFirstTrackShown ? 0 : 1;
        // Find the track index of the sender
        for (auto view : _arrangementView->trackHeadersOverlayListView()->subviews()) {
            std::shared_ptr<TrackHeaderView> trackHeaderView = (std::static_pointer_cast<TrackHeaderView>)(view);
            if (trackHeaderView->soloButton()->state()) break;
            ++trackIndex;
        }
    }

    _studioController->sequencer()->setSoloTrackIndex(trackIndex);
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int ArrangementViewController::trackHeaderChannelNbRows(MDStudio::ComboBox* sender) {
    return STUDIO_MAX_CHANNELS + 1;
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MDStudio::View> ArrangementViewController::trackHeaderChannelViewForRow(MDStudio::ComboBox* sender,
                                                                                        int row) {
    std::string title = (row < STUDIO_MAX_CHANNELS) ? std::to_string(row + 1) : "MULTI";

    std::shared_ptr<MDStudio::ListItemView> listItemView = std::shared_ptr<MDStudio::ListItemView>(
        new MDStudio::ListItemView("listItemView" + std::to_string(row), nullptr, title));
    return listItemView;
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::trackHeaderChannelDidSelectRow(MDStudio::ComboBox* sender, int row) {
    int trackIndex = _isFirstTrackShown ? 0 : 1;
    for (auto view : _arrangementView->trackHeadersOverlayListView()->subviews()) {
        std::shared_ptr<TrackHeaderView> trackHeaderView = (std::static_pointer_cast<TrackHeaderView>)(view);
        if (trackHeaderView->channelComboBox().get() == sender) break;
        ++trackIndex;
    }

    std::string title = (row < STUDIO_MAX_CHANNELS) ? std::to_string(row + 1) : "MULTI";
    sender->setTitle(title);
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::trackHeaderChannelDidHoverRow(MDStudio::ComboBox* sender, int row) {
    for (unsigned int i = 0; i < sender->rowViews().size(); ++i) {
        std::shared_ptr<MDStudio::View> view = sender->rowViews()[i];
        std::shared_ptr<MDStudio::ListItemView> listItemView = (std::static_pointer_cast<MDStudio::ListItemView>)(view);
        listItemView->setIsHovering(i == row);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::trackHeaderChannelDidConfirmRowSelection(MDStudio::ComboBox* sender, int row) {
    sender->close();

    int trackIndex = _isFirstTrackShown ? 0 : 1;
    for (auto view : _arrangementView->trackHeadersOverlayListView()->subviews()) {
        std::shared_ptr<TrackHeaderView> trackHeaderView = (std::static_pointer_cast<TrackHeaderView>)(view);
        if (trackHeaderView->channelComboBox().get() == sender) break;
        ++trackIndex;
    }

    int channel = (row >= STUDIO_MAX_CHANNELS) ? SEQUENCE_TRACK_MULTI_CHANNEL : row;

    _sequenceEditor->undoManager()->beginGroup();

    auto track = _sequenceEditor->sequence()->data.tracks[trackIndex];

    _sequenceEditor->setTrackChannel(track, channel);

    // Rechannelize the events in the track
    if (channel != SEQUENCE_TRACK_MULTI_CHANNEL) {
        _sequenceEditor->setChannelOfEvents(track, track->clips[0]->events, channel);
    }

    _sequenceEditor->undoManager()->endGroup();

    _arrangementView->trackHeadersListView()->captureFocus();
}

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::clearTrackSelection() { _arrangementView->trackHeadersListView()->setSelectedRow(-1); }

// ---------------------------------------------------------------------------------------------------------------------
void ArrangementViewController::selectTrack(int trackIndex) {
    if ((trackIndex == 0) && !_isFirstTrackShown) return;

    _arrangementView->tracksListView()->setSelectedRow(_isFirstTrackShown ? trackIndex : trackIndex - 1);
}

// ---------------------------------------------------------------------------------------------------------------------
bool ArrangementViewController::deleteSelectedTracks() {
    if (!_arrangementView->trackHeadersListView()->hasFocus()) return false;

    return removeSelectedTracks();
}

// ---------------------------------------------------------------------------------------------------------------------
bool ArrangementViewController::copySelectedTracks() {
    if (!_arrangementView->trackHeadersListView()->hasFocus()) return false;

    auto selectedRows = _arrangementView->trackHeadersListView()->selectedRows();
    if (selectedRows.size() == 0) return false;

    std::vector<std::shared_ptr<MelobaseCore::Track>> selectedTracks;

    _sequenceEditor->undoManager()->beginGroup();
    for (auto selectedRow : selectedRows) {
        auto selectedTrack = _sequence->data.tracks[selectedRow + (_isFirstTrackShown ? 0 : 1)];
        selectedTracks.push_back(selectedTrack);
    }
    _sequenceEditor->undoManager()->endGroup();

    MDStudio::Pasteboard::sharedInstance()->setContent(selectedTracks);

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool ArrangementViewController::cutSelectedTracks() {
    if (!_arrangementView->trackHeadersListView()->hasFocus()) return false;

    auto selectedRows = _arrangementView->trackHeadersListView()->selectedRows();
    if (selectedRows.size() == 0) return false;

    if (_willModifySequenceFn) _willModifySequenceFn(this);

    std::vector<std::shared_ptr<MelobaseCore::Track>> selectedTracks;

    for (auto selectedRow : selectedRows) {
        auto selectedTrack = _sequence->data.tracks[selectedRow + (_isFirstTrackShown ? 0 : 1)];
        selectedTracks.push_back(selectedTrack);
    }

    _sequenceEditor->undoManager()->beginGroup();
    for (auto selectedTrack : selectedTracks) _sequenceEditor->removeTrack(selectedTrack);
    _sequenceEditor->undoManager()->endGroup();

    MDStudio::Pasteboard::sharedInstance()->setContent(selectedTracks);

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool ArrangementViewController::pasteTracks() {
    auto pasteboard = MDStudio::Pasteboard::sharedInstance();
    if (!pasteboard->isContentAvailable()) return false;

    if (!pasteboard->content().is<std::vector<std::shared_ptr<MelobaseCore::Track>>>()) return false;

    // If the sequence is not multi-tracks, abort the operation
    if (_sequence->data.format != SEQUENCE_DATA_FORMAT_MULTI_TRACK) {
        MDStudio::Platform::sharedInstance()->beep();
        return true;
    }

    if (_willModifySequenceFn) _willModifySequenceFn(this);

    auto selectedRow = _arrangementView->trackHeadersListView()->selectedRow();

    auto tracks = pasteboard->content().as<std::vector<std::shared_ptr<MelobaseCore::Track>>>();

    _sequenceEditor->undoManager()->beginGroup();
    int firstInsertIndex =
        selectedRow >= 0 ? selectedRow + 1 + (_isFirstTrackShown ? 0 : 1) : (int)_sequence->data.tracks.size();
    int insertIndex = firstInsertIndex;

    for (auto track : tracks) {
        // Add a copy of the track
        _sequenceEditor->addTrack(track->copy(), insertIndex);
        insertIndex++;
    }
    _sequenceEditor->undoManager()->endGroup();

    // After the table reload is performed, select the new tracks
    MDStudio::Platform::sharedInstance()->invoke([=] {
        if (_arrangementView->trackHeadersListView()->isInChain())
            _arrangementView->trackHeadersListView()->captureFocus();
        for (int index = firstInsertIndex; index < insertIndex; ++index)
            _arrangementView->trackHeadersListView()->setSelectedRow(index - (_isFirstTrackShown ? 0 : 1), true, false);
    });

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool ArrangementViewController::removeSelectedTracks() {
    auto selectedRows = _arrangementView->trackHeadersListView()->selectedRows();
    if (selectedRows.size() == 0) return false;

    if (_willModifySequenceFn) _willModifySequenceFn(this);

    std::vector<std::shared_ptr<MelobaseCore::Track>> selectedTracks;

    for (auto selectedRow : selectedRows) {
        auto selectedTrack = _sequence->data.tracks[selectedRow + (_isFirstTrackShown ? 0 : 1)];
        selectedTracks.push_back(selectedTrack);
    }

    _sequenceEditor->undoManager()->beginGroup();
    for (auto selectedTrack : selectedTracks) _sequenceEditor->removeTrack(selectedTrack);
    _sequenceEditor->undoManager()->endGroup();

    return true;
}
