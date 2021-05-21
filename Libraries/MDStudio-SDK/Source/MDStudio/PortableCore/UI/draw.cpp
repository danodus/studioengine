//
//  draw.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2013-09-09.
//  Copyright (c) 2013-2021 Daniel Cliche. All rights reserved.
//

#include "draw.h"

#include <math.h>

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "../platform.h"
#include "bezier.h"
#include "triangulate.h"

#ifdef _WIN32

#define NOMINMAX
#include <GL/glew.h>
#include <Windows.h>

#elif _LINUX

#include <SDL2/SDL_opengl.h>

#else

#if TARGET_OS_IPHONE
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#else
#include <OpenGL/gl.h>
#endif

#endif

using namespace MDStudio;

static float currentBackingStoreScale = 1.0f;

static MDStudio::Point currentTranslation = {0, 0};
static float currentRotation = 0.0f;
static MDStudio::Rect currentScissor = {{0, 0}, {-1, -1}};

static constexpr float sqrf(float x) { return x * x; }

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::drawText(MultiDPIFont* font, Point pt, Color color, const std::string& text, float angle) {
    // Convert to utf-16
    std::vector<unsigned short> str16;
    utf8::unchecked::utf8to16(text.begin(), text.end(), back_inserter(str16));

    pt.x = floorf(pt.x);
    pt.y = floorf(pt.y);

    // GLuint f = font->fontForScale(currentBackingStoreScale)->listBase();

    // glPushAttrib(GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT);

    glColor4f(color.red, color.green, color.blue, color.alpha);

    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glShadeModel(GL_FLAT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // glListBase(f);

    glPushMatrix();
    glTranslatef(pt.x, pt.y, 0);
    glRotatef(angle, 0.0f, 0.0f, 1.0f);

    size_t length = str16.size();

    auto f = font->fontForScale(currentBackingStoreScale);

    for (size_t i = 0; i < length; ++i) {
        f->drawChar(f->charIndex(str16[i]));
    }

    glPopMatrix();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    // glPopAttrib();
}

// ---------------------------------------------------------------------------------------------------------------------
float MDStudio::getTextWidth(MultiDPIFont* font, const std::string& text) {
    float width = 0;

    // Convert to utf-16
    std::vector<unsigned short> str16;
    utf8::unchecked::utf8to16(text.begin(), text.end(), back_inserter(str16));

    size_t length = str16.size();

    for (size_t i = 0; i < length; ++i) {
        int index = font->fontForScale(currentBackingStoreScale)->charIndex(str16[i]);
        width += font->fontForScale(currentBackingStoreScale)->glyphWidth(index);
    }

    return width;
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::drawCenteredText(MultiDPIFont* font, Rect rect, Color color, const std::string& text) {
    float textWidth = getTextWidth(font, text);
    float cx = rect.origin.x + rect.size.width / 2.0f;
    float cy = rect.origin.y + rect.size.height / 2.0f;
    drawText(font,
             makePoint(cx - textWidth / 2.0f, cy - font->fontForScale(currentBackingStoreScale)->height() / 2.0f +
                                                  -font->fontForScale(currentBackingStoreScale)->descender()),
             color, text, 0.0f);
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::drawLeftText(MultiDPIFont* font, Rect rect, Color color, const std::string& text) {
    float cy = rect.origin.y + rect.size.height / 2.0f;
    drawText(font,
             makePoint(rect.origin.x, cy - font->fontForScale(currentBackingStoreScale)->height() / 2.0f +
                                          -font->fontForScale(currentBackingStoreScale)->descender()),
             color, text, 0.0f);
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::drawRightText(MultiDPIFont* font, Rect rect, Color color, const std::string& text) {
    float textWidth = getTextWidth(font, text);
    float cy = rect.origin.y + rect.size.height / 2.0f;
    drawText(font,
             makePoint(rect.origin.x + rect.size.width - textWidth,
                       cy - font->fontForScale(currentBackingStoreScale)->height() / 2.0f +
                           -font->fontForScale(currentBackingStoreScale)->descender()),
             color, text, 0.0f);
}

// ---------------------------------------------------------------------------------------------------------------------
float MDStudio::fontHeight(MultiDPIFont* font) { return font->fontForScale(currentBackingStoreScale)->height(); }

// ---------------------------------------------------------------------------------------------------------------------
inline void drawPlainRect(MDStudio::Rect rect) {
    glEnableClientState(GL_VERTEX_ARRAY);

    float vertices[] = {rect.origin.x,
                        rect.origin.y,  // bottom left corner
                        rect.origin.x + rect.size.width,
                        rect.origin.y,  // bottom right corner
                        rect.origin.x + rect.size.width,
                        rect.origin.y + rect.size.height,  // top right corner
                        rect.origin.x,
                        rect.origin.y + rect.size.height};  // top left corner

    GLubyte indices[] = {0, 1, 2, 0, 2, 3};

    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);

    glDisableClientState(GL_VERTEX_ARRAY);
}

// ---------------------------------------------------------------------------------------------------------------------
// Note: Only horizontal or vertical for now
inline void drawHVLine(MDStudio::Point p1, MDStudio::Point p2, Color color, float width) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    width = width / 2.0f;

    glColor4f(color.red, color.green, color.blue, color.alpha);

    if (p1.y == p2.y) {
        // Horizontal
        drawPlainRect(makeRect(p1.x, p1.y - width, p2.x - p1.x, 2.0f * width));
    } else if (p1.x == p2.x) {
        // Vertical
        drawPlainRect(makeRect(p1.x - width, p1.y, 2.0f * width, p2.y - p1.y));
    }

    glDisable(GL_BLEND);
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::drawRect(Rect rect, Color color, float lineWidth) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(color.red, color.green, color.blue, color.alpha);

    glEnableClientState(GL_VERTEX_ARRAY);

    float vertices[] = {rect.origin.x,
                        rect.origin.y,
                        rect.origin.x,
                        rect.origin.y + rect.size.height,
                        rect.origin.x + rect.size.width,
                        rect.origin.y + rect.size.height,
                        rect.origin.x + rect.size.width,
                        rect.origin.y,
                        rect.origin.x + lineWidth,
                        rect.origin.y + lineWidth,
                        rect.origin.x + lineWidth,
                        rect.origin.y + rect.size.height - lineWidth,
                        rect.origin.x + rect.size.width - lineWidth,
                        rect.origin.y + rect.size.height - lineWidth,
                        rect.origin.x + rect.size.width - lineWidth,
                        rect.origin.y + lineWidth};

    GLubyte indices[] = {0, 4, 1, 5, 2, 6, 3, 7, 0, 4};

    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glDrawElements(GL_TRIANGLE_STRIP, 10, GL_UNSIGNED_BYTE, indices);

    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable(GL_BLEND);
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::drawFilledRect(Rect rect, Color fillColor) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(fillColor.red, fillColor.green, fillColor.blue, fillColor.alpha);
    drawPlainRect(rect);

    glDisable(GL_BLEND);
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::drawFilledRectGradientV(Rect rect, Color fillColorBottom, Color fillColorTop) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glShadeModel(GL_SMOOTH);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    float vertices[] = {rect.origin.x,
                        rect.origin.y,  // bottom left corner
                        rect.origin.x + rect.size.width,
                        rect.origin.y,  // bottom right corner
                        rect.origin.x + rect.size.width,
                        rect.origin.y + rect.size.height,  // top right corner
                        rect.origin.x,
                        rect.origin.y + rect.size.height};  // top left corner

    float colors[] = {fillColorBottom.red, fillColorBottom.green, fillColorBottom.blue, fillColorBottom.alpha,
                      fillColorBottom.red, fillColorBottom.green, fillColorBottom.blue, fillColorBottom.alpha,
                      fillColorTop.red,    fillColorTop.green,    fillColorTop.blue,    fillColorTop.alpha,
                      fillColorTop.red,    fillColorTop.green,    fillColorTop.blue,    fillColorTop.alpha};

    GLubyte indices[] = {0, 1, 2, 0, 2, 3};

    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glColorPointer(4, GL_FLOAT, 0, colors);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glShadeModel(GL_FLAT);

    glDisable(GL_BLEND);
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::drawRoundRect(Rect rect, float radius, Color color, float lineWidth) {
    float lineWidthHalf = lineWidth / 2.0f;

    // Bottom-left
    drawArc(makePoint(rect.origin.x + radius, rect.origin.y + radius), radius - lineWidthHalf, M_PI, M_PI / 2.0, color,
            lineWidth);

    // Bottom-right
    drawArc(makePoint(rect.origin.x + rect.size.width - radius, rect.origin.y + radius), radius - lineWidthHalf,
            3.0 * M_PI / 2.0, M_PI / 2.0, color, lineWidth);

    // Top-left
    drawArc(makePoint(rect.origin.x + radius, rect.origin.y + rect.size.height - radius), radius - lineWidthHalf,
            M_PI / 2.0, M_PI / 2.0, color, lineWidth);

    // Top-right
    drawArc(makePoint(rect.origin.x + rect.size.width - radius, rect.origin.y + rect.size.height - radius),
            radius - lineWidthHalf, 0.0, M_PI / 2.0, color, lineWidth);

    // Left
    drawLine(makePoint(rect.origin.x + lineWidthHalf, rect.origin.y + radius),
             makePoint(rect.origin.x + lineWidthHalf, rect.origin.y + rect.size.height - radius), color, lineWidth);

    // Top
    drawLine(makePoint(rect.origin.x + radius, rect.origin.y + rect.size.height - lineWidthHalf),
             makePoint(rect.origin.x + rect.size.width - radius, rect.origin.y + rect.size.height - lineWidthHalf),
             color, lineWidth);

    // Right
    drawLine(makePoint(rect.origin.x + rect.size.width - lineWidthHalf, rect.origin.y + radius),
             makePoint(rect.origin.x + rect.size.width - lineWidthHalf, rect.origin.y + rect.size.height - radius),
             color, lineWidth);

    // Bottom
    drawLine(makePoint(rect.origin.x + radius, rect.origin.y + lineWidthHalf),
             makePoint(rect.origin.x + rect.size.width - radius, rect.origin.y + lineWidthHalf), color, lineWidth);
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::drawFilledRoundRect(Rect rect, float radius, Color fillColor) {
    float lineWidth = 1.0f / 2.0f;

    // rect.origin.x = floor(rect.origin.x);
    // rect.origin.y = floor(rect.origin.y);
    // rect.size.width = floor(rect.size.width);
    // rect.size.height = floor(rect.size.height);

    // Bottom-left
    drawFilledArc(makePoint(rect.origin.x + radius, rect.origin.y + radius), radius - lineWidth, M_PI, M_PI / 2.0,
                  fillColor);

    // Bottom-right
    drawFilledArc(makePoint(rect.origin.x + rect.size.width - radius, rect.origin.y + radius), radius - lineWidth,
                  3.0 * M_PI / 2.0, M_PI / 2.0, fillColor);

    // Top-left
    drawFilledArc(makePoint(rect.origin.x + radius, rect.origin.y + rect.size.height - radius), radius - lineWidth,
                  M_PI / 2.0, M_PI / 2.0, fillColor);

    // Top-right
    drawFilledArc(makePoint(rect.origin.x + rect.size.width - radius, rect.origin.y + rect.size.height - radius),
                  radius - lineWidth, 0.0, M_PI / 2.0, fillColor);

    // Left
    drawFilledRect(makeRect(rect.origin.x, rect.origin.y + radius, radius, rect.size.height - 2.0f * radius),
                   fillColor);

    // Top
    drawFilledRect(makeRect(rect.origin.x + radius, rect.origin.y + rect.size.height - radius,
                            rect.size.width - 2.0f * radius, radius),
                   fillColor);

    // Right
    drawFilledRect(makeRect(rect.origin.x + rect.size.width - radius, rect.origin.y + radius, radius,
                            rect.size.height - 2.0f * radius),
                   fillColor);

    // Bottom
    drawFilledRect(makeRect(rect.origin.x + radius, rect.origin.y, rect.size.width - 2.0f * radius, radius), fillColor);

    // Center
    drawFilledRect(makeRect(rect.origin.x + radius, rect.origin.y + radius, rect.size.width - 2.0f * radius,
                            rect.size.height - 2.0f * radius),
                   fillColor);
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::drawSegment(Point p0, Point p1, Point p2, Point p3, Color color, float thickness) {
    auto v0 = Vector2(p0.x, p0.y);
    auto v1 = Vector2(p1.x, p1.y);
    auto v2 = Vector2(p2.x, p2.y);
    auto v3 = Vector2(p3.x, p3.y);

    thickness /= 2.0f;

    // Skip if zero length
    if (v1 == v2) return;

    // Define the line between the two points
    Vector2 line = (v2 - v1).normalized();

    // Find the normal vector of this line
    Vector2 normal = Vector2(-line.y(), line.x()).normalized();

    // Find the tangent vector at both the end points:
    //  - if there are no segments before or after this one, use the line itself
    //  - otherwise, add the two normalized lines and average them by normalizing again
    Vector2 tangent1 = (v0 == v1) ? line : ((v1 - v0).normalized() + line).normalized();
    Vector2 tangent2 = (v2 == v3) ? line : ((v3 - v2).normalized() + line).normalized();

    // Find the miter line, which is the normal of the tangent
    Vector2 miter1 = Vector2(-tangent1.y(), tangent1.x());
    Vector2 miter2 = Vector2(-tangent2.y(), tangent2.x());

    // Find length of miter by projecting the miter onto the normal,
    // take the length of the projection, invert it and multiply it by the thickness:
    //   length = thickness * ( 1 / |normal|.|miter| )
    float length1 = thickness / normal.dot(miter1);
    float length2 = thickness / normal.dot(miter2);

    glEnable(GL_BLEND);
    glColor4f(color.red, color.green, color.blue, color.alpha);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto pp0 = makePoint(v1 - length1 * miter1);
    auto pp1 = makePoint(v2 - length2 * miter2);
    auto pp2 = makePoint(v1 + length1 * miter1);
    auto pp3 = makePoint(v2 + length2 * miter2);

    const float featherThickness = 0.5f;
    length1 += featherThickness;
    length2 += featherThickness;

    auto ppf0 = makePoint(v1 - length1 * miter1);
    auto ppf1 = makePoint(v2 - length2 * miter2);
    auto ppf2 = makePoint(v1 + length1 * miter1);
    auto ppf3 = makePoint(v2 + length2 * miter2);

    glEnableClientState(GL_VERTEX_ARRAY);

    GLubyte indices[] = {0, 2, 3, 0, 3, 1};

    //
    // Draw line
    //

    if (thickness > 0.0f) {
        float vertices[] = {pp0.x, pp0.y, pp1.x, pp1.y, pp2.x, pp2.y, pp3.x, pp3.y};

        glVertexPointer(2, GL_FLOAT, 0, vertices);

        glColor4f(color.red, color.green, color.blue, color.alpha);
        glDrawElements(GL_TRIANGLES, (GLsizei)6, GL_UNSIGNED_BYTE, indices);
    }

    //
    // Draw feather
    //

    glShadeModel(GL_SMOOTH);
    glEnableClientState(GL_COLOR_ARRAY);

    // Bottom
    float verticesFeatherBottom[] = {ppf0.x, ppf0.y, ppf1.x, ppf1.y, pp0.x, pp0.y, pp1.x, pp1.y};
    float colorsFeatherBottom[] = {color.red,  color.green, color.blue, 0.0f,        color.red,  color.green,
                                   color.blue, 0.0f,        color.red,  color.green, color.blue, color.alpha,
                                   color.red,  color.green, color.blue, color.alpha};
    glVertexPointer(2, GL_FLOAT, 0, verticesFeatherBottom);
    glColorPointer(4, GL_FLOAT, 0, colorsFeatherBottom);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);

    // Top
    float verticesFeatherTop[] = {pp2.x, pp2.y, pp3.x, pp3.y, ppf2.x, ppf2.y, ppf3.x, ppf3.y};
    float colorsFeatherTop[] = {color.red,  color.green, color.blue, color.alpha, color.red,  color.green,
                                color.blue, color.alpha, color.red,  color.green, color.blue, 0.0f,
                                color.red,  color.green, color.blue, 0.0f};
    glVertexPointer(2, GL_FLOAT, 0, verticesFeatherTop);
    glColorPointer(4, GL_FLOAT, 0, colorsFeatherTop);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);

    glDisableClientState(GL_COLOR_ARRAY);
    glShadeModel(GL_FLAT);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_BLEND);

    /*
     drawFilledArc(pp0, 5.0f, 0, 2 * M_PI, redColor);
     drawFilledArc(pp1, 5.0f, 0, 2 * M_PI, greenColor);
     drawFilledArc(pp2, 5.0f, 0, 2 * M_PI, blueColor);
     drawFilledArc(pp3, 5.0f, 0, 2 * M_PI, yellowColor);
    */
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::drawLine(Point p1, Point p2, Color color, float thickness) {
    drawSegment(p1, p1, p2, p2, color, thickness);
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::drawArc(MDStudio::Point pt, float radius, double startAngle, double arcAngle, Color color,
                       float lineWidth) {
    drawArc2(pt, radius, radius, startAngle, arcAngle, color, lineWidth);
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::drawArc2(MDStudio::Point pt, float radiusX, float radiusY, double startAngle, double arcAngle,
                        Color color, float lineWidth) {
    int nbSteps = static_cast<int>(arcAngle * std::max(radiusX, radiusY));
    double arcAngleStep = arcAngle / (double)(nbSteps - 1);

    std::deque<Point> pts;
    for (int i = 0; i < nbSteps + 3; ++i) {
        double angle = startAngle - arcAngleStep + arcAngleStep * (double)i;

        pts.push_back(makePoint(pt.x + radiusX * cos(angle), pt.y + radiusY * sin(angle)));

        if (i > 3) {
            drawSegment(pts[0], pts[1], pts[2], pts[3], color, lineWidth);
            pts.pop_front();
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::drawFilledArc(MDStudio::Point pt, float radius, double startAngle, double arcAngle, Color fillColor) {
    drawFilledArc2(pt, radius, radius, startAngle, arcAngle, fillColor);
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::drawFilledArc2(MDStudio::Point pt, float radiusX, float radiusY, double startAngle, double arcAngle,
                              Color fillColor) {
    int nbSteps = static_cast<int>(arcAngle * std::max(radiusX, radiusY));
    double arcAngleStep = arcAngle / (double)(nbSteps - 1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(fillColor.red, fillColor.green, fillColor.blue, fillColor.alpha);

    std::vector<float> vertices;

    vertices.push_back(pt.x);
    vertices.push_back(pt.y);

    for (int i = 0; i < nbSteps; ++i) {
        double angle = startAngle + arcAngleStep * (double)i;
        float x = pt.x + radiusX * cos(angle);
        float y = pt.y + radiusY * sin(angle);

        vertices.push_back(x);
        vertices.push_back(y);
    }

    std::vector<GLushort> indices;

    size_t nbIndices = vertices.size() / 2;
    for (size_t i = 0; i < nbIndices; ++i) indices.push_back((GLushort)i);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, &vertices[0]);
    glDrawElements(GL_TRIANGLE_FAN, (GLsizei)nbIndices, GL_UNSIGNED_SHORT, &indices[0]);
    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable(GL_BLEND);
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::drawTriangle(Point p0, Point p1, Point p2, Color color, float thickness) {
    drawSegment(p0, p1, p2, p0, color, thickness);
    drawSegment(p1, p2, p0, p1, color, thickness);
    drawSegment(p2, p0, p1, p2, color, thickness);
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::drawFilledTriangle(Point p0, Point p1, Point p2, Color fillColor) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::vector<float> vertices;

    vertices.push_back(p0.x);
    vertices.push_back(p0.y);
    vertices.push_back(p1.x);
    vertices.push_back(p1.y);
    vertices.push_back(p2.x);
    vertices.push_back(p2.y);

    std::vector<GLubyte> indices;

    size_t nbIndices = vertices.size() / 2;
    for (size_t i = 0; i < nbIndices; ++i) indices.push_back(i);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, &vertices[0]);

    glColor4f(fillColor.red, fillColor.green, fillColor.blue, fillColor.alpha);
    glDrawElements(GL_TRIANGLES, (GLsizei)nbIndices, GL_UNSIGNED_BYTE, &indices[0]);

    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable(GL_BLEND);
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::setViewport(Rect rect) { glViewport(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height); }

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::setScissor(Rect rect) {
    // assert(floorf(rect.size.width) == rect.size.width);
    // assert(floorf(rect.size.height) == rect.size.height);
    // assert(floorf(rect.origin.x) == rect.origin.x);
    // assert(floorf(rect.origin.y) == rect.origin.y);

    if (rect.size.width < 0 || rect.size.height < 0) {
        glDisable(GL_SCISSOR_TEST);
    } else {
        glEnable(GL_SCISSOR_TEST);
        glScissor(rect.origin.x * currentBackingStoreScale, rect.origin.y * currentBackingStoreScale,
                  rect.size.width * currentBackingStoreScale, rect.size.height * currentBackingStoreScale);
    }
    currentScissor = rect;
}

// ---------------------------------------------------------------------------------------------------------------------
MDStudio::Rect MDStudio::scissor() { return currentScissor; }

// ---------------------------------------------------------------------------------------------------------------------
MDStudio::Point MDStudio::translation() { return currentTranslation; }

// ---------------------------------------------------------------------------------------------------------------------
float MDStudio::rotation() { return currentRotation; }

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::setTransform(Point translationPoint, float rotationAngle) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    currentTranslation.x = translationPoint.x;
    currentTranslation.y = translationPoint.y;
    currentRotation = rotationAngle;
    glTranslatef(translationPoint.x, translationPoint.y, 0.0f);
    glRotatef(rotationAngle, 0, 0, 1);
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::drawImage(Rect rect, Image* image, Color color) {
    glEnable(GL_BLEND);
    glColor4f(color.red, color.green, color.blue, color.alpha);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    image->bindTexture();

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_FLAT);

    float w = (float)image->internalSize().width / (float)image->internalTextureSize().width;
    float h = (float)image->internalSize().height / (float)image->internalTextureSize().height;

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    float vertices[] = {rect.origin.x,
                        rect.origin.y,  // bottom left corner
                        rect.origin.x + rect.size.width,
                        rect.origin.y,  // bottom right corner
                        rect.origin.x + rect.size.width,
                        rect.origin.y + rect.size.height,  // top right corner
                        rect.origin.x,
                        rect.origin.y + rect.size.height};  // top left corner

    float texCoords[] = {0.0f, 0.0f, w, 0.0f, w, h, 0.0f, h};

    GLubyte indices[] = {0, 1, 2, 0, 2, 3};

    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::drawImage(Point pt, Image* image, Color color) {
    drawImage(makeRect(pt.x, pt.y, image->size().width, image->size().height), image, color);
}

// ---------------------------------------------------------------------------------------------------------------------
bool MDStudio::isPointInRect(Point pt, Rect rect) {
    normalizeRect(&rect);
    return ((pt.x >= rect.origin.x) && (pt.x <= rect.origin.x + rect.size.width) && (pt.y >= rect.origin.y) &&
            (pt.y <= rect.origin.y + rect.size.height));
}

// ---------------------------------------------------------------------------------------------------------------------
bool MDStudio::isRectInRect(Rect rect1, Rect rect2) {
    normalizeRect(&rect1);
    normalizeRect(&rect2);

    float rect1X1 = rect1.origin.x;
    float rect2X2 = rect2.origin.x + rect2.size.width;
    float rect1X2 = rect1.origin.x + rect1.size.width;
    float rect2X1 = rect2.origin.x;
    float rect1Y1 = rect1.origin.y;
    float rect2Y2 = rect2.origin.y + rect2.size.height;
    float rect1Y2 = rect1.origin.y + rect1.size.height;
    float rect2Y1 = rect2.origin.y;
    return (rect1X1 < rect2X2 && rect1X2 > rect2X1 && rect1Y1 < rect2Y2 && rect1Y2 > rect2Y1);
}

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::setBackingStoreScale(float scale) { currentBackingStoreScale = scale; }

// ---------------------------------------------------------------------------------------------------------------------
float MDStudio::backingStoreScale() { return currentBackingStoreScale; }

// ---------------------------------------------------------------------------------------------------------------------
void MDStudio::clear() {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
}

// ---------------------------------------------------------------------------------------------------------------------
double MDStudio::angleBetweenPoints(Point p1, Point p2) {
    auto b = p2.y - p1.y;
    auto a = p2.x - p1.x;

    if (a == 0.0f) {
        return b > 0.0f ? M_PI / 2.0 : 3.0 * 2.0 * M_PI / 4.0f;
    } else if (b == 0.0f) {
        return a > 0.0f ? 0.0f : M_PI;
    } else {
        double angle = atan(abs(b) / abs(a));

        if (a < 0 && b > 0) {
            angle = M_PI - angle;
        } else if (a < 0 && b < 0) {
            angle = M_PI + angle;
        } else if (a > 0 && b < 0) {
            angle = 2 * M_PI - angle;
        }
        return angle;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
float MDStudio::distanceBetweenPoints(Point p1, Point p2) { return sqrtf(sqrf(p2.x - p1.x) + sqrf(p2.y - p1.y)); }
