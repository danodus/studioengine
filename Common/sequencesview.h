//
//  sequencesview.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-09-15.
//  Copyright (c) 2014-2015 Daniel Cliche. All rights reserved.
//

#ifndef SEQUENCESVIEW_H
#define SEQUENCESVIEW_H

#include <view.h>
#include <segmentedcontrol.h>
#include <image.h>
#include <imageview.h>
#include <tableview.h>
#include <scrollview.h>
#include <labelview.h>
#include <searchfield.h>
#include <ui.h>

#include <memory>

class SequencesView : public MDStudio::View {
    
    MDStudio::UI _ui;
    
    std::vector<std::shared_ptr<MDStudio::Image>> _filterImages;
    
    std::shared_ptr<MDStudio::View> _controlsView;
    std::shared_ptr<MDStudio::Button> _showHideFoldersButton;
    std::shared_ptr<MDStudio::SegmentedControl> _filterSegmentedControl;
    std::shared_ptr<MDStudio::SearchField> _nameSearchField;
    std::shared_ptr<MDStudio::TableView> _tableView;
    
    std::shared_ptr<MDStudio::Image> _showHideFoldersImage;
    std::shared_ptr<MDStudio::Image> _noSequencesImage;
    std::shared_ptr<MDStudio::ImageView> _noSequencesImageView;
    

public:
    SequencesView(std::string name, void *owner);
    ~SequencesView();
    
    std::shared_ptr<MDStudio::TableView> tableView() { return _tableView; }
    std::shared_ptr<MDStudio::Button> showHideFoldersButton() { return _showHideFoldersButton; }
    std::shared_ptr<MDStudio::SegmentedControl> filterSegmentedControl() { return _filterSegmentedControl; }
    std::shared_ptr<MDStudio::SearchField> nameSearchField() { return _nameSearchField; }
    std::shared_ptr<MDStudio::ImageView> noSequencesImageView() { return _noSequencesImageView; }
    
    MDStudio::UI &ui() { return _ui; }
    
    void setFrame(MDStudio::Rect rect) override;

};




#endif // SEQUENCESVIEW_H
