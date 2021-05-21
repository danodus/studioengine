//
//  test_undomanager.cpp
//  MelobaseStationTests
//
//  Created by Daniel Cliche on 2015-04-13.
//  Copyright (c) 2015 Daniel Cliche. All rights reserved.
//

#include "test_undomanager.h"

#include <undomanager.h>
#include <iostream>

using namespace MDStudio;

int lastValue = 0;
UndoManager undoManager;

void setValue(int value)
{
    int lv = lastValue;
    undoManager.pushFn([=]{setValue(lv);});
    lastValue = value;
}

// ---------------------------------------------------------------------------------------------------------------------
bool testUndoManager()
{
    undoManager.beginGroup();
    setValue(10);
    setValue(20);
    undoManager.endGroup();
    setValue(30);
    if (lastValue != 30)
        return false;
    undoManager.undo();
    if (lastValue != 20)
        return false;
    undoManager.undo();
    if (lastValue != 0)
        return false;
    if (undoManager.canUndo())
        return false;
    
    if (!undoManager.canRedo())
        return false;
    
    undoManager.redo();
    
    if (lastValue != 20)
        return false;
    
    if (!undoManager.canRedo())
        return false;
    
    undoManager.redo();

    if (lastValue != 30)
        return false;
    
    if (undoManager.canRedo())
        return false;
    
    return true;
}