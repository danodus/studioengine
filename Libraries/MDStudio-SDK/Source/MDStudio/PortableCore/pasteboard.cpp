//
//  pasteboard.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2015-06-30.
//  Copyright (c) 2015-2019 Daniel Cliche. All rights reserved.
//

#include "pasteboard.h"

#include "platform.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
Pasteboard* Pasteboard::sharedInstance() {
    static Pasteboard instance;
    return &instance;
}

// ---------------------------------------------------------------------------------------------------------------------
void Pasteboard::setContent(Any content) {
    // Content of type string is always set to the platform level

    if (content.is<std::string>()) {
        _content = nullptr;
        Platform::sharedInstance()->setPasteboardContent(content);
    } else {
        _content = content;
        Platform::sharedInstance()->setPasteboardContent(std::string());
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Pasteboard::clear() {
    _content = Any();
    Platform::sharedInstance()->setPasteboardContent(std::string());
}

// ---------------------------------------------------------------------------------------------------------------------
bool Pasteboard::isContentAvailable() {
    // Content of type string is always set to the platform level

    auto platformContent = Platform::sharedInstance()->getPasteboardContent();
    return !platformContent.empty() || _content.not_null();
}

// ---------------------------------------------------------------------------------------------------------------------
Any Pasteboard::content() {
    // Content of type string is always set to the platform level

    auto platformContent = Platform::sharedInstance()->getPasteboardContent();
    if (!platformContent.empty()) {
        return platformContent;
    }

    return _content;
}
