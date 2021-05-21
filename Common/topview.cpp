//
//  topview.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2014-07-12.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#include "topview.h"

#include <platform.h>

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
TopView::TopView(std::string name, void* owner, Studio* studio, double eventTickWidth, float eventHeight)
    : View(name, owner) {
    _handleEventFn = nullptr;

    _ui.loadUI(this, MDStudio::Platform::sharedInstance()->resourcesPath() + "/TopView.lua");

    // Load images
    _playingImage = std::make_shared<Image>("Playing@2x.png");
    _recordingImage = std::make_shared<Image>("Recording@2x.png");
    _metronomeLearningImage = std::make_shared<Image>("MetronomeLearning@2x.png");
    _learnTempoImage = std::make_shared<Image>("LearnTempo@2x.png");

    // Add controls view
    _controlsView = std::shared_ptr<View>(new View("controlsView", owner));
    _controlsViewUI = new UI();
    _controlsViewUI->loadUI(_controlsView.get(),
                            MDStudio::Platform::sharedInstance()->resourcesPath() + "/ControlsView.lua");
    addSubview(_controlsView);

    // Add studio view
    _studioView = std::shared_ptr<StudioView>(new StudioView("studioView", owner));
    addSubview(_studioView);

    // Create sequence view
    _sequenceView =
        std::shared_ptr<SequenceView>(new SequenceView("sequenceView", owner, studio, eventTickWidth, eventHeight));

    // Create db view
    _dbView = std::shared_ptr<DBView>(new DBView("dbView", owner));

    // Add split view
    _horizSplitView = std::shared_ptr<SplitViewH>(new SplitViewH("horizSplitView", owner, _dbView, _sequenceView, 420));
    addSubview(_horizSplitView);

    _splitSheet = std::shared_ptr<Sheet>(new Sheet("splitSheet", this));

    _splitLabelView0 =
        std::shared_ptr<LabelView>(new LabelView("splitLabelView0", this, _ui.findString("splitMessage0")));
    _splitLabelView1 =
        std::shared_ptr<LabelView>(new LabelView("splitLabelView1", this, _ui.findString("splitMessage1")));
    _splitSheet->addSubview(_splitLabelView0);
    _splitSheet->addSubview(_splitLabelView1);

    _cancelButton = std::shared_ptr<Button>(new Button("cancelButton", this, _ui.findString("cancel")));
    _cancelButton->setType(Button::CancelButtonType);
    _splitSheet->addSubview(_cancelButton);

    // Loading view
    _loadingBoxView = std::make_shared<MDStudio::BoxView>("loadingBoxView", this);
    _loadingBoxView->setBorderColor(MDStudio::zeroColor);
    addSubview(_loadingBoxView);

    _learnTempoSheet = std::shared_ptr<Sheet>(new Sheet("learnTempoSheet", this, false));
    _learnTempoImageView = std::shared_ptr<ImageView>(new ImageView("learnTempoImageView", this, _learnTempoImage));
    _learnTempoLabelView = std::shared_ptr<LabelView>(new LabelView("learnTempoLabelView", this, ""));
    _learnTempoLabelView->setTextAlign(LabelView::CenterTextAlign);

    // About view
    _aboutView = std::shared_ptr<MDStudio::View>(new MDStudio::View("aboutView", this));

    // Acknowledgements view
    _acknowledgementsView = std::shared_ptr<MDStudio::View>(new MDStudio::View("acknowledgementsView", this));

    // Audio export view
    _audioExportView = std::shared_ptr<MDStudio::View>(new MDStudio::View("audioExportView", this));

    // MIDI import view
    _midiImportView = std::shared_ptr<MDStudio::View>(new MDStudio::View("midiImportView", this));

    // Preferences view
    _preferencesView = std::shared_ptr<MDStudio::View>(new MDStudio::View("preferencesView", this));

    // Confirm cleanup view
    _confirmCleanupView = std::shared_ptr<MDStudio::View>(new MDStudio::View("confirmCleanupView", this));

    // Confirm empty trash view
    _confirmEmptyTrashView = std::shared_ptr<MDStudio::View>(new MDStudio::View("confirmEmptyTrashView", this));

    // Confirm set as played all view
    _confirmSetAsPlayedAllView = std::shared_ptr<MDStudio::View>(new MDStudio::View("confirmSetAsPlayedAllView", this));

    // New sequence view
    _newSequenceView = std::shared_ptr<MDStudio::View>(new MDStudio::View("newSequenceView", this));

    // Localized strings
    _untitledStr = _ui.findString("untitled");
    _learnTempoStr[0] = _ui.findString("learnTempoText1");
    _learnTempoStr[1] = _ui.findString("learnTempoText2");
    _learnTempoStr[2] = _ui.findString("learnTempoText3");
}

// ---------------------------------------------------------------------------------------------------------------------
TopView::~TopView() {
    delete _controlsViewUI;
    removeAllSubviews();
}

// ---------------------------------------------------------------------------------------------------------------------
bool TopView::handleEvent(const UIEvent* event) {
    if (_handleEventFn) {
        if (_handleEventFn(this, event)) return true;
    }
    return View::handleEvent(event);
}

// ---------------------------------------------------------------------------------------------------------------------
void TopView::setFrame(Rect aRect) {
    View::setFrame(aRect);

    _loadingBoxView->setFrame(bounds());

    const float studioViewHeight = _studioView->isVisible() ? 150.0f : 0;

    Rect r = makeRect(0.0f, 0.0f, frame().size.width, studioViewHeight);
    _studioView->setFrame(r);

    r = makeRect(0.0f, studioViewHeight, frame().size.width, frame().size.height - studioViewHeight - 80);
    _horizSplitView->setFrame(r);

    r = makeRect(0.0f, frame().size.height - 80, frame().size.width, 80);
    _controlsView->setFrame(r);

    r = makeCenteredRectInRect(topView()->frame(), 400.0f, 108.0f);
    _splitSheet->setFrame(r);

    _splitLabelView0->setFrame(makeRect(20.0f, 68.0f, 360.0f, 20.0f));
    _splitLabelView1->setFrame(makeRect(20.0f, 48.0f, 360.0f, 20.0f));
    _cancelButton->setFrame(makeRect(200.0f - 50.0f, 20.0f, 100.0f, 20.0f));

    r = makeCenteredRectInRect(topView()->frame(), 700.0f, 200.0f);
    _learnTempoSheet->setFrame(r);
    _learnTempoImageView->setFrame(makeRect(10.0f, 40.0f, r.size.width - 20.0f, r.size.height - 40.0f));
    _learnTempoLabelView->setFrame(makeRect(10.0f, 20.0f, r.size.width - 20.0f, 20.0f));
}
