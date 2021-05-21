//
//  rulerview.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2015-12-15.
//  Copyright © 2015-2019 Daniel Cliche. All rights reserved.
//

#ifndef RULERVIEW_H
#define RULERVIEW_H

#include <view.h>

class RulerView : public MDStudio::View {
    
    float _margin;
    int _min;
    unsigned int _nbDivisions;
    unsigned int _divStep;
    bool _hasFocus;
    
public:
    
    RulerView(std::string name, void *owner, float margin, int min = 0, unsigned int nbDivisions = 0, unsigned int divStep = 0);
    
    void setRange(unsigned int min, unsigned int nbDivisions, unsigned int divStep);
    
    void draw() override;
    
    void setFocusState(bool state) { _hasFocus = state; setDirty(); }
    
};

#endif // RULERVIEW_H

