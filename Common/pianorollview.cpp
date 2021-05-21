//
//  pianorollview.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2014-07-11.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#include "pianorollview.h"

#include <draw.h>
#include <math.h>
#include <platform.h>

#include "helpers.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
PianoRollView::PianoRollView(std::string name, void* owner, Studio* studio, double eventTickWidth, float eventHeight)
    : View(name, owner) {
    _ui.loadUI(this, MDStudio::Platform::sharedInstance()->resourcesPath() + "/PianoRollView.lua");

    _editionArrowImage = std::make_shared<Image>("EditionArrow@2x.png");
    _editionSelectionImage = std::make_shared<Image>("EditionSelection@2x.png");
    _editionDrawImage = std::make_shared<Image>("EditionDraw@2x.png");
    _editionMoveImage = std::make_shared<Image>("EditionMove@2x.png");
    _editionResizeImage = std::make_shared<Image>("EditionResize@2x.png");
    _zoomOutImage = std::make_shared<Image>("ZoomOut@2x.png");
    _zoomInImage = std::make_shared<Image>("ZoomIn@2x.png");
    _allChannelsVisibilityImage = std::make_shared<Image>("AllChannelsVisibility@2x.png");

    _pianoRollUtilitiesView =
        std::shared_ptr<PianoRollUtilitiesView>(new PianoRollUtilitiesView("pianoRollUtilitiesView", owner));
    _pianoRollUtilitiesView->setIsVisible(false);

    _editionView = std::shared_ptr<View>(new View("editionView", owner));
    _editionBoxView = std::shared_ptr<BoxView>(new BoxView("editionBoxView", owner));
    _editionBoxView->setFillColor(veryDimGrayColor);
    _editionBoxView->setBorderColor(veryDimGrayColor);

    std::vector<std::shared_ptr<Image>> editionImages;
    editionImages.push_back(_editionArrowImage);
    editionImages.push_back(_editionSelectionImage);
    editionImages.push_back(_editionDrawImage);
    editionImages.push_back(_editionMoveImage);
    editionImages.push_back(_editionResizeImage);

    std::vector<Any> editionItems;
    for (auto image : editionImages) {
        editionItems.push_back(image);
    }
    _editionSegmentedControl =
        std::shared_ptr<SegmentedControl>(new SegmentedControl("editionSegmentedControl", owner, editionItems));
    _editionView->addSubview(_editionBoxView);
    _editionView->addSubview(_editionSegmentedControl);

    _zoomSlider = std::shared_ptr<Slider>(new Slider("zoomSlider", owner, 0.0, 1.0, 0.4));
    _editionView->addSubview(_zoomSlider);

    _zoomOutImageView = std::shared_ptr<ImageView>(new ImageView("zoomOutImageView", owner, _zoomOutImage, false));
    _zoomInImageView = std::shared_ptr<ImageView>(new ImageView("zoomInImageView", owner, _zoomInImage, false));
    _editionView->addSubview(_zoomOutImageView);
    _editionView->addSubview(_zoomInImageView);

    for (int channel = 0; channel < STUDIO_MAX_CHANNELS; ++channel) {
        std::shared_ptr<Button> button = std::shared_ptr<Button>(
            new Button("visibleChannelButton" + std::to_string(channel), owner, std::to_string(channel + 1), nullptr));
        button->setFont(SystemFonts::sharedInstance()->semiboldFontSmall());
        button->setType(Button::CustomCheckBoxButtonType);
        button->setHighlightColor(grayColor);
        button->setState(true);
        _visibleChannelButtons[channel] = button;
        _editionView->addSubview(button);

        auto colorLabelBoxView = std::make_shared<BoxView>("visibleChannelColorLabelBoxView", owner);
        Color color = channelColors[channel];
        color.red *= 0.75f;
        color.green *= 0.75f;
        color.blue *= 0.75f;
        colorLabelBoxView->setBorderColor(color);
        colorLabelBoxView->setFillColor(color);
        colorLabelBoxView->setCornerRadius(0.0f);
        _visibleChannelColorLabelBoxViews[channel] = colorLabelBoxView;
        _editionView->addSubview(colorLabelBoxView);
    }

    _allVisibleChannelButton =
        std::make_shared<Button>("allVisibleChannelButton", owner, "", _allChannelsVisibilityImage);
    _editionView->addSubview(_allVisibleChannelButton);

    _bottomPanelImage = std::make_shared<Image>("BottomPanel@2x.png");
    _rightPanelImage = std::make_shared<Image>("RightPanel@2x.png");

    _visibleControllerPaneButton =
        std::shared_ptr<Button>(new Button("visibleControllerPaneButton", owner, "", _bottomPanelImage));
    _visibleControllerPaneButton->setType(Button::CustomCheckBoxButtonType);
    _editionView->addSubview(_visibleControllerPaneButton);

    _visiblePropertiesPaneButton =
        std::shared_ptr<Button>(new Button("visiblePropertiesPaneButton", owner, "", _rightPanelImage));
    _visiblePropertiesPaneButton->setType(Button::CustomCheckBoxButtonType);
    _editionView->addSubview(_visiblePropertiesPaneButton);

    _quantizeButton =
        std::shared_ptr<Button>(new Button("quantizeButton", owner, _ui.findString("quantizeStr"), nullptr));
    _quantizeButton->setFont(SystemFonts::sharedInstance()->semiboldFontSmall());
    _editionView->addSubview(_quantizeButton);

    _addFlagButton =
        std::make_shared<Button>("addFlagButton", owner, "", std::make_shared<MDStudio::Image>("AddFlag@2x.png"));
    _addFlagButton->setTooltipText(_ui.findString("addFlagTooltipStr"));
    _editionView->addSubview(_addFlagButton);

    _removeFlagButton =
        std::make_shared<Button>("removeFlagButton", owner, "", std::make_shared<MDStudio::Image>("RemoveFlag@2x.png"));
    _removeFlagButton->setTooltipText(_ui.findString("removeFlagTooltipStr"));
    _editionView->addSubview(_removeFlagButton);

    _removeAllFlagsButton = std::make_shared<Button>("removeAllFlagsButton", owner, "",
                                                     std::make_shared<MDStudio::Image>("RemoveAllFlags@2x.png"));
    _removeAllFlagsButton->setTooltipText(_ui.findString("removeAllFlagsTooltipStr"));
    _editionView->addSubview(_removeAllFlagsButton);

    _goToPreviousFlagButton = std::make_shared<Button>("goToPreviousFlagButton", owner, "",
                                                       std::make_shared<MDStudio::Image>("PreviousFlag@2x.png"));
    _goToPreviousFlagButton->setTooltipText(_ui.findString("goToPreviousFlagTooltipStr"));
    _editionView->addSubview(_goToPreviousFlagButton);

    _goToNextFlagButton =
        std::make_shared<Button>("goToNextFlagButton", owner, "", std::make_shared<MDStudio::Image>("NextFlag@2x.png"));
    _goToNextFlagButton->setTooltipText(_ui.findString("goToNextFlagTooltipStr"));
    _editionView->addSubview(_goToNextFlagButton);

    _editionSixteenthNote = std::make_shared<Image>("EditionSixteenthNote@2x.png");
    _editionEighthNote = std::make_shared<Image>("EditionEighthNote@2x.png");
    _editionQuarterNote = std::make_shared<Image>("EditionQuarterNote@2x.png");
    _editionHalfNote = std::make_shared<Image>("EditionHalfNote@2x.png");
    _editionHalfDotNote = std::make_shared<Image>("EditionHalfDotNote@2x.png");
    _editionWholeNote = std::make_shared<Image>("EditionWholeNote@2x.png");

    std::vector<std::shared_ptr<Image>> editionNoteImages = {_editionSixteenthNote, _editionEighthNote,
                                                             _editionQuarterNote,   _editionHalfNote,
                                                             _editionHalfDotNote,   _editionWholeNote};
    std::vector<Any> editionNoteItems;
    for (auto image : editionNoteImages) editionNoteItems.push_back(image);
    _currentNoteSegmentedControl =
        std::shared_ptr<SegmentedControl>(new SegmentedControl("currentNoteSegmentedControl", owner, editionNoteItems));
    _editionView->addSubview(_currentNoteSegmentedControl);
    _currentNoteSegmentedControl->setIsVisible(false);

    _quantizeNewEventsButton =
        std::shared_ptr<Button>(new Button("quantizeNewEventsButton", owner, _ui.findString("quantizeStr"), nullptr));
    _quantizeNewEventsButton->setFont(SystemFonts::sharedInstance()->semiboldFontSmall());
    _quantizeNewEventsButton->setType(Button::CheckBoxButtonType);
    _editionView->addSubview(_quantizeNewEventsButton);
    _quantizeNewEventsButton->setIsVisible(false);

    _mainView = make_shared<PianoRollMainView>("mainView", this, studio, eventTickWidth, eventHeight);
    _horizSplitView = make_shared<MDStudio::SplitViewH>("horizSplitView", this, _mainView, _pianoRollUtilitiesView,
                                                        PIANO_ROLL_VIEW_UTILITIES_WIDTH, true);
    _horizSplitView->setRightPaneVisibility(false);

    addSubview(_editionView);

    addSubview(_horizSplitView);
}

