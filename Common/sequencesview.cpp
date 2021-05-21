//
//  sequencesview.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-09-15.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#include "sequencesview.h"

#include <algorithm>

#include "platform.h"
#include "sequencelistitemview.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
SequencesView::SequencesView(std::string name, void* owner) : View(name, owner) {
    _ui.loadUI(this, MDStudio::Platform::sharedInstance()->resourcesPath() + "/SequencesView.lua");

    _showHideFoldersImage = std::make_shared<Image>("Folders@2x.png");

    _showHideFoldersButton =
        std::make_shared<MDStudio::Button>("showHideFoldersButton", owner, "", _showHideFoldersImage);
    _showHideFoldersButton->setType(MDStudio::Button::CustomCheckBoxButtonType);

    _filterImages.push_back(std::make_shared<Image>("FilterAny@2x.png"));
    _filterImages.push_back(std::make_shared<Image>("FilterNew@2x.png"));
    _filterImages.push_back(std::make_shared<Image>("FilterFlag@2x.png"));

    // Load images
    for (int i = 0; i < 5; ++i) {
        auto image = std::make_shared<Image>(std::string("Filter") + std::to_string(i + 1) + std::string("@2x.png"));
        _filterImages.push_back(image);
    }

    std::vector<Any> items;
    for (auto image : _filterImages) items.push_back(image);

    _filterSegmentedControl =
        std::shared_ptr<SegmentedControl>(new SegmentedControl("filterSegmentedControl", owner, items));
    _filterSegmentedControl->setSelectedSegment(0, false);
    _controlsView = std::shared_ptr<View>(new View("controlsView", owner));
    _controlsView->addSubview(_showHideFoldersButton);
    _controlsView->addSubview(_filterSegmentedControl);

    _nameSearchField = std::shared_ptr<SearchField>(new SearchField("nameSearchField", owner));
    _controlsView->addSubview(_nameSearchField);

    // Create table view
    _tableView = std::shared_ptr<TableView>(new TableView("tableView", owner, 18.0f, true, false));
    // Set it pass though in order to allow the subviews of the selected row to receive events
    _tableView->setIsPassThrough(true);

    _noSequencesImage = std::make_shared<Image>(
        MDStudio::Platform::sharedInstance()->language() == "fr" ? "NoSequencesFr@2x.png" : "NoSequencesEn@2x.png");
    _noSequencesImageView = std::shared_ptr<ImageView>(new ImageView("noSequencesImageView", owner, _noSequencesImage));
    _noSequencesImageView->setIsVisible(false);

    addSubview(_tableView);
    addSubview(_controlsView);
    addSubview(_noSequencesImageView);
}

// ---------------------------------------------------------------------------------------------------------------------
SequencesView::~SequencesView() {
    _controlsView->removeSubview(_showHideFoldersButton);
    _controlsView->removeSubview(_filterSegmentedControl);
    _controlsView->removeSubview(_nameSearchField);
    removeSubview(_tableView);
    removeSubview(_controlsView);
}

// ---------------------------------------------------------------------------------------------------------------------
void SequencesView::setFrame(Rect aRect) {
    View::setFrame(aRect);

    _controlsView->setFrame(makeRect(0.0f, rect().size.height - 20.0f, rect().size.width, 20.0f));
    _showHideFoldersButton->setFrame(makeRect(0.0f, 0.0f, 20.0f, 20.0f));
    _filterSegmentedControl->setFrame(makeRect(30.0f, 0.0f, 160.0f, 20.0f));
    Rect r = _controlsView->bounds();
    _nameSearchField->setFrame(makeRect(r.size.width - 150.0f, 0.0f, 150.0f, r.size.height));

    _tableView->setFrame(makeRect(0.0f, 0.0f, rect().size.width, rect().size.height - 20.0f));

    _noSequencesImageView->setFrame(_tableView->frame());
}
