//
//  newsequenceviewcontroller.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-03-10.
//  Copyright © 2016 Daniel Cliche. All rights reserved.
//

#ifndef NEWSEQUENCEVIEWCONTROLLER_H
#define NEWSEQUENCEVIEWCONTROLLER_H

#include "modalviewcontroller.h"

#include <textfield.h>
#include <slider.h>
#include <segmentedcontrol.h>
#include <labelview.h>

#include <memory>

class NewSequenceViewController : public ModalViewController {
    
    std::shared_ptr<MDStudio::TextField> _nameTextField;
    std::shared_ptr<MDStudio::SegmentedControl> _timeSigNumSegmentedControl;
    std::shared_ptr<MDStudio::SegmentedControl> _timeSigDenumSegmentedControl;
    std::shared_ptr<MDStudio::Slider> _tempoSlider;
    std::shared_ptr<MDStudio::LabelView> _tempoValueLabelView;

    std::string _name;
    unsigned int _timeSignatureNum, _timeSignatureDenum;
    unsigned int _bpm;
    
    void nameTextFieldTextDidChange(MDStudio::TextField *sender, std::string text);
    void timeSignatureNumSegmentedControlDidSelectSegment(MDStudio::SegmentedControl *sender, int segment);
    void timeSignatureDenumSegmentedControlDidSelectSegment(MDStudio::SegmentedControl *sender, int segment);
    void tempoSliderPosChanged(MDStudio::Slider *sender, float value);
    void tempoSliderPosSet(MDStudio::Slider *sender, float value);
    
public:
    NewSequenceViewController(MDStudio::View *topView, std::shared_ptr<MDStudio::View> view, std::string uiPath);
    
    void setName(std::string name);
    std::string name() { return _name; }

    void setTimeSignatureNum(unsigned int timeSignatureNum);
    unsigned int timeSignatureNum() { return _timeSignatureNum; }
    
    void setTimeSignatureDenum(unsigned int timeSignatureDenum);
    int timeSignatureDenum() { return _timeSignatureDenum; }

    void setBPM(unsigned int bpm);
    unsigned int bpm() { return _bpm; }

    void showModal() override;
};

#endif // NEWSEQUENCEVIEWCONTROLLER_H
