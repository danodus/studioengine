//
//  draw.h
//  MDStudio
//
//  Created by Daniel Cliche on 2013-09-09.
//  Copyright (c) 2013-2020 Daniel Cliche. All rights reserved.
//

#ifndef DRAW_H
#define DRAW_H

#include <string>

#include "color.h"
#include "font.h"
#include "image.h"
#include "point.h"
#include "rect.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace MDStudio {

void setBackingStoreScale(float scale);
float backingStoreScale();

void setViewport(Rect rect);
void setScissor(Rect rect);
Rect scissor();
void clear();
void setTransform(Point translationPoint, float rotationAngle);
Point translation();
float rotation();
void drawRect(Rect rect, Color color, float lineWidth = 1.0f);
void drawFilledRect(Rect rect, Color fillColor);
void drawLine(Point p1, Point p2, Color color, float thickness = 1.0f);
void drawSegment(Point p0, Point p1, Point p2, Point p3, Color color, float thickness = 1.0f);
void drawFilledRectGradientV(Rect rect, Color fillColorBottom, Color fillColorTop);
void drawRoundRect(Rect rect, float radius, Color color, float lineWidth = 1.0f);
void drawFilledRoundRect(Rect rect, float radius, Color fillColor);
void drawArc(MDStudio::Point pt, float radius, double startAngle, double arcAngle, Color color, float lineWidth = 1.0f);
void drawArc2(MDStudio::Point pt, float radiusX, float radiusY, double startAngle, double arcAngle, Color color,
              float lineWidth = 1.0f);
void drawFilledArc(MDStudio::Point pt, float radius, double startAngle, double arcAngle, Color fillColor);
void drawFilledArc2(MDStudio::Point pt, float radiusX, float radiusY, double startAngle, double arcAngle,
                    Color fillColor);
void drawTriangle(Point p0, Point p1, Point p2, Color color, float thickness = 1.0f);
void drawFilledTriangle(Point p0, Point p1, Point p2, Color fillColor);
void drawText(MultiDPIFont* font, Point pt, Color color, const std::string& text, float angle = 0.0f);
void drawCenteredText(MultiDPIFont* font, Rect rect, Color color, const std::string& text);
void drawLeftText(MultiDPIFont* font, Rect rect, Color color, const std::string& text);
void drawRightText(MultiDPIFont* font, Rect rect, Color color, const std::string& text);
float getTextWidth(MultiDPIFont* font, const std::string& text);
float fontHeight(MultiDPIFont* font);
void drawImage(Rect rect, Image* image, Color color = whiteColor);
void drawImage(Point pt, Image* image, Color color = whiteColor);
bool isPointInRect(Point pt, Rect rect);
bool isRectInRect(Rect rect1, Rect rect2);
double angleBetweenPoints(Point p1, Point p2);
float distanceBetweenPoints(Point p1, Point p2);

}  // namespace MDStudio

#endif  // DRAW_H
