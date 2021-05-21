//
//  drawcontext.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2018-01-15.
//  Copyright (c) 2016-2020 Daniel Cliche. All rights reserved.
//

#include "drawcontext.h"

#include <math.h>

#include <vector>

#include "draw.h"
#include "triangulate.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
void DrawContext::setTransform(DrawContextStates states) {
    ::setScissor(states.scissor);
    ::setTransform(states.translation, states.rotation);
}

// ---------------------------------------------------------------------------------------------------------------------
void DrawContext::drawRect(Rect rect) {
    DrawContextStates states = _states.top();

    if (states.fillColor.alpha > 0.0f) {
        _cmds.push_back([=] {
            setTransform(states);
            ::drawFilledRect(getRect(rect, states.scaleX, states.scaleY), states.fillColor);
        });
    }

    if (_states.top().strokeWidth > 0 && states.strokeColor.alpha > 0.0f) {
        _cmds.push_back([=] {
            setTransform(states);
            ::drawRect(getRect(rect, states.scaleX, states.scaleY), states.strokeColor,
                       states.scaleX * states.strokeWidth);
        });
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void DrawContext::drawRectGradientV(Rect rect, Color fillColorBottom, Color fillColorTop) {
    DrawContextStates states = _states.top();

    _cmds.push_back([=] {
        setTransform(states);
        ::drawFilledRectGradientV(getRect(rect, states.scaleX, states.scaleY), fillColorBottom, fillColorTop);
    });

    if (_states.top().strokeWidth > 0 && states.strokeColor.alpha > 0.0f) {
        _cmds.push_back([=] {
            setTransform(states);
            ::drawRect(getRect(rect, states.scaleX, states.scaleY), states.strokeColor,
                       states.scaleX * states.strokeWidth);
        });
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void DrawContext::drawTriangle(Point p0, Point p1, Point p2) {
    DrawContextStates states = _states.top();

    if (states.fillColor.alpha > 0.0f) {
        _cmds.push_back([=] {
            setTransform(states);
            ::drawFilledTriangle(getPt(p0, states.scaleX, states.scaleY), getPt(p1, states.scaleX, states.scaleY),
                                 getPt(p2, states.scaleX, states.scaleY), states.fillColor);
        });
    }

    if (_states.top().strokeWidth > 0 && states.strokeColor.alpha > 0.0f) {
        _cmds.push_back([=] {
            setTransform(states);
            ::drawTriangle(getPt(p0, states.scaleX, states.scaleY), getPt(p1, states.scaleX, states.scaleY),
                           getPt(p2, states.scaleX, states.scaleY), states.strokeColor, states.strokeWidth);
        });
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void DrawContext::drawCircle(Point p, float radius) {
    DrawContextStates states = _states.top();

    if (states.fillColor.alpha > 0.0f) {
        _cmds.push_back([=] {
            setTransform(states);
            ::drawFilledArc2(getPt(p, states.scaleX, states.scaleY), states.scaleX * radius, states.scaleY * radius,
                             0.0, 2.0 * M_PI, states.fillColor);
        });
    }

    if (_states.top().strokeWidth > 0 && states.strokeColor.alpha > 0.0f) {
        _cmds.push_back([=] {
            setTransform(states);
            ::drawArc2(getPt(p, states.scaleX, states.scaleY), states.scaleX * radius, states.scaleY * radius, 0.0,
                       2.0 * M_PI, states.strokeColor, states.scaleX * states.strokeWidth);
        });
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void DrawContext::drawEllipse(Point p, float radiusX, float radiusY) {
    DrawContextStates states = _states.top();

    if (states.fillColor.alpha > 0.0f) {
        _cmds.push_back([=] {
            setTransform(states);
            ::drawFilledArc2(getPt(p, states.scaleX, states.scaleY), states.scaleX * radiusX, states.scaleY * radiusY,
                             0.0, 2.0 * M_PI, states.fillColor);
        });
    }

    if (_states.top().strokeWidth > 0 && states.strokeColor.alpha > 0.0f) {
        _cmds.push_back([=] {
            setTransform(states);
            ::drawArc2(getPt(p, states.scaleX, states.scaleY), states.scaleX * radiusX, states.scaleY * radiusY, 0.0,
                       2.0 * M_PI, states.strokeColor, states.scaleX * states.strokeWidth);
        });
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void DrawContext::drawLine(Point p0, Point p1) {
    DrawContextStates states = _states.top();
    _cmds.push_back([=] {
        setTransform(states);
        ::drawLine(getPt(p0, states.scaleX, states.scaleY), getPt(p1, states.scaleX, states.scaleY), states.strokeColor,
                   states.scaleX * states.strokeWidth);
    });
}

// ---------------------------------------------------------------------------------------------------------------------
void DrawContext::drawPolygon(Path* path, bool isOutlineClosed) {
    if (path->hasCommands()) {
        path->clearPoints();
        path->execute();
    }

    if (path->points().size() == 0) return;

    //
    // Draw filled path
    //

    DrawContextStates states = _states.top();

    if (states.fillColor.alpha > 0.0f) {
        Vector2dVector a;

        for (auto pt : path->points()) {
            a.push_back(Vector2(pt.x, pt.y));
        }

        // allocate an STL vector to hold the answer.
        Vector2dVector result;

        //  Invoke the triangulator to triangulate this polygon.
        if (!Triangulate::Process(a, result)) {
            // printf("UNABLE TO TRIANGULATE!!!\n");
        }

        int tcount = (int)result.size() / 3;

        for (int i = 0; i < tcount; i++) {
            Vector2& p0 = result[i * 3 + 0];
            Vector2& p1 = result[i * 3 + 1];
            Vector2& p2 = result[i * 3 + 2];
            Point pt0 = makePoint(p0.x(), p0.y());
            Point pt1 = makePoint(p1.x(), p1.y());
            Point pt2 = makePoint(p2.x(), p2.y());
            _cmds.push_back([=] {
                setTransform(states);
                ::drawFilledTriangle(getPt(pt0, states.scaleX, states.scaleY), getPt(pt1, states.scaleX, states.scaleY),
                                     getPt(pt2, states.scaleX, states.scaleY), states.fillColor);
            });
        }
    }

    //
    // Draw outline
    //

    if (_states.top().strokeWidth > 0 && _states.top().strokeColor.alpha > 0.0f) {
        if (path->points().size() < 2) return;

        std::deque<Point> pts;

        if (path->points().size() >= 3) {
            if (isOutlineClosed) {
                for (const auto& p : path->points()) pts.push_back(p);
                pts.push_back(path->points()[0]);
                pts.push_back(path->points()[1]);
                pts.push_back(path->points()[2]);
            } else {
                pts.push_back(path->points()[0]);
                for (const auto& p : path->points()) pts.push_back(p);
                pts.push_back(*(path->points().end() - 1));
            }
        } else {
            // Line only
            pts.push_back(path->points()[0]);
            pts.push_back(path->points()[0]);
            pts.push_back(path->points()[1]);
            pts.push_back(path->points()[1]);
        }

        _cmds.push_back([=] {
            setTransform(states);
            auto pts2 = pts;
            while (pts2.size() >= 4) {
                ::drawSegment(
                    getPt(pts2[0], states.scaleX, states.scaleY), getPt(pts2[1], states.scaleX, states.scaleY),
                    getPt(pts2[2], states.scaleX, states.scaleY), getPt(pts2[3], states.scaleX, states.scaleY),
                    states.strokeColor, states.scaleX * states.strokeWidth);
                pts2.pop_front();
            }
        });
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void DrawContext::drawPolyline(Path* path) {
    DrawContextStates states = _states.top();

    if (path->hasCommands()) {
        path->clearPoints();
        path->execute();
    }

    if (path->points().size() < 2) return;

    std::deque<Point> pts;
    pts.push_back(path->points()[0]);
    for (const auto& p : path->points()) pts.push_back(p);
    pts.push_back(*(path->points().end() - 1));

    _cmds.push_back([=] {
        setTransform(states);
        auto pts2 = pts;
        while (pts2.size() >= 4) {
            ::drawSegment(getPt(pts2[0], states.scaleX, states.scaleY), getPt(pts2[1], states.scaleX, states.scaleY),
                          getPt(pts2[2], states.scaleX, states.scaleY), getPt(pts2[3], states.scaleX, states.scaleY),
                          states.strokeColor, states.scaleX * states.strokeWidth);
            pts2.pop_front();
        }
    });
}

// ---------------------------------------------------------------------------------------------------------------------
void DrawContext::drawArc(Point p, float radius, double startAngle, double arcAngle) {
    DrawContextStates states = _states.top();
    if (states.fillColor.alpha > 0.0f) {
        _cmds.push_back([=] {
            setTransform(states);
            ::drawFilledArc(p, radius, startAngle, arcAngle, states.fillColor);
        });
    }
    if (_states.top().strokeWidth > 0 && states.strokeColor.alpha > 0.0f) {
        _cmds.push_back([=] {
            setTransform(states);
            ::drawArc(p, radius, startAngle, arcAngle, states.strokeColor, states.strokeWidth);
        });
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void DrawContext::drawRoundRect(Rect rect, float radius) {
    DrawContextStates states = _states.top();
    if (states.fillColor.alpha > 0.0f) {
        _cmds.push_back([=] {
            setTransform(states);
            ::drawFilledRoundRect(rect, radius, states.fillColor);
        });
    }
    if (_states.top().strokeWidth > 0 && states.strokeColor.alpha > 0.0f) {
        _cmds.push_back([=] {
            setTransform(states);
            ::drawRoundRect(rect, radius, states.strokeColor, states.strokeWidth);
        });
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void DrawContext::drawText(MultiDPIFont* font, Point pt, const std::string& text, float angle) {
    DrawContextStates states = _states.top();
    _cmds.push_back([=] {
        setTransform(states);
        ::drawText(font, pt, states.strokeColor, text, angle);
    });
}

// ---------------------------------------------------------------------------------------------------------------------
void DrawContext::drawLeftText(MultiDPIFont* font, Rect rect, const std::string& text) {
    DrawContextStates states = _states.top();
    _cmds.push_back([=] {
        setTransform(states);
        ::drawLeftText(font, rect, states.strokeColor, text);
    });
}

// ---------------------------------------------------------------------------------------------------------------------
void DrawContext::drawCenteredText(MultiDPIFont* font, Rect rect, const std::string& text) {
    DrawContextStates states = _states.top();
    _cmds.push_back([=] {
        setTransform(states);
        ::drawCenteredText(font, rect, states.strokeColor, text);
    });
}

// ---------------------------------------------------------------------------------------------------------------------
void DrawContext::drawRightText(MultiDPIFont* font, Rect rect, const std::string& text) {
    DrawContextStates states = _states.top();
    _cmds.push_back([=] {
        setTransform(states);
        ::drawRightText(font, rect, states.strokeColor, text);
    });
}

// ---------------------------------------------------------------------------------------------------------------------
void DrawContext::drawImage(Rect rect, std::shared_ptr<Image> image, Color color) {
    DrawContextStates states = _states.top();
    _cmds.push_back([=] {
        setTransform(states);
        ::drawImage(rect, image.get(), color);
    });
}

// ---------------------------------------------------------------------------------------------------------------------
void DrawContext::drawImage(Point pt, std::shared_ptr<Image> image, Color color) {
    DrawContextStates states = _states.top();
    _cmds.push_back([=] {
        setTransform(states);
        ::drawImage(pt, image.get(), color);
    });
}

// ---------------------------------------------------------------------------------------------------------------------
void DrawContext::draw() {
    auto oldScissor = ::scissor();

    if (_clearRegion.size.width > 0.0f && _clearRegion.size.height > 0.0f) {
        ::setScissor(_clearRegion);
        ::clear();
    }

    _clearRegion = {0.0f, 0.0f, -1.0f, -1.0f};

    // Draw back to front
    while (!_cmds.empty()) {
        auto cmd = _cmds.front();
        cmd();
        _cmds.pop_front();
    }

    ::setScissor(oldScissor);
}
