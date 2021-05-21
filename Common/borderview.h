//
//  borderview.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2018-07-18.
//  Copyright © 2018 Daniel Cliche. All rights reserved.
//

#ifndef BORDERVIEW_H
#define BORDERVIEW_H

#include <view.h>

class BorderView : public MDStudio::View
{
    std::shared_ptr<MDStudio::View> _contentView;
    float _leftBorder, _rightBorder, _bottomBorder, _topBorder;
    
public:
    BorderView(const std::string &name, void *owner, std::shared_ptr<MDStudio::View> contentView, float leftBorder, float rightBorder, float bottomBorder, float topBorder);
    ~BorderView();
    
    std::shared_ptr<MDStudio::View> contentView() { return _contentView; }
    
    void setFrame(MDStudio::Rect frame) override;
};

#endif // BORDERVIEW_H
