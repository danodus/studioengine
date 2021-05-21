//
//  sequencelistitemview.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-09-13.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#ifndef SEQUENCELISTITEMVIEW_H
#define SEQUENCELISTITEMVIEW_H

#include <boxview.h>
#include <imageview.h>
#include <labelview.h>
#include <levelindicator.h>
#include <sequencesdb.h>
#include <textfield.h>
#include <view.h>

#include <memory>
#include <string>

class SequenceListItemView : public MDStudio::View {
    bool _isHighlighted;
    bool _hasFocus;

    bool _isSubviewsConfigured;

    MelobaseCore::SequencesDB* _sequencesDB;
    int _row;

    MelobaseCore::SequencesDB::sequencesFilterEnum _filter;
    MelobaseCore::SequencesDB::sequencesOrderFieldEnum _orderField;
    MelobaseCore::SequencesDB::orderDirectionEnum _orderDirection;

    std::string _nameSearch;
    std::shared_ptr<MelobaseCore::SequencesFolder> _folder;
    bool _isIncludingSubfolders;

    std::shared_ptr<MDStudio::BoxView> _boxView;
    std::shared_ptr<MDStudio::ImageView> _statusImageView;
    std::shared_ptr<MDStudio::LabelView> _timeLabelView;
    std::shared_ptr<MDStudio::TextField> _nameTextField;
    std::shared_ptr<MDStudio::LevelIndicator> _ratingLevelIndicator;

    std::shared_ptr<MDStudio::Image> _statusFlagImage, _statusNewImage, _statusTrashImage;

    std::shared_ptr<MelobaseCore::Sequence> _sequence;

    std::vector<float> _columnWidths;

    void configureSubviews() override;

    bool isSendingDrop() override;

   public:
    SequenceListItemView(std::string name, void* owner, std::vector<float> columnWidths,
                         std::shared_ptr<MDStudio::Image> statusFlagImage,
                         std::shared_ptr<MDStudio::Image> statusNewImage,
                         std::shared_ptr<MDStudio::Image> statusTrashImage, MelobaseCore::SequencesDB* sequencesDB,
                         int row, MelobaseCore::SequencesDB::sequencesFilterEnum filter, std::string nameSearch,
                         std::shared_ptr<MelobaseCore::SequencesFolder> folder, bool isIncludingSubfolders,
                         MelobaseCore::SequencesDB::sequencesOrderFieldEnum orderField,
                         MelobaseCore::SequencesDB::orderDirectionEnum orderDirection);
    ~SequenceListItemView();

    void setIsHighlighted(bool isHighlighted);
    void setFocusState(bool focusState);
    void setColumnWidths(std::vector<float> columnWidths) {
        _columnWidths = columnWidths;
        setFrame(frame());
    }

    std::shared_ptr<MDStudio::TextField> nameTextField() { return _nameTextField; }
    std::shared_ptr<MDStudio::LevelIndicator> ratingLevelIndicator() { return _ratingLevelIndicator; }

    void startNameEdition();

    std::shared_ptr<MelobaseCore::Sequence> sequence() {
        configureSubviews();
        return _sequence;
    }

    void setFrame(MDStudio::Rect rect) override;
};

#endif  // SEQUENCELISTITEMVIEW_H
