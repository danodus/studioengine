//
//  path.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2018-01-15.
//  Copyright (c) 2016-2020 Daniel Cliche. All rights reserved.
//

#include "path.h"

#include <platform.h>

#include "bezier.h"

using namespace MDStudio;

class MovePathCmd : public PathCmd {
    Point _pt;
    bool _isRelative;

   public:
    MovePathCmd(Path* path, Point pt, bool isRelative) : PathCmd(path), _pt(pt), _isRelative(isRelative){};
    void execute() override;
};

class LinePathCmd : public PathCmd {
    Point _pt;
    bool _isRelative;

   public:
    LinePathCmd(Path* path, Point pt, bool isRelative) : PathCmd(path), _pt(pt), _isRelative(isRelative){};
    void execute() override;
};

class CubicCurvePathCmd : public PathCmd {
    Point _pt0, _pt1, _pt2;
    bool _isRelative;

   public:
    CubicCurvePathCmd(Path* path, Point pt0, Point pt1, Point pt2, bool isRelative)
        : PathCmd(path), _pt0(pt0), _pt1(pt1), _pt2(pt2), _isRelative(isRelative){};
    void execute() override;
};

class SmoothCubicCurvePathCmd : public PathCmd {
    Point _pt0, _pt1;
    bool _isRelative;

   public:
    SmoothCubicCurvePathCmd(Path* path, Point pt0, Point pt1, bool isRelative)
        : PathCmd(path), _pt0(pt0), _pt1(pt1), _isRelative(isRelative){};
    void execute() override;
};

class QuadraticCurvePathCmd : public PathCmd {
    Point _pt0, _pt1;
    bool _isRelative;

   public:
    QuadraticCurvePathCmd(Path* path, Point pt0, Point pt1, bool isRelative)
        : PathCmd(path), _pt0(pt0), _pt1(pt1), _isRelative(isRelative){};
    void execute() override;
};

class HorizontalLinePathCmd : public PathCmd {
    float _x;
    bool _isRelative;

   public:
    HorizontalLinePathCmd(Path* path, float x, bool isRelative) : PathCmd(path), _x(x), _isRelative(isRelative){};
    void execute() override;
};

class VerticalLinePathCmd : public PathCmd {
    float _y;
    bool _isRelative;

   public:
    VerticalLinePathCmd(Path* path, float y, bool isRelative) : PathCmd(path), _y(y), _isRelative(isRelative){};
    void execute() override;
};

// ---------------------------------------------------------------------------------------------------------------------
void MovePathCmd::execute() {
    auto pt = _pt;
    if (_isRelative) {
        pt.x += _path->point().x;
        pt.y += _path->point().y;
    }
    _path->addPoint(pt);
    _path->setSmoothCurveControlPoint(pt);
}

// ---------------------------------------------------------------------------------------------------------------------
void LinePathCmd::execute() {
    auto pt = _pt;
    if (_isRelative) {
        pt.x += _path->point().x;
        pt.y += _path->point().y;
    }
    _path->addPoint(pt);
}

// ---------------------------------------------------------------------------------------------------------------------
void CubicCurvePathCmd::execute() {
    Point p = _path->lastPoint();

    auto pt0 = _pt0;
    auto pt1 = _pt1;
    auto pt2 = _pt2;

    if (_isRelative) {
        pt0.x += _path->point().x;
        pt0.y += _path->point().y;
        pt1.x += _path->point().x;
        pt1.y += _path->point().y;
        pt2.x += _path->point().x;
        pt2.y += _path->point().y;
    }

    Vector2 q0 = calculateCubicBezierPoint(0, Vector2(p.x, p.y), Vector2(pt0.x, pt0.y), Vector2(pt1.x, pt1.y),
                                           Vector2(pt2.x, pt2.y));
    const int nbSegments = 100;
    for (int i = 1; i <= nbSegments; ++i) {
        float t = (float)i / (float)(nbSegments);
        Vector2 q1 = calculateCubicBezierPoint(t, Vector2(p.x, p.y), Vector2(pt0.x, pt0.y), Vector2(pt1.x, pt1.y),
                                               Vector2(pt2.x, pt2.y));

        _path->addPoint(makePoint(q1.x(), q1.y()));

        q0 = q1;
    }

    float dx = pt2.x - pt1.x;
    float dy = pt2.y - pt1.y;

    _path->setSmoothCurveControlPoint(makePoint(pt2.x - dx, pt2.y - dy));
}

