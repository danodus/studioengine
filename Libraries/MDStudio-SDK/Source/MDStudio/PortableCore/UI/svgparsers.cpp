//
//  svgparsers.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2018-01-16.
//  Copyright (c) 2018-2021 Daniel Cliche. All rights reserved.
//

#include "svgparsers.h"

#include <string>

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
static float parseFloat(const char** s) {
    std::string v;

    while (**s == ' ') (*s)++;

    bool isFirst = true;
    while ((**s >= '0' && **s <= '9') || (**s == '.') || (isFirst && (**s == '-'))) {
        v.push_back(**s);
        (*s)++;
        isFirst = false;
    }

    while (**s == ',' || **s == ' ') (*s)++;

    return std::stof(v);
}

// ---------------------------------------------------------------------------------------------------------------------
bool MDStudio::parseSVGPolygon(Path* path, const char* s) {
    float x = 0.0f, y = 0.0f;
    int coordinateIndex = 0;
    int valueIndex = 0;
    while (*s != '\0') {
        auto value = parseFloat(&s);
        if (coordinateIndex == 0) {
            x = value;
            ++coordinateIndex;
        } else {
            y = value;
            coordinateIndex = 0;
            if (valueIndex == 0) {
                path->addMoveCmd(makePoint(x, y), false);
            } else {
                path->addLineCmd(makePoint(x, y), false);
            }
            ++valueIndex;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool MDStudio::parseSVGPath(Path* path, const char* s, bool* isOutlineClosed) {
    enum States {
        CommandState,
        MoveState,
        LineState,
        CurveState,
        SmoothCurveState,
        QuadraticCurveState,
        SmoothQuadraticCurveState,
        VerticalLineState,
        HorizontalLineState,
        EllipticalArcState
    };

    States state = CommandState;
    States lastState = state;
    bool isRelative = false;
    bool lastIsRelative = false;
    if (isOutlineClosed) *isOutlineClosed = false;
    while (*s != '\0') {
        switch (state) {
            case CommandState:
                isRelative = islower(*s);
                if (*s == 'M' || *s == 'm') {
                    s++;
                    state = MoveState;
                } else if (*s == 'L' || *s == 'l') {
                    s++;
                    state = LineState;
                } else if (*s == 'C' || *s == 'c') {
                    s++;
                    state = CurveState;
                } else if (*s == 'S' || *s == 's') {
                    s++;
                    state = SmoothCurveState;
                } else if (*s == 'Q' || *s == 'q') {
                    s++;
                    state = QuadraticCurveState;
                } else if (*s == 'T' || *s == 't') {
                    s++;
                    state = SmoothQuadraticCurveState;
                } else if (*s == 'V' || *s == 'v') {
                    s++;
                    state = VerticalLineState;
                } else if (*s == 'H' || *s == 'h') {
                    s++;
                    state = HorizontalLineState;
                } else if (*s == 'A' || *s == 'a') {
                    s++;
                    state = EllipticalArcState;
                } else if (*s == 'Z' || *s == 'z') {
                    if (isOutlineClosed) *isOutlineClosed = true;
                    return true;
                } else {
                    // repeat last
                    state = lastState;
                    isRelative = lastIsRelative;
                }
                break;
            case MoveState: {
                lastState = state;
                lastIsRelative = isRelative;
                float f0 = parseFloat(&s);
                float f1 = parseFloat(&s);
                path->addMoveCmd(makePoint(f0, f1), isRelative);
                state = CommandState;
            } break;
            case LineState: {
                lastState = state;
                lastIsRelative = isRelative;
                float f0 = parseFloat(&s);
                float f1 = parseFloat(&s);
                path->addLineCmd(makePoint(f0, f1), isRelative);
                state = CommandState;
            } break;
            case CurveState: {
                lastState = state;
                lastIsRelative = isRelative;
                float f0 = parseFloat(&s);
                float f1 = parseFloat(&s);
                float f2 = parseFloat(&s);
                float f3 = parseFloat(&s);
                float f4 = parseFloat(&s);
                float f5 = parseFloat(&s);
                path->addCubicCurveCmd(makePoint(f0, f1), makePoint(f2, f3), makePoint(f4, f5), isRelative);
                state = CommandState;
            } break;
            case SmoothCurveState: {
                lastState = state;
                lastIsRelative = isRelative;
                float f0 = parseFloat(&s);
                float f1 = parseFloat(&s);
                float f2 = parseFloat(&s);
                float f3 = parseFloat(&s);
                path->addSmoothCubicCurveCmd(makePoint(f0, f1), makePoint(f2, f3), isRelative);
                state = CommandState;
            } break;
            case QuadraticCurveState: {
                lastState = state;
                lastIsRelative = isRelative;
                float f0 = parseFloat(&s);
                float f1 = parseFloat(&s);
                float f2 = parseFloat(&s);
                float f3 = parseFloat(&s);
                path->addQuadraticCurveCmd(makePoint(f0, f1), makePoint(f2, f3), isRelative);
                state = CommandState;
            } break;
            case SmoothQuadraticCurveState: {
                lastState = state;
                lastIsRelative = isRelative;
                // TODO
                state = CommandState;
            } break;
            case VerticalLineState: {
                lastState = state;
                lastIsRelative = isRelative;
                float f0 = parseFloat(&s);
                path->addVerticalLineCmd(f0, isRelative);
                state = CommandState;
            } break;
            case HorizontalLineState: {
                lastState = state;
                lastIsRelative = isRelative;
                float f0 = parseFloat(&s);
                path->addHorizontalLineCmd(f0, isRelative);
                state = CommandState;
            } break;
            case EllipticalArcState: {
                lastState = state;
                lastIsRelative = isRelative;
                // TODO
                state = CommandState;
            } break;
        }
    }

    return true;
}
