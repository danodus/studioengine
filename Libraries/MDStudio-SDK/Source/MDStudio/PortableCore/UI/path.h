//
//  path.h
//  MDStudio
//
//  Created by Daniel Cliche on 2018-01-15.
//  Copyright (c) 2016-2020 Daniel Cliche. All rights reserved.
//

#ifndef PATH_H
#define PATH_H

#include <point.h>

#include <vector>

namespace MDStudio {

class Path;
class PathCmd {
   protected:
    Path* _path;

   public:
    PathCmd(Path* path) : _path(path) {}
    virtual ~PathCmd() {}
    virtual void execute() = 0;
};

class Path {
    std::vector<PathCmd*> _cmds;

    std::vector<Point> _points;

    Point _point;
    Point _smoothCurveControlPoint;
    Point _lastPoint;

   public:
    Path();
    ~Path();

    void setPoint(Point pt) { _point = pt; }
    Point point() { return _point; }

    Point lastPoint() { return _lastPoint; }

    void clearPoints() { _points.clear(); }
    void execute();

    std::vector<Point>& points() { return _points; }

    void setSmoothCurveControlPoint(Point pt) { _smoothCurveControlPoint = pt; }
    Point smoothCurveControlPoint() { return _smoothCurveControlPoint; }

    void addPoint(Point pt) {
        _points.push_back(pt);
        _lastPoint = pt;
        _point = pt;
    }

    bool hasCommands() { return _cmds.size() > 0; }
    void addMoveCmd(Point pt, bool isRelative);
    void addLineCmd(Point pt, bool isRelative);
    void addCubicCurveCmd(Point pt0, Point pt1, Point pt2, bool isRelative);
    void addSmoothCubicCurveCmd(Point pt0, Point pt1, bool isRelative);
    void addQuadraticCurveCmd(Point pt0, Point pt1, bool isRelative);
    // TODO: addSmoothQuadraticCurveCmd
    void addHorizontalLineCmd(float x, bool isRelative);
    void addVerticalLineCmd(float x, bool isRelative);
};

}  // namespace MDStudio

#endif  // PATH_H
