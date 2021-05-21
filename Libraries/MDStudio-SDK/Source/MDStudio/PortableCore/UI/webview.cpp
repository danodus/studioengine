//
//  webview.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2016-09-30.
//  Copyright © 2016-2020 Daniel Cliche. All rights reserved.
//

#include "webview.h"

#include <expat.h>

#include "draw.h"

#define BUFFSIZE 8192

using namespace MDStudio;

char Buff[BUFFSIZE];

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
WebView::WebView(const std::string& name, void* owner, const std::string& s) : View(name, owner), _s(s) {
    _pt = makeZeroPoint();
    _isDrawing = false;
}

// ---------------------------------------------------------------------------------------------------------------------
static void XMLCALL start(void* data, const char* el, const char** attr) {
    WebView* webView = (WebView*)data;

    auto element = std::string(el);

    switch (webView->_parserState) {
        case WebView::RootState:
            if (element == "html") webView->_parserState = WebView::HtmlState;
            break;
        case WebView::HtmlState:
            if (element == "head") {
                webView->_parserState = WebView::HeadState;
            } else if (element == "body") {
                webView->_parserState = WebView::BodyState;
            }
            break;
        case WebView::HeadState:
            if (element == "title") {
                webView->_parserState = WebView::TitleState;
            }
            break;
        case WebView::BodyState:
            if (element == "h1") {
                webView->_parserState = WebView::Header1State;
            } else if (element == "h2") {
                webView->_parserState = WebView::Header2State;
            } else if (element == "p") {
                webView->_parserState = WebView::ParaState;
            }
            break;
        case WebView::ParaState:
            if (element == "img") {
                webView->_parserState = WebView::ImageState;
                std::string src;
                for (int i = 0; attr[i]; i += 2) {
                    auto parameter = std::string(attr[i]);
                    if (parameter == "src") {
                        src = std::string(attr[i + 1]);
                    }
                }
                if (!src.empty()) webView->drawImage(src);
            }
            break;
        default:
            webView->_parserState = WebView::RootState;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
static void XMLCALL chars(void* data, const char* el, int len) {
    WebView* webView = (WebView*)data;

    auto s = std::string(el, len);

    switch (webView->_parserState) {
        case WebView::Header1State:
        case WebView::Header2State:
        case WebView::ParaState: {
            std::string word;
            for (auto c : s) {
                if (c == ' ') {
                    webView->printWord(word);
                    word = "";
                } else {
                    word += c;
                }
            }
            if (!word.empty()) {
                webView->printWord(word);
            }
        } break;
        default:
            break;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
static void XMLCALL end(void* data, const char* el) {
    WebView* webView = (WebView*)data;

    auto element = std::string(el);

    switch (webView->_parserState) {
        case WebView::HtmlState:
            if (element == "html") webView->_parserState = WebView::RootState;
            break;
        case WebView::HeadState:
            if (element == "head") {
                webView->_parserState = WebView::HtmlState;
            }
            break;
        case WebView::TitleState:
            if (element == "title") {
                webView->_parserState = WebView::HeadState;
            }
            break;
        case WebView::BodyState:
            if (element == "body") {
                webView->_parserState = WebView::HtmlState;
            }
            break;
        case WebView::Header1State:
            if (element == "h1") {
                webView->_parserState = WebView::BodyState;
                webView->printVerticalSpace(2.0f);
            }
            break;
        case WebView::Header2State:
            if (element == "h2") {
                webView->_parserState = WebView::BodyState;
                webView->printVerticalSpace(2.0f);
            }
            break;
        case WebView::ParaState:
            if (element == "p") {
                webView->_parserState = WebView::BodyState;
                webView->printVerticalSpace(1.5f);
            }
            break;
        case WebView::ImageState:
            if (element == "img") {
                webView->_parserState = WebView::ParaState;
            }
            break;
        default:
            webView->_parserState = WebView::RootState;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void WebView::printWord(const std::string& word) {
    DrawContext* dc = drawContext();

    if (_pt.x + getTextWidth(SystemFonts::sharedInstance()->semiboldFont(), word) > _width) {
        _pt.x = 0.0f;
        _pt.y -= fontHeight(SystemFonts::sharedInstance()->semiboldFont());
    }
    if (_isDrawing) {
        dc->pushStates();
        dc->setStrokeColor(blackColor);
        dc->drawText(SystemFonts::sharedInstance()->semiboldFont(), _pt, word + " ");
        dc->popStates();
    }
    _pt.x += getTextWidth(SystemFonts::sharedInstance()->semiboldFont(), word + " ");
}

// ---------------------------------------------------------------------------------------------------------------------
void WebView::printVerticalSpace(float factor) {
    _pt.x = 0;
    _pt.y -= factor * fontHeight(SystemFonts::sharedInstance()->semiboldFont());
}

// ---------------------------------------------------------------------------------------------------------------------
void WebView::drawImage(const std::string& src) {
    DrawContext* dc = drawContext();

    auto image = std::make_shared<Image>(src, true);
    _pt.y -= image->size().height;
    if (_isDrawing) dc->drawImage(_pt, image);
    _pt.x += image->size().width;
}

// ---------------------------------------------------------------------------------------------------------------------
bool WebView::parse() {
    _parserState = RootState;

    XML_Parser p = XML_ParserCreate(NULL);
    if (!p) {
        fprintf(stderr, "Couldn't allocate memory for parser\n");
        return false;
    }

    XML_SetUserData(p, this);

    XML_SetElementHandler(p, start, end);
    XML_SetCharacterDataHandler(p, chars);

    for (;;) {
        int done;
        int len;

        len = (int)_s.length();
        done = 1;

        if (XML_Parse(p, _s.data(), len, done) == XML_STATUS_ERROR) {
            fprintf(stderr, "Parse error at line %lu:\n%s\n", XML_GetCurrentLineNumber(p),
                    XML_ErrorString(XML_GetErrorCode(p)));
            XML_ParserFree(p);
            return false;
        }

        if (done) break;
    }
    XML_ParserFree(p);
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool WebView::drawHTML() {
    _width = rect().size.width;
    _pt = makePoint(0.0f, rect().size.height - fontHeight(SystemFonts::sharedInstance()->semiboldFont()));

    _parserState = RootState;

    _isDrawing = true;
    bool ret = parse();
    _isDrawing = false;

    return ret;
}

// ---------------------------------------------------------------------------------------------------------------------
float WebView::contentHeight(float width) {
    _pt = makeZeroPoint();
    _width = width;
    bool ret = parse();
    if (ret) {
        return -_pt.y;
    }
    return 0.0f;
}

// ---------------------------------------------------------------------------------------------------------------------
void WebView::draw() {
    DrawContext* dc = drawContext();
    dc->pushStates();
    dc->setFillColor(whiteColor);
    dc->drawRect(bounds());
    dc->popStates();
    drawHTML();
}
