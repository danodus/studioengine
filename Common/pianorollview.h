//
//  pianorollview.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2014-07-11.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#ifndef PIANOROLLVIEW_H
#define PIANOROLLVIEW_H

#include <boxview.h>
#include <combobox.h>
#include <image.h>
#include <imageview.h>
#include <labelview.h>
#include <scrollbar.h>
#include <scrollview.h>
#include <segmentedcontrol.h>
#include <slider.h>
#include <splitviewh.h>
#include <ui.h>
#include <view.h>

#include <array>
#include <memory>

#include "pianorollmainview.h"
#include "pianorollutilitiesview.h"

#define PIANO_ROLL_VIEW_UTILITIES_WIDTH 200.0f

class PianoRollView : public MDStudio::View {
    MDStudio::UI _ui;
    std::shared_ptr<View> _editionView;
    std::shared_ptr<MDStudio::BoxView> _editionBoxView;
    std::shared_ptr<MDStudio::SegmentedControl> _editionSegmentedControl;
    std::shared_ptr<MDStudio::Slider> _zoomSlider;
    std::shared_ptr<MDStudio::ImageView> _zoomOutImageView, _zoomInImageView;
    std::shared_ptr<MDStudio::SegmentedControl> _currentNoteSegmentedControl;
    std::shared_ptr<MDStudio::Button> _quantizeNewEventsButton;
    std::array<std::shared_ptr<MDStudio::Button>, STUDIO_MAX_CHANNELS> _visibleChannelButtons;
    std::shared_ptr<MDStudio::Button> _allVisibleChannelButton;
    std::array<std::shared_ptr<MDStudio::BoxView>, STUDIO_MAX_CHANNELS> _visibleChannelColorLabelBoxViews;
    std::shared_ptr<MDStudio::Button> _visibleControllerPaneButton;
    std::shared_ptr<MDStudio::Button> _visiblePropertiesPaneButton;
    std::shared_ptr<MDStudio::Button> _quantizeButton;
    std::shared_ptr<MDStudio::Button> _addFlagButton;
    std::shared_ptr<MDStudio::Button> _removeFlagButton;
    std::shared_ptr<MDStudio::Button> _removeAllFlagsButton;
    std::shared_ptr<MDStudio::Button> _goToPreviousFlagButton;
    std::shared_ptr<MDStudio::Button> _goToNextFlagButton;

    std::shared_ptr<PianoRollMainView> _mainView;
    std::shared_ptr<PianoRollUtilitiesView> _pianoRollUtilitiesView;
    std::shared_ptr<MDStudio::SplitViewH> _horizSplitView;

    std::shared_ptr<MDStudio::Image> _editionArrowImage, _editionSelectionImage, _editionDrawImage, _editionMoveImage,
        _editionResizeImage, _zoomOutImage, _zoomInImage;
    std::shared_ptr<MDStudio::Image> _editionSixteenthNote, _editionEighthNote, _editionQuarterNote, _editionHalfNote,
        _editionHalfDotNote, _editionWholeNote;
    std::shared_ptr<MDStudio::Image> _bottomPanelImage, _rightPanelImage;
    std::shared_ptr<MDStudio::Image> _allChannelsVisibilityImage;

   public:
    PianoRollView(std::string name, void* owner, MDStudio::Studio* studio, double eventTickWidth, float eventHeight);
    ~PianoRollView();

    std::shared_ptr<View> editionView() { return _editionView; }
    std::shared_ptr<MDStudio::SegmentedControl> editionSegmentedControl() { return _editionSegmentedControl; }
    std::shared_ptr<MDStudio::Slider> zoomSlider() { return _zoomSlider; }
    std::array<std::shared_ptr<MDStudio::Button>, STUDIO_MAX_CHANNELS> visibleChannelButtons() {
        return _visibleChannelButtons;
    }
    std::shared_ptr<MDStudio::Button> allVisibleChannelButton() { return _allVisibleChannelButton; }
    std::shared_ptr<MDStudio::Button> visibleControllerPaneButton() { return _visibleControllerPaneButton; }
    std::shared_ptr<MDStudio::Button> visiblePropertiesPaneButton() { return _visiblePropertiesPaneButton; }
    std::shared_ptr<PianoRollUtilitiesView> pianoRollUtilitiesView() { return _pianoRollUtilitiesView; }
    std::shared_ptr<MDStudio::SegmentedControl> currentNoteSegmentedControl() { return _currentNoteSegmentedControl; }
    std::shared_ptr<MDStudio::Button> quantizeNewEventsButton() { return _quantizeNewEventsButton; }
    std::shared_ptr<MDStudio::Button> quantizeButton() { return _quantizeButton; }
    std::shared_ptr<MDStudio::Button> addFlagButton() { return _addFlagButton; }
    std::shared_ptr<MDStudio::Button> removeFlagButton() { return _removeFlagButton; }
    std::shared_ptr<MDStudio::Button> removeAllFlagsButton() { return _removeAllFlagsButton; }
    std::shared_ptr<MDStudio::Button> goToPreviousFlagButton() { return _goToPreviousFlagButton; }
    std::shared_ptr<MDStudio::Button> goToNextFlagButton() { return _goToNextFlagButton; }
    std::shared_ptr<MDStudio::SplitViewH> horizSplitView() { return _horizSplitView; }
    std::shared_ptr<PianoRollMainView> mainView() { return _mainView; }

    void setFrame(MDStudio::Rect rect) override;
};

#endif  // PIANOROLLVIEW_H
