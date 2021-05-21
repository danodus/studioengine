//
//  sequencelistitemview.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-09-13.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#include "sequencelistitemview.h"

#include "draw.h"
#include "platform.h"

// ---------------------------------------------------------------------------------------------------------------------
SequenceListItemView::SequenceListItemView(
    std::string name, void* owner, std::vector<float> columnWidths, std::shared_ptr<MDStudio::Image> statusFlagImage,
    std::shared_ptr<MDStudio::Image> statusNewImage, std::shared_ptr<MDStudio::Image> statusTrashImage,
    MelobaseCore::SequencesDB* sequencesDB, int row, MelobaseCore::SequencesDB::sequencesFilterEnum filter,
    std::string nameSearch, std::shared_ptr<MelobaseCore::SequencesFolder> folder, bool isIncludingSubfolders,
    MelobaseCore::SequencesDB::sequencesOrderFieldEnum orderField,
    MelobaseCore::SequencesDB::orderDirectionEnum orderDirection)
    : View(name, owner),
      _columnWidths(columnWidths),
      _statusFlagImage(statusFlagImage),
      _statusNewImage(statusNewImage),
      _statusTrashImage(statusTrashImage),
      _sequencesDB(sequencesDB),
      _row(row),
      _filter(filter),
      _nameSearch(nameSearch),
      _folder(folder),
      _isIncludingSubfolders(isIncludingSubfolders),
      _orderField(orderField),
      _orderDirection(orderDirection) {
    _isHighlighted = false;
    _hasFocus = false;

    _isSubviewsConfigured = false;

    _boxView = std::shared_ptr<MDStudio::BoxView>(new MDStudio::BoxView("boxView", owner));
    _boxView->setBorderColor(MDStudio::blackColor);
    _statusImageView = std::shared_ptr<MDStudio::ImageView>(new MDStudio::ImageView("statusImageView", owner, nullptr));
    _timeLabelView = std::shared_ptr<MDStudio::LabelView>(new MDStudio::LabelView("timeLabelView", owner, ""));
    _ratingLevelIndicator =
        std::shared_ptr<MDStudio::LevelIndicator>(new MDStudio::LevelIndicator("ratingLevelIndicator", owner));
    _nameTextField = std::shared_ptr<MDStudio::TextField>(new MDStudio::TextField("nameTextField", owner));

    addSubview(_boxView);
    addSubview(_statusImageView);
    addSubview(_timeLabelView);
    addSubview(_ratingLevelIndicator);
    addSubview(_nameTextField);
}

// ---------------------------------------------------------------------------------------------------------------------
SequenceListItemView::~SequenceListItemView() {
    removeSubview(_boxView);
    removeSubview(_statusImageView);
    removeSubview(_timeLabelView);
    removeSubview(_ratingLevelIndicator);
    removeSubview(_nameTextField);
}

// ---------------------------------------------------------------------------------------------------------------------
void SequenceListItemView::setFrame(MDStudio::Rect aRect) {
    View::setFrame(aRect);
    _boxView->setFrame(bounds());
    _statusImageView->setFrame(MDStudio::makeRect(0.0f, 0.0f, _columnWidths.at(0), rect().size.height));
    _timeLabelView->setFrame(MDStudio::makeRect(_columnWidths.at(0), 0.0f, _columnWidths.at(1), rect().size.height));
    _ratingLevelIndicator->setFrame(MDStudio::makeRect(_columnWidths.at(0) + _columnWidths.at(1), 0.0f,
                                                       _columnWidths.at(2) - 10.0f, rect().size.height));
    _nameTextField->setFrame(MDStudio::makeRect(_columnWidths.at(0) + _columnWidths.at(1) + _columnWidths.at(2), 0.0f,
                                                _columnWidths.at(3), rect().size.height));
}

// ---------------------------------------------------------------------------------------------------------------------
void SequenceListItemView::setIsHighlighted(bool isHighlighted) {
    _isHighlighted = isHighlighted;
    _boxView->setFillColor(_isHighlighted ? (_hasFocus ? MDStudio::blueColor : MDStudio::veryDimGrayColor)
                                          : MDStudio::blackColor);
}

// ---------------------------------------------------------------------------------------------------------------------
void SequenceListItemView::setFocusState(bool focusState) {
    _hasFocus = focusState;
    _boxView->setFillColor(_isHighlighted ? (_hasFocus ? MDStudio::blueColor : MDStudio::veryDimGrayColor)
                                          : MDStudio::blackColor);
}

// ---------------------------------------------------------------------------------------------------------------------
void SequenceListItemView::configureSubviews() {
    if (_isSubviewsConfigured) return;

    _sequence = _sequencesDB->getSequence(_row, _filter, _nameSearch, _folder, _isIncludingSubfolders, _orderField,
                                          _orderDirection);

    //
    // Configure the status
    //

    std::shared_ptr<MDStudio::Image> statusImage = nullptr;
    if (_sequence->rating < 0) {
        statusImage = _statusTrashImage;
    } else if (_sequence->annotations.size() > 0) {
        statusImage = _statusFlagImage;
    } else if (_sequence->playCount == 0) {
        statusImage = _statusNewImage;
    }
    _statusImageView->setImage(statusImage);

    //
    // Configure the date and time
    //

    _timeLabelView->setTitle(MDStudio::Platform::sharedInstance()->timestampToStr(_sequence->date));
    _nameTextField->setText(_sequence->name, false);
    _ratingLevelIndicator->setLevel(_sequence->rating, false);

    _isSubviewsConfigured = true;
}

// ---------------------------------------------------------------------------------------------------------------------
void SequenceListItemView::startNameEdition() { _nameTextField->startEdition(); }

// ---------------------------------------------------------------------------------------------------------------------
bool SequenceListItemView::isSendingDrop() { return _isHighlighted && _hasFocus; }
