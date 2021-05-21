//
//  borderview.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2018-07-18.
//  Copyright © 2018 Daniel Cliche. All rights reserved.
//

#include "borderview.h"

// ---------------------------------------------------------------------------------------------------------------------
BorderView::BorderView(const std::string &name, void *owner, std::shared_ptr<MDStudio::View> contentView, float leftBorder, float rightBorder, float bottomBorder, float topBorder) : MDStudio::View(name, owner), _contentView(contentView), _leftBorder(leftBorder), _rightBorder(rightBorder), _bottomBorder(bottomBorder), _topBorder(topBorder)
{
    addSubview(contentView);
}

// ---------------------------------------------------------------------------------------------------------------------
BorderView::~BorderView()
{
    removeSubview(_contentView);
}

// ---------------------------------------------------------------------------------------------------------------------
void BorderView::setFrame(MDStudio::Rect frame)
{
    View::setFrame(frame);
    
    auto r = bounds();
    _contentView->setFrame(MDStudio::makeRect(_leftBorder, _bottomBorder, r.size.width - _leftBorder - _rightBorder, r.size.height - _bottomBorder - _topBorder));
}
