//
//  trackheaderview.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-05-19.
//  Copyright © 2016-2019 Daniel Cliche. All rights reserved.
//

#ifndef TRACKHEADERVIEW_H
#define TRACKHEADERVIEW_H

#include <view.h>
#include <studio.h>
#include <boxview.h>
#include <labelview.h>
#include <textfield.h>
#include <button.h>
#include <combobox.h>
#include <melobasecore_sequence.h>
#include <ui.h>

class TrackHeaderView : public MDStudio::View {
    
    MDStudio::UI _ui;
    
    std::shared_ptr<MDStudio::BoxView> _boxView;
    std::shared_ptr<MDStudio::BoxView> _channelBoxView;
    std::shared_ptr<MDStudio::LabelView> _nameLabelView;
    std::shared_ptr<MDStudio::TextField> _nameTextField;
    std::shared_ptr<MDStudio::Button> _armedButton;
    std::shared_ptr<MDStudio::Button> _muteButton;
    std::shared_ptr<MDStudio::Button> _soloButton;
    std::shared_ptr<MDStudio::LabelView> _channelLabelView;
    std::shared_ptr<MDStudio::ComboBox> _channelComboBox;
    
    MelobaseCore::Sequence *_sequence;
    
    int _trackIndex;
    
    bool _isHighlighted;
    bool _hasFocus;
    
public:
    
    TrackHeaderView(std::string name, void *owner, MelobaseCore::Sequence *sequence, int trackIndex);
    ~TrackHeaderView();
    
    void setFrame(MDStudio::Rect rect) override;
    
    std::shared_ptr<MDStudio::BoxView> channelBoxView() { return _channelBoxView; }
    std::shared_ptr<MDStudio::TextField> nameTextField() { return _nameTextField; }
    std::shared_ptr<MDStudio::Button> armedButton() { return _armedButton; }
    std::shared_ptr<MDStudio::Button> muteButton() { return _muteButton; }
    std::shared_ptr<MDStudio::Button> soloButton() { return _soloButton; }
    std::shared_ptr<MDStudio::ComboBox> channelComboBox() { return _channelComboBox; }
    
    int trackIndex() { return _trackIndex; }
        
    void setAreControlsVisible(bool areVisible);
    
    void startNameEdition();
};


#endif // TRACKHEADERVIEW_H
