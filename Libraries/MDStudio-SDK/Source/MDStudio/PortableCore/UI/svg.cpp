//
//  svg.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2018-01-15.
//  Copyright © 2016-2020 Daniel Cliche. All rights reserved.
//

#include "svg.h"

#include <expat.h>
#include <platform.h>

#include <cassert>
#include <map>

#include "draw.h"
#include "path.h"
#include "svgparsers.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
std::map<std::string, std::string> parseParameters(std::string s) {
    std::string key;
    std::string value;

    std::map<std::string, std::string> map;

    bool isParsingValue = false;

    for (auto c : s) {
        if (c == ' ') continue;
        if (isParsingValue) {
            if (c == ';') {
                isParsingValue = false;
                map.insert(std::make_pair(key, value));
                key = "";
                value = "";
            } else {
                value += c;
            }
        } else {
            if (c == ':') {
                isParsingValue = true;
            } else {
                key += c;
            }
        }
    }

    if (isParsingValue) map.insert(std::make_pair(key, value));

    return map;
}

// ---------------------------------------------------------------------------------------------------------------------
void setColorComponent(Color* color, int index, int value) {
    auto v = (float)value / 255.0f;
    if (index == 0) {
        color->red = v;
    } else if (index == 1) {
        color->green = v;
    } else if (index == 2) {
        color->blue = v;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
Color namedColor(std::string s) {
    const std::map<std::string, Color> colors = {
        {"none", zeroColor},     {"black", blackColor}, {"white", whiteColor},   {"red", redColor},
        {"green", greenColor},   {"blue", blueColor},   {"yellow", yellowColor}, {"pink", pinkColor},
        {"purple", purpleColor}, {"lime", limeColor}};

    Color color = blackColor;
    auto colorIt = colors.find(s);
    if (colorIt != colors.end()) {
        color = colorIt->second;
    } else if (s.substr(0, 3) == "rgb") {
        auto components = s.substr(3);
        int componentIndex = 0;
        std::string component;
        for (auto c : components) {
            if (c == '(' || c == ' ' || c == ')') continue;
            if (c == ',') {
                setColorComponent(&color, componentIndex, std::stoi(component));
                ++componentIndex;
                component = "";
            } else {
                component += c;
            }
        }
        setColorComponent(&color, componentIndex, std::stoi(component));
    }
    return color;
}

// ---------------------------------------------------------------------------------------------------------------------
void setStyle(DrawContext* drawContext, std::map<std::string, std::string> style) {
    for (auto p : style) {
        if (p.first == "fill") {
            drawContext->setFillColor(namedColor(p.second));
        } else if (p.first == "stroke") {
            drawContext->setStrokeColor(namedColor(p.second));
        } else if (p.first == "stroke-width") {
            drawContext->setStrokeWidth(std::stof(p.second));
        } else if (p.first == "stroke-opacity") {
            Color strokeColor = drawContext->strokeColor();
            strokeColor.alpha = std::stof(p.second);
            drawContext->setStrokeColor(strokeColor);
        } else if (p.first == "fill-opacity") {
            Color fillColor = drawContext->fillColor();
            fillColor.alpha = std::stof(p.second);
            drawContext->setFillColor(fillColor);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void processStyleAttribute(DrawContext* drawContext, std::string name, std::string value) {
    if (name == "style") {
        auto style = ::parseParameters(value);
        ::setStyle(drawContext, style);
    } else if (name == "fill") {
        drawContext->setFillColor(namedColor(value));
    } else if (name == "stroke") {
        drawContext->setStrokeColor(namedColor(value));
    } else if (name == "stroke-width") {
        drawContext->setStrokeWidth(std::stof(value));
    } else if (name == "stroke-opacity") {
        Color strokeColor = drawContext->strokeColor();
        strokeColor.alpha = std::stof(value);
        drawContext->setStrokeColor(strokeColor);
    } else if (name == "fill-opacity") {
        Color fillColor = drawContext->fillColor();
        fillColor.alpha = std::stof(value);
        drawContext->setFillColor(fillColor);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
static void XMLCALL start(void* data, const char* el, const char** attr) {
    SVG* svg = (SVG*)data;

    int i;

    auto element = std::string(el);

    if (svg->_parserState == SVG::RootState && element == "svg") {
        svg->_parserState = SVG::SVGState;
        for (i = 0; attr[i]; i += 2) {
            auto parameter = std::string(attr[i]);
            if (parameter == "width") {
                svg->setSize(makeSize(std::stof(attr[i + 1]), svg->size().height));
            } else if (parameter == "height") {
                svg->setSize(makeSize(svg->size().width, std::stof(attr[i + 1])));
            }
        }
    } else if (svg->_parserState == SVG::SVGState && element == "rect") {
        svg->_parserState = SVG::SVGRectState;
        if (svg->drawContext()) {
            float x = 0.0f;
            float y = 0.0f;
            float width = 0.0f;
            float height = 0.0f;
            svg->drawContext()->resetStyle();
            for (i = 0; attr[i]; i += 2) {
                auto parameter = std::string(attr[i]);
                if (parameter == "x") {
                    x = std::stof(attr[i + 1]);
                } else if (parameter == "y") {
                    y = std::stof(attr[i + 1]);
                } else if (parameter == "width") {
                    width = std::stof(attr[i + 1]);
                } else if (parameter == "height") {
                    height = std::stof(attr[i + 1]);
                } else {
                    processStyleAttribute(svg->drawContext(), parameter, attr[i + 1]);
                }
            }
            svg->drawContext()->drawRect(makeRect(x, y, width, height));
        }
    } else if (svg->_parserState == SVG::SVGState && element == "circle") {
        svg->_parserState = SVG::SVGCircleState;
        if (svg->drawContext()) {
            float cx = 0.0f;
            float cy = 0.0f;
            float r = 0.0f;
            svg->drawContext()->resetStyle();
            for (i = 0; attr[i]; i += 2) {
                auto parameter = std::string(attr[i]);
                if (parameter == "cx") {
                    cx = std::stof(attr[i + 1]);
                } else if (parameter == "cy") {
                    cy = std::stof(attr[i + 1]);
                } else if (parameter == "r") {
                    r = std::stof(attr[i + 1]);
                } else {
                    processStyleAttribute(svg->drawContext(), parameter, attr[i + 1]);
                }
            }
            svg->drawContext()->drawCircle(makePoint(cx, cy), r);
        }
    } else if (svg->_parserState == SVG::SVGState && element == "ellipse") {
        svg->_parserState = SVG::SVGEllipseState;
        if (svg->drawContext()) {
            float cx = 0.0f;
            float cy = 0.0f;
            float rx = 0.0f;
            float ry = 0.0f;
            svg->drawContext()->resetStyle();
            for (i = 0; attr[i]; i += 2) {
                auto parameter = std::string(attr[i]);
                if (parameter == "cx") {
                    cx = std::stof(attr[i + 1]);
                } else if (parameter == "cy") {
                    cy = std::stof(attr[i + 1]);
                } else if (parameter == "rx") {
                    rx = std::stof(attr[i + 1]);
                } else if (parameter == "ry") {
                    ry = std::stof(attr[i + 1]);
                } else {
                    processStyleAttribute(svg->drawContext(), parameter, attr[i + 1]);
                }
            }
            svg->drawContext()->drawEllipse(makePoint(cx, cy), rx, ry);
        }
    } else if (svg->_parserState == SVG::SVGState && element == "line") {
        svg->_parserState = SVG::SVGLineState;
        if (svg->drawContext()) {
            float x1 = 0.0f;
            float y1 = 0.0f;
            float x2 = 0.0f;
            float y2 = 0.0f;
            svg->drawContext()->resetStyle();
            for (i = 0; attr[i]; i += 2) {
                auto parameter = std::string(attr[i]);
                if (parameter == "x1") {
                    x1 = std::stof(attr[i + 1]);
                } else if (parameter == "y1") {
                    y1 = std::stof(attr[i + 1]);
                } else if (parameter == "x2") {
                    x2 = std::stof(attr[i + 1]);
                } else if (parameter == "y2") {
                    y2 = std::stof(attr[i + 1]);
                } else {
                    processStyleAttribute(svg->drawContext(), parameter, attr[i + 1]);
                }
            }
            svg->drawContext()->drawLine(makePoint(x1, y1), makePoint(x2, y2));
        }
    } else if (svg->_parserState == SVG::SVGState && element == "polygon") {
        svg->_parserState = SVG::SVGPolygonState;
        if (svg->drawContext()) {
            Path path;
            svg->drawContext()->resetStyle();
            for (i = 0; attr[i]; i += 2) {
                auto parameter = std::string(attr[i]);
                if (parameter == "points") {
                    auto value = std::string(attr[i + 1]);
                    parseSVGPolygon(&path, value.c_str());
                } else {
                    processStyleAttribute(svg->drawContext(), parameter, attr[i + 1]);
                }
            }
            svg->drawContext()->drawPolygon(&path);
        }
    } else if (svg->_parserState == SVG::SVGState && element == "polyline") {
        svg->_parserState = SVG::SVGPolylineState;
        if (svg->drawContext()) {
            Path path;
            svg->drawContext()->resetStyle();
            for (i = 0; attr[i]; i += 2) {
                auto parameter = std::string(attr[i]);
                if (parameter == "points") {
                    auto value = std::string(attr[i + 1]);
                    parseSVGPolygon(&path, value.c_str());
                } else {
                    processStyleAttribute(svg->drawContext(), parameter, attr[i + 1]);
                }
            }
            svg->drawContext()->drawPolyline(&path);
        }
    } else if (svg->_parserState == SVG::SVGState && element == "path") {
        svg->_parserState = SVG::SVGPathState;
        if (svg->drawContext()) {
            Path path;
            svg->drawContext()->resetStyle();
            bool isOutlineClosed = false;
            for (i = 0; attr[i]; i += 2) {
                auto parameter = std::string(attr[i]);
                if (parameter == "d") {
                    auto value = std::string(attr[i + 1]);
                    parseSVGPath(&path, value.c_str(), &isOutlineClosed);
                } else {
                    processStyleAttribute(svg->drawContext(), parameter, attr[i + 1]);
                }
            }
            svg->drawContext()->drawPolygon(&path, isOutlineClosed);
        }
    } else {
        assert(0);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
static void XMLCALL end(void* data, const char* el) {
    SVG* svg = (SVG*)data;

    auto element = std::string(el);

    if (svg->_parserState == SVG::SVGState && element == "svg") {
        svg->_parserState = SVG::RootState;
    } else if (svg->_parserState == SVG::SVGRectState && element == "rect") {
        svg->_parserState = SVG::SVGState;
    } else if (svg->_parserState == SVG::SVGCircleState && element == "circle") {
        svg->_parserState = SVG::SVGState;
    } else if (svg->_parserState == SVG::SVGEllipseState && element == "ellipse") {
        svg->_parserState = SVG::SVGState;
    } else if (svg->_parserState == SVG::SVGLineState && element == "line") {
        svg->_parserState = SVG::SVGState;
    } else if (svg->_parserState == SVG::SVGPolygonState && element == "polygon") {
        svg->_parserState = SVG::SVGState;
    } else if (svg->_parserState == SVG::SVGPolylineState && element == "polyline") {
        svg->_parserState = SVG::SVGState;
    } else if (svg->_parserState == SVG::SVGPathState && element == "path") {
        svg->_parserState = SVG::SVGState;
    } else {
        assert(0);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
SVG::SVG(const std::string& s) : _s(s) {
    // Initial parse without draw context in order to have the size
    _drawContext = nullptr;
    parse();
}

// ---------------------------------------------------------------------------------------------------------------------
SVG::~SVG() {}

// ---------------------------------------------------------------------------------------------------------------------
bool SVG::parse() {
    _parserState = RootState;

    XML_Parser p = XML_ParserCreate(NULL);
    if (!p) {
        fprintf(stderr, "Couldn't allocate memory for parser\n");
        return false;
    }

    XML_SetUserData(p, this);

    XML_SetElementHandler(p, start, end);

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
bool SVG::draw(DrawContext* drawContext, float scaleX, float scaleY, Point translation) {
    _drawContext = drawContext;

    _drawContext->pushStates();
    _drawContext->setScaleX(scaleX);
    _drawContext->setScaleY(scaleY);
    _drawContext->setTranslation(
        makePoint(_drawContext->translation().x + translation.x, _drawContext->translation().y + translation.y));

    if (!parse()) {
        _drawContext->popStates();
        return false;
    }

    _drawContext->popStates();

    return true;
}
