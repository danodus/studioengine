//
//  textview.h
//  MDStudio-SDK
//
//  Created by Daniel Cliche on 2016-06-29.
//  Copyright © 2016-2020 Daniel Cliche. All rights reserved.
//

#ifndef TEXTVIEW_H
#define TEXTVIEW_H

#include <undomanager.h>
#include <ustring.h>

#include "control.h"
#include "font.h"

namespace MDStudio {

class TextView : public Control {
   public:
    typedef std::function<void(TextView* sender, std::string text)> textChangingFnType;
    typedef std::function<void(TextView* sender, std::string text)> textDidChangeFnType;
    typedef std::function<void(TextView* sender, unsigned int col, unsigned int line)> cursorPositionDidChangeFnType;
    typedef std::function<void(TextView* sender)> mouseSelectionDidBeginFnType;
    typedef std::function<void(TextView* sender)> mouseSelectionDidEndFnType;

   private:
    UndoManager* _undoManager = nullptr;
    MultiDPIFont* _font;
    std::vector<UString> _lines;
    unsigned int _cursorPosCol, _cursorPosLine;
    unsigned int _selCursorPosCol, _selCursorPosLine;
    unsigned int _selBeginPosCol, _selBeginPosLine;
    unsigned int _selEndPosCol, _selEndPosLine;
    bool _isEnabled;
    bool _hasFocus;
    bool _isCaptured;
    bool _isCursorVisible;
    bool _isSingleLine;
    bool _isSelecting;

    textChangingFnType _textChangingFn;
    textDidChangeFnType _textDidChangeFn;
    cursorPositionDidChangeFnType _cursorPositionDidChangeFn;
    mouseSelectionDidBeginFnType _mouseSelectionDidBeginFn;
    mouseSelectionDidEndFnType _mouseSelectionDidEndFn;

    void blinkCursor();
    Point getPointAtPosColLine(unsigned int posCol, unsigned int posLine);
    void getCursorPosition(Point pt, unsigned int* cursorPosLine, unsigned* cursorPosCol);
    void clampSelection();
    bool isTextSelected();
    UString selectedText();
    void clearSelectedText();
    unsigned int indexFromPosColLine(unsigned int posCol, unsigned int posLine);
    void setTextInternal(std::string text);
    void setSelection(unsigned int selBeginPosCol, unsigned int selBeginPosLine, unsigned int selEndPosCol,
                      unsigned int selEndPosLine);
    void clearSelection();

    bool isCursorAtBeginning();
    bool isCursorAtEnd();
    bool isCursorAtBeginningOfLine();
    bool isCursorAtEndOfLine();
    void moveCursorForward(int nbSteps);
    void moveCursorBackward(int nbSteps);

    int addCharacters(int cursorPosCol, int cursorPosLine, const std::string& characters);
    int deleteCharacters(int cursorPosCol, int cursorPosLine, const std::string& characters);

    void getNormalizedSelectionPos(unsigned int* selBeginPosCol, unsigned int* selBeginPosLine,
                                   unsigned int* selEndPosCol, unsigned int* selEndPosLine);

    void selectAll() override;

    void handleMouse(const MDStudio::UIEvent* event, const MDStudio::Rect& rect);
    void handleCut();
    void handleCopy();
    void handlePaste();
    void handleMouseUp(const MDStudio::UIEvent* event);
    void handleMouseMoved(const MDStudio::UIEvent* event);
    void handleKeyBackspace();
    void handleKeyDelete();
    void handleKeyLeft(const MDStudio::UIEvent* event);
    void handleKeyRight(const MDStudio::UIEvent* event);
    void handleKeyUp(const MDStudio::UIEvent* event);
    void handleKeyDown(const MDStudio::UIEvent* event);
    void handleKeyEnter();
    void handleKeyHome(const MDStudio::UIEvent* event);
    void handleKeyEnd(const MDStudio::UIEvent* event);
    void handleKeyTab(const MDStudio::UIEvent* event);

   protected:
    void draw() override;

   public:
    TextView(const std::string& name, void* owner, bool isSingleLine = false);
    ~TextView();

    void setUndoManager(UndoManager* undoManager) { _undoManager = undoManager; }

    bool handleEvent(const UIEvent* event) override;
    void resignFirstResponder() override;

    bool hasFocus() { return _hasFocus; }

    void setIsEnabled(bool isEnabled) { _isEnabled = isEnabled; }
    bool isEnabled() { return _isEnabled; }

    void setFont(MultiDPIFont* font) { _font = font; }
    MultiDPIFont* font() { return _font; }

    void setText(std::string text, bool isDelegateNotified = true);
    std::string text();
    std::vector<unsigned int> lineLengths();
    void setCursorPos(unsigned int posCol, unsigned int posLine, bool isDelegateNotified = true,
                      bool isUndoable = false);

    void selectAllText();

    void startEdition();
    void stopEdition();

    Size contentSize();
    Rect cursorRect();
    Rect selectionCursorRect();

    void setTextChangingFn(textChangingFnType textChangingFn) { _textChangingFn = textChangingFn; }
    void setTextDidChangeFn(textDidChangeFnType textDidChangeFn) { _textDidChangeFn = textDidChangeFn; }
    void setCursorPositionDidChangeFn(cursorPositionDidChangeFnType cursorPositionDidChangeFn) {
        _cursorPositionDidChangeFn = cursorPositionDidChangeFn;
    }
    void setMouseSelectionDidBeginFn(mouseSelectionDidBeginFnType mouseSelectionDidBeginFn) {
        _mouseSelectionDidBeginFn = mouseSelectionDidBeginFn;
    }
    void setMouseSelectionDidEndFn(mouseSelectionDidEndFnType mouseSelectionDidEndFn) {
        _mouseSelectionDidEndFn = mouseSelectionDidEndFn;
    }
};

}  // namespace MDStudio

#endif  // TEXTVIEW_H
