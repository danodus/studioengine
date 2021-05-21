//
//  test_pasteboard.cpp
//  MelobaseStationTests
//
//  Created by Daniel Cliche on 2015-06-30.
//  Copyright (c) 2015 Daniel Cliche. All rights reserved.
//

#include "test_pasteboard.h"

#include <pasteboard.h>

// ---------------------------------------------------------------------------------------------------------------------
bool testPasteboard()
{
    
    MDStudio::Pasteboard *pasteboard = MDStudio::Pasteboard::sharedInstance();
    
    if (pasteboard->isContentAvailable())
        return false;
    
    std::string myContent("Hello World");
    
    pasteboard->setContent(myContent);
    
    if (!pasteboard->isContentAvailable())
        return false;
    
    if (!pasteboard->content().is<std::string>())
        return false;
    
    if (pasteboard->content().as<std::string>() != "Hello World")
        return false;
    
    pasteboard->clear();
    
    if (pasteboard->isContentAvailable())
        return false;
    
    return true;
}