// ---------------------------------------------------------------------------------------------------------------------
PianoRollView::~PianoRollView() {}

// ---------------------------------------------------------------------------------------------------------------------
void PianoRollView::setFrame(Rect aRect) {
    View::setFrame(aRect);

    Rect r = makeRect(0.0f, 0.0f, bounds().size.width, bounds().size.height - 20.0f);
    _horizSplitView->setFrame(r);

    r = makeRect(0.0f, bounds().size.height - 20.0f, bounds().size.width, 20.0f);
    _editionView->setFrame(r);
    _editionBoxView->setFrame(_editionView->bounds());

    r = makeRect(0.0f, 0.0f, 100.0f, _editionView->bounds().size.height);
    _editionSegmentedControl->setFrame(r);

    r.origin.x += _editionSegmentedControl->frame().size.width + 10.0f;
    r.size.width = 20.0f;
    for (int channel = 0; channel < STUDIO_MAX_CHANNELS; ++channel) {
        _visibleChannelButtons[channel]->setFrame(r);
        _visibleChannelColorLabelBoxViews[channel]->setFrame(
            makeRect(r.origin.x, r.origin.y + r.size.height - 2.0f, r.size.width, 2.0f));
        r.origin.x += r.size.width - 1.0f;
    }
    r.origin.x += 4.0f;
    _allVisibleChannelButton->setFrame(r);
    r.origin.x += r.size.width;

    r.origin.x += 20.0f;
    r.size.width = 60.0f;
    _quantizeButton->setFrame(r);

    r.origin.x += r.size.width + 20.0f;
    r.size.width = 20.0f;
    _goToPreviousFlagButton->setFrame(r);

    r.origin.x += r.size.width - 1.0f;
    r.size.width = 20.0f;
    _goToNextFlagButton->setFrame(r);

    r.origin.x += r.size.width + 5.0f;
    r.size.width = 20.0f;
    _addFlagButton->setFrame(r);

    r.origin.x += r.size.width - 1.0f;
    r.size.width = 20.0f;
    _removeFlagButton->setFrame(r);

    r.origin.x += r.size.width + 5.0f;
    r.size.width = 20.0f;
    _removeAllFlagsButton->setFrame(r);

    r.origin.x += r.size.width + 20.0f;
    r.size.width = 120.0f;
    _currentNoteSegmentedControl->setFrame(r);

    r.origin.x += r.size.width;
    r.size.width = 80.0f;
    _quantizeNewEventsButton->setFrame(r);

    if (_editionView->rect().size.width < 910) {
        _visibleControllerPaneButton->setIsVisible(false);
        _visiblePropertiesPaneButton->setIsVisible(false);
        r = makeRect(_editionView->bounds().size.width - 90.0f, 0.0f, 60.0f, _editionView->bounds().size.height);
        if (_editionView->rect().size.width < 760) r.origin.x += 760 - _editionView->rect().size.width;
    } else {
        _visibleControllerPaneButton->setIsVisible(true);
        _visiblePropertiesPaneButton->setIsVisible(true);
        r = makeRect(_editionView->bounds().size.width - 230.0f, 0.0f, 150.0f, _editionView->bounds().size.height);
    }

    r = makeInsetRect(r, 0.0f, 2.0f);
    _zoomSlider->setFrame(r);

    Rect r2 = r;
    r2.origin.x -= 20.0f;
    r2.size.width = 20.0f;
    _zoomOutImageView->setFrame(r2);
    r2.origin.x = r.origin.x + r.size.width;
    _zoomInImageView->setFrame(r2);

    r2.origin.x += r2.size.width + 20.0f;
    r2.origin.y = 0.0f;
    r2.size.height = 20.0f;
    _visibleControllerPaneButton->setFrame(r2);
    r2.origin.x += 20.0f - 1.0f;
    _visiblePropertiesPaneButton->setFrame(r2);
}
