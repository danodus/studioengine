//
//  newsequenceviewcontroller.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-03-10.
//  Copyright © 2016-2019 Daniel Cliche. All rights reserved.
//

#include "newsequenceviewcontroller.h"

#include <sstream>

// ---------------------------------------------------------------------------------------------------------------------
NewSequenceViewController::NewSequenceViewController(MDStudio::View *topView, std::shared_ptr<MDStudio::View> view, std::string uiPath) : ModalViewController(topView, view, uiPath)
{
    using namespace std::placeholders;
    
    _nameTextField = std::dynamic_pointer_cast<MDStudio::TextField>(_ui->findView("nameTextField"));
    _nameTextField->setTextDidChangeFn(std::bind(&NewSequenceViewController::nameTextFieldTextDidChange, this, _1, _2));
    
    _timeSigNumSegmentedControl = std::dynamic_pointer_cast<MDStudio::SegmentedControl>(_ui->findView("timeSigNumSegmentedControl"));
    _timeSigNumSegmentedControl->setDidSelectSegmentFn(std::bind(&NewSequenceViewController::timeSignatureNumSegmentedControlDidSelectSegment, this, _1, _2));
    
    _timeSigDenumSegmentedControl = std::dynamic_pointer_cast<MDStudio::SegmentedControl>(_ui->findView("timeSigDenumSegmentedControl"));
    _timeSigDenumSegmentedControl->setDidSelectSegmentFn(std::bind(&NewSequenceViewController::timeSignatureDenumSegmentedControlDidSelectSegment, this, _1, _2));
    
    _tempoSlider = std::dynamic_pointer_cast<MDStudio::Slider>(_ui->findView("tempoSlider"));
    _tempoSlider->setPosChangedFn(std::bind(&NewSequenceViewController::tempoSliderPosChanged, this, _1, _2));
    _tempoSlider->setPosSetFn(std::bind(&NewSequenceViewController::tempoSliderPosSet, this, _1, _2));
    
    _tempoValueLabelView = std::dynamic_pointer_cast<MDStudio::LabelView>(_ui->findView("tempoValueLabelView"));
    
    setName("");
    setTimeSignatureNum(4);
    setTimeSignatureDenum(4);
    setBPM(120);
}

// ---------------------------------------------------------------------------------------------------------------------
void NewSequenceViewController::setName(std::string name)
{
    _nameTextField->setText(name);
}

// ---------------------------------------------------------------------------------------------------------------------
void NewSequenceViewController::setTimeSignatureNum(unsigned int timeSignatureNum)
{
    int segment = -1;
    switch (timeSignatureNum) {
        case 2:
            segment = 0;
            break;
        case 3:
            segment = 1;
            break;
        case 4:
            segment = 2;
            break;
        case 5:
            segment = 3;
            break;
        case 7:
            segment = 4;
            break;
        case 11:
            segment = 5;
            break;
        default:
            segment = -1;
    }
    
    _timeSigNumSegmentedControl->setSelectedSegment(segment);
}

// ---------------------------------------------------------------------------------------------------------------------
void NewSequenceViewController::setTimeSignatureDenum(unsigned int timeSignatureDenum)
{
    int segment = -1;
    
    switch (timeSignatureDenum) {
        case 1:
            segment = 0;
            break;
        case 2:
            segment = 1;
            break;
        case 4:
            segment = 2;
            break;
        case 8:
            segment = 3;
            break;
        case 16:
            segment = 4;
            break;
        case 32:
            segment = 5;
            break;
        case 64:
            segment = 6;
            break;
        default:
            segment = -1;
    }
    
    _timeSigDenumSegmentedControl->setSelectedSegment(segment);
}

// ---------------------------------------------------------------------------------------------------------------------
void NewSequenceViewController::setBPM(unsigned int bpm)
{
    _tempoSlider->setPos(static_cast<float>(bpm));
}

// ---------------------------------------------------------------------------------------------------------------------
void NewSequenceViewController::nameTextFieldTextDidChange(MDStudio::TextField *sender, std::string text)
{
    _name = text;
}

// ---------------------------------------------------------------------------------------------------------------------
void NewSequenceViewController::timeSignatureNumSegmentedControlDidSelectSegment(MDStudio::SegmentedControl *sender, int segment)
{
    int numerators[] = {2, 3, 4, 5, 7, 11};
    _timeSignatureNum = numerators[segment];
}

// ---------------------------------------------------------------------------------------------------------------------
void NewSequenceViewController::timeSignatureDenumSegmentedControlDidSelectSegment(MDStudio::SegmentedControl *sender, int segment)
{
    int denumerators[] = {1, 2, 4, 8, 16, 32, 64};
    _timeSignatureDenum = denumerators[segment];
}

// ---------------------------------------------------------------------------------------------------------------------
void NewSequenceViewController::tempoSliderPosChanged(MDStudio::Slider *sender, float value)
{
    _bpm = static_cast<unsigned int>(value + 5.0f) / 10 * 10;
    
    std::stringstream ss;
    ss << _bpm << " BPM";
    _tempoValueLabelView->setTitle(ss.str());
}

// ---------------------------------------------------------------------------------------------------------------------
void NewSequenceViewController::tempoSliderPosSet(MDStudio::Slider *sender, float value)
{
    sender->setPos(static_cast<float>(_bpm), false);
}

// ---------------------------------------------------------------------------------------------------------------------
void NewSequenceViewController::showModal()
{
    ModalViewController::showModal();
    _nameTextField->startEdition();
}