// ---------------------------------------------------------------------------------------------------------------------
void SmoothCubicCurvePathCmd::execute() {
    Point p = _path->lastPoint();
    Point c = _path->smoothCurveControlPoint();

    auto pt0 = _pt0;
    auto pt1 = _pt1;

    if (_isRelative) {
        pt0.x += _path->point().x;
        pt0.y += _path->point().y;
        pt1.x += _path->point().x;
        pt1.y += _path->point().y;
    }

    Vector2 q0 = calculateCubicBezierPoint(0, Vector2(p.x, p.y), Vector2(c.x, c.y), Vector2(pt0.x, pt0.y),
                                           Vector2(pt1.x, pt1.y));
    const int nbSegments = 100;
    for (int i = 1; i <= nbSegments; ++i) {
        float t = (float)i / (float)(nbSegments);
        Vector2 q1 = calculateCubicBezierPoint(t, Vector2(p.x, p.y), Vector2(c.x, c.y), Vector2(pt0.x, pt0.y),
                                               Vector2(pt1.x, pt1.y));

        _path->addPoint(makePoint(q1.x(), q1.y()));

        q0 = q1;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void QuadraticCurvePathCmd::execute() {
    Point p = _path->lastPoint();

    auto pt0 = _pt0;
    auto pt1 = _pt1;

    if (_isRelative) {
        pt0.x += _path->point().x;
        pt0.y += _path->point().y;
        pt1.x += _path->point().x;
        pt1.y += _path->point().y;
    }

    Vector2 q0 = calculateQuadraticBezierPoint(0, Vector2(p.x, p.y), Vector2(pt0.x, pt0.y), Vector2(pt1.x, pt1.y));
    const int nbSegments = 100;
    for (int i = 1; i <= nbSegments; ++i) {
        float t = (float)i / (float)(nbSegments);
        Vector2 q1 = calculateQuadraticBezierPoint(t, Vector2(p.x, p.y), Vector2(pt0.x, pt0.y), Vector2(pt1.x, pt1.y));

        _path->addPoint(makePoint(q1.x(), q1.y()));

        q0 = q1;
    }

    // TODO: Set smooth point
}

// ---------------------------------------------------------------------------------------------------------------------
void HorizontalLinePathCmd::execute() {
    auto x = _x;
    if (_isRelative) x += _path->point().x;
    _path->addPoint(makePoint(x, _path->lastPoint().y));
}

// ---------------------------------------------------------------------------------------------------------------------
void VerticalLinePathCmd::execute() {
    auto y = _y;
    if (_isRelative) y += _path->point().y;
    _path->addPoint(makePoint(_path->lastPoint().x, y));
}

// ---------------------------------------------------------------------------------------------------------------------
Path::Path() {}

// ---------------------------------------------------------------------------------------------------------------------
Path::~Path() {
    for (auto cmd : _cmds) delete cmd;
}

// ---------------------------------------------------------------------------------------------------------------------
void Path::addMoveCmd(Point pt, bool isRelative) { _cmds.push_back(new MovePathCmd(this, pt, isRelative)); }

// ---------------------------------------------------------------------------------------------------------------------
void Path::addLineCmd(Point pt, bool isRelative) { _cmds.push_back(new LinePathCmd(this, pt, isRelative)); }

// ---------------------------------------------------------------------------------------------------------------------
void Path::addCubicCurveCmd(Point pt0, Point pt1, Point pt2, bool isRelative) {
    _cmds.push_back(new CubicCurvePathCmd(this, pt0, pt1, pt2, isRelative));
}

// ---------------------------------------------------------------------------------------------------------------------
void Path::addSmoothCubicCurveCmd(Point pt0, Point pt1, bool isRelative) {
    _cmds.push_back(new SmoothCubicCurvePathCmd(this, pt0, pt1, isRelative));
}

// ---------------------------------------------------------------------------------------------------------------------
void Path::addQuadraticCurveCmd(Point pt0, Point pt1, bool isRelative) {
    _cmds.push_back(new QuadraticCurvePathCmd(this, pt0, pt1, isRelative));
}

// ---------------------------------------------------------------------------------------------------------------------
void Path::addHorizontalLineCmd(float x, bool isRelative) {
    _cmds.push_back(new HorizontalLinePathCmd(this, x, isRelative));
}

// ---------------------------------------------------------------------------------------------------------------------
void Path::addVerticalLineCmd(float y, bool isRelative) {
    _cmds.push_back(new VerticalLinePathCmd(this, y, isRelative));
}

// ---------------------------------------------------------------------------------------------------------------------
void Path::execute() {
    for (auto cmd : _cmds) cmd->execute();

    // Dump points
    /*
     printf("--------------------\n");
     for (auto p : _points) {
     printf("x: %g, y: %g\n", p.x, p.y);
     }
     */
}
