//
//  webview.h
//  MDStudio
//
//  Created by Daniel Cliche on 2016-09-30.
//  Copyright (c) 2016-2020 Daniel Cliche. All rights reserved.
//

#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <view.h>

namespace MDStudio {

class WebView : public MDStudio::View {
    const std::string _s;

    bool drawHTML();

    bool _isDrawing;
    float _width;

    bool parse();

   public:
    typedef enum {
        RootState,
        HtmlState,
        HeadState,
        TitleState,
        BodyState,
        Header1State,
        Header2State,
        ParaState,
        ImageState
    } States;

    MDStudio::Point _pt;
    States _parserState;

    void printWord(const std::string& word);
    void printVerticalSpace(float factor);
    void drawImage(const std::string& src);

    WebView(const std::string& name, void* owner, const std::string& s);

    float contentHeight(float width);

    void draw() override;
};

}  // namespace MDStudio

#endif  // WEBVIEW_H
