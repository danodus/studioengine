//
//  drawcontext.h
//  MDStudio
//
//  Created by Daniel Cliche on 2018-01-15.
//  Copyright (c) 2016-2020 Daniel Cliche. All rights reserved.
//

#ifndef DRAWCONTEXT_H
#define DRAWCONTEXT_H

#include <functional>
#include <stack>

#include "color.h"
#include "font.h"
#include "image.h"
#include "path.h"
#include "rect.h"

namespace MDStudio {

struct DrawContextStates {
    float scaleX, scaleY;
    Rect scissor;
    Point translation;
    float rotation;

    Color strokeColor;
    Color fillColor;
    float strokeWidth;
};

class DrawContext {
    std::deque<std::function<void()>> _cmds;
    std::stack<DrawContextStates> _states;
    Rect _clearRegion;

    Point getPt(Point pt, float scaleX, float scaleY) { return makePoint(scaleX * pt.x, scaleY * pt.y); }

    Rect getRect(Rect rect, float scaleX, float scaleY) {
        Point o = getPt(rect.origin, scaleX, scaleY);
        float w = scaleX * rect.size.width;
        float h = scaleY * rect.size.height;
        return makeRect(o.x, o.y, w, h);
    }

    void setTransform(DrawContextStates states);

   public:
    DrawContext() {
        _clearRegion = {0.0f, 0.0f, -1.0f, -1.0f};
        DrawContextStates states;
        _states.push(states);
        setScaleX(1.0f);
        setScaleY(1.0f);
        setTranslation(makeZeroPoint());
        setRotation(0.0f);
        setScissor({{0, 0}, {-1, -1}});
        resetStyle();
    }

    void setClearRegion(Rect clearRegion) { _clearRegion = clearRegion; };

    void pushStates() { _states.push(_states.top()); }

    void popStates() { _states.pop(); }

    size_t nbStates() { return _states.size(); }

    void drawRect(Rect rect);
    void drawRectGradientV(Rect rect, Color fillColorBottom, Color fillColorTop);
    void drawTriangle(Point p0, Point p1, Point p2);
    void drawCircle(Point p, float radius);
    void drawEllipse(Point p, float radiusX, float radiusY);
    void drawLine(Point p0, Point p1);
    void drawPolygon(Path* path, bool isOutlineClosed = true);
    void drawPolyline(Path* path);
    void drawArc(Point p, float radius, double startAngle, double arcAngle);
    void drawRoundRect(Rect rect, float radius);

    void drawText(MultiDPIFont* font, Point pt, const std::string& text, float angle = 0.0f);
    void drawLeftText(MultiDPIFont* font, Rect rect, const std::string& text);
    void drawCenteredText(MultiDPIFont* font, Rect rect, const std::string& text);
    void drawRightText(MultiDPIFont* font, Rect rect, const std::string& text);

    void drawImage(Rect rect, std::shared_ptr<Image> image, Color color = whiteColor);
    void drawImage(Point pt, std::shared_ptr<Image> image, Color color = whiteColor);

    void drawFn(std::function<void()> fn) { _cmds.push_back(fn); }

    void draw();

    void setScissor(Rect scissor) { _states.top().scissor = scissor; }
    Rect scissor() { return _states.top().scissor; }

    void setTranslation(Point translation) { _states.top().translation = translation; }
    Point translation() { return _states.top().translation; }

    void setRotation(float rotation) { _states.top().rotation = rotation; }
    float rotation() { return _states.top().rotation; }

    void setScaleX(float scaleX) { _states.top().scaleX = scaleX; }
    float scaleX() { return _states.top().scaleX; }

    void setScaleY(float scaleY) { _states.top().scaleY = scaleY; }
    float scaleY() { return _states.top().scaleY; }

    void resetStyle() {
        _states.top().strokeColor = zeroColor;
        _states.top().fillColor = zeroColor;
        _states.top().strokeWidth = 1.0f;
    }

    void setStrokeColor(Color strokeColor) { _states.top().strokeColor = strokeColor; }
    Color strokeColor() { return _states.top().strokeColor; }

    void setFillColor(Color fillColor) { _states.top().fillColor = fillColor; }
    Color fillColor() { return _states.top().fillColor; }

    void setStrokeWidth(float strokeWidth) { _states.top().strokeWidth = strokeWidth; }
    float strokeWidth() { return _states.top().strokeWidth; }
};

}  // namespace MDStudio

#endif  // DRAWCONTEXT_H
