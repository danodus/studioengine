//
//  textview.cpp
//  MDStudio-SDK
//
//  Created by Daniel Cliche on 2016-06-29.
//  Copyright (c) 2016-2021 Daniel Cliche. All rights reserved.
//

#include "textview.h"

#include <assert.h>

#include "../platform.h"
#include "draw.h"
#include "pasteboard.h"
#include "responderchain.h"

#define TEXT_VIEW_CURSOR_BLINK_PERIOD 0.5  // Cursor blink period in seconds

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
TextView::TextView(const std::string& name, void* owner, bool isSingleLine)
    : Control(name, owner), _isSingleLine(isSingleLine) {
    _textChangingFn = nullptr;
    _textDidChangeFn = nullptr;
    _cursorPositionDidChangeFn = nullptr;
    _mouseSelectionDidBeginFn = nullptr;
    _mouseSelectionDidEndFn = nullptr;
    _isEnabled = true;
    _font = SystemFonts::sharedInstance()->semiboldFont();
    _hasFocus = false;
    _isCaptured = false;
    _cursorPosCol = 0;
    _cursorPosLine = 0;
    _selCursorPosCol = 0;
    _selCursorPosLine = 0;
    _selBeginPosCol = 0;
    _selBeginPosLine = 0;
    _selEndPosCol = 0;
    _selEndPosLine = 0;
    _isSelecting = false;

    _lines.push_back(UString(""));
}

// ---------------------------------------------------------------------------------------------------------------------
TextView::~TextView() {
    if (_hasFocus) Platform::sharedInstance()->cancelDelayedInvokes(this);
}

// ---------------------------------------------------------------------------------------------------------------------
Point TextView::getPointAtPosColLine(unsigned int posCol, unsigned int posLine) {
    float y = bounds().size.height - (posLine + 1) * fontHeight(_font);

    std::string substr = _lines[posLine].substr(0, posCol).string();
    float textWidth = getTextWidth(_font, substr);

    return makePoint(textWidth, y);
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::draw() {
    DrawContext* dc = drawContext();

    if (_hasFocus) {
        dc->pushStates();
        dc->setFillColor(veryDimGrayColor);
        dc->drawRect(bounds());
        dc->popStates();
    }

    if (isTextSelected()) {
        //
        // Draw text selection
        //

        // Get the normalized selection positions
        unsigned int selBeginPosCol = _selBeginPosCol, selBeginPosLine = _selBeginPosLine;
        unsigned int selEndPosCol = _selEndPosCol, selEndPosLine = _selEndPosLine;

        if (selBeginPosLine > selEndPosLine) {
            unsigned int t = selBeginPosLine;
            selBeginPosLine = selEndPosLine;
            selEndPosLine = t;

            t = selBeginPosCol;
            selBeginPosCol = selEndPosCol;
            selEndPosCol = t;
        }

        if ((selBeginPosCol > selEndPosCol) && (selBeginPosLine == selEndPosLine)) {
            unsigned int t = selBeginPosCol;
            selBeginPosCol = selEndPosCol;
            selEndPosCol = t;
        }

        // Draw selection
        for (unsigned int posLine = selBeginPosLine; posLine <= selEndPosLine; ++posLine) {
            Point ptBegin, ptEnd;

            // If it is the only line
            if (posLine == selBeginPosLine && posLine == selEndPosLine) {
                ptBegin = getPointAtPosColLine(selBeginPosCol, posLine);
                ptEnd = getPointAtPosColLine(selEndPosCol, posLine);
            } else if (posLine == selBeginPosLine) {
                // This is the first line
                ptBegin = getPointAtPosColLine(selBeginPosCol, posLine);
                ptEnd = getPointAtPosColLine((unsigned int)_lines[posLine].length(), posLine);
                ptEnd.x = rect().size.width;
            } else if (posLine == selEndPosLine) {
                // This is the last line
                ptBegin = getPointAtPosColLine(0, posLine);
                ptEnd = getPointAtPosColLine(selEndPosCol, posLine);
            } else {
                // Between first and last line
                ptBegin = getPointAtPosColLine(0, posLine);
                ptEnd = getPointAtPosColLine((unsigned int)_lines[posLine].length(), posLine);
                ptBegin.x = 0.0f;
                ptEnd.x = rect().size.width;
            }

            dc->pushStates();
            dc->setFillColor(_hasFocus ? blueColor : makeColor(0.0f, 0.0f, 0.5f, 1.0f));
            dc->drawRect(makeRect(ptBegin.x, ptBegin.y, ptEnd.x - ptBegin.x, ptBegin.y - ptEnd.y + fontHeight(_font)));
            dc->popStates();
        }  // for each selected line
    }      // if text selected

    //
    // Draw text
    //

    dc->pushStates();
    dc->setStrokeColor(whiteColor);

    if (_isSingleLine) {
        if (_lines.size() > 0) {
            dc->drawLeftText(_font, bounds(), _lines[0].string());
        }
    } else {
        int lineNum = 0;
        float y = bounds().size.height - fontHeight(_font);
        auto w = makeRect(-offset().x, -offset().y, clippedRect().size.width, clippedRect().size.height);
        for (auto line : _lines) {
            auto textWidth = getTextWidth(_font, line.string());
            if (isRectInRect(makeRect(0.0f, y, textWidth, fontHeight(_font)), w))
                dc->drawText(_font, makePoint(0.0f, y), line.string());
            ++lineNum;
            y -= fontHeight(_font);
        }  // for each line
    }

    dc->popStates();

    //
    // Draw cursor
    //

    if (_hasFocus && _isCursorVisible && (_lines.size() > 0) && (!_isSelecting || !isTextSelected())) {
        Point pt = getPointAtPosColLine(_cursorPosCol, _cursorPosLine);
        Rect r = makeRect(pt.x, pt.y, 1.0f, fontHeight(_font));
        dc->pushStates();
        dc->setStrokeColor(whiteColor);
        dc->drawRect(r);
        dc->popStates();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::blinkCursor() {
    _isCursorVisible = !_isCursorVisible;
    setDirty();
    Platform::sharedInstance()->invokeDelayed(
        this, [=] { blinkCursor(); }, TEXT_VIEW_CURSOR_BLINK_PERIOD);
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::getCursorPosition(Point pt, unsigned int* cursorPosLine, unsigned* cursorPosCol) {
    *cursorPosLine = (View::rect().origin.y + View::rect().size.height + resolvedOffset().y - pt.y) / fontHeight(_font);

    if (*cursorPosLine >= _lines.size()) *cursorPosLine = (int)(_lines.size() - 1);

    float clickPos = pt.x - View::rect().origin.x - resolvedOffset().x;
    *cursorPosCol = 0;
    float textWidth, lastCharWidth;

    UString line = _lines[*cursorPosLine];
    while (*cursorPosCol <= line.length()) {
        UString substr = line.substr(0, *cursorPosCol);
        std::string lastCharSubstr;
        if (!substr.isEmpty()) lastCharSubstr = substr[substr.length() - 1];
        textWidth = getTextWidth(_font, substr.string());
        lastCharWidth = getTextWidth(_font, lastCharSubstr);
        if (textWidth - lastCharWidth / 2.0f >= clickPos) break;
        (*cursorPosCol)++;
    }
    if (*cursorPosCol > 0) (*cursorPosCol)--;
}

// ---------------------------------------------------------------------------------------------------------------------
bool TextView::isCursorAtBeginning() { return _cursorPosCol == 0 && _cursorPosLine == 0; }

// ---------------------------------------------------------------------------------------------------------------------
bool TextView::isCursorAtEnd() {
    return _cursorPosLine == (_lines.size() - 1) && _cursorPosCol == _lines[_cursorPosLine].length();
}

// ---------------------------------------------------------------------------------------------------------------------
bool TextView::isCursorAtBeginningOfLine() { return _cursorPosCol == 0; }

// ---------------------------------------------------------------------------------------------------------------------
bool TextView::isCursorAtEndOfLine() { return _cursorPosCol == _lines[_cursorPosLine].length(); }

// ---------------------------------------------------------------------------------------------------------------------
void TextView::moveCursorForward(int nbSteps) {
    unsigned int cursorPosCol = _cursorPosCol, cursorPosLine = _cursorPosLine;
    for (; nbSteps > 0; --nbSteps) {
        if (cursorPosCol < _lines[cursorPosLine].length()) {
            cursorPosCol++;
        } else if (cursorPosLine < (_lines.size() - 1)) {
            ++cursorPosLine;
            cursorPosCol = 0;
        } else {
            // At the end
            break;
        }
    }
    setCursorPos(cursorPosCol, cursorPosLine, true, true);
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::moveCursorBackward(int nbSteps) {
    unsigned int cursorPosCol = _cursorPosCol, cursorPosLine = _cursorPosLine;
    for (; nbSteps > 0; --nbSteps) {
        if (cursorPosCol > 0) {
            --cursorPosCol;
        } else if (cursorPosLine > 0) {
            --cursorPosLine;
            cursorPosCol = (int)_lines[cursorPosLine].length();
        } else {
            // At the beginning
            break;
        }
    }
    setCursorPos(cursorPosCol, cursorPosLine, true, true);
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::handleCut() {
    if (isTextSelected()) {
        UString text = selectedText();
        clearSelectedText();
        Pasteboard::sharedInstance()->setContent(text.string());
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::handleMouse(const MDStudio::UIEvent* event, const MDStudio::Rect& rect) {
    if (!_hasFocus) {
        if (_isEnabled) {
            startEdition();
            responderChain()->setCursorInRect(this, Platform::IBeamCursor, rect);
        }
    } else {
        _isSelecting = true;
        _isCaptured = true;

        // Get the cursor position
        unsigned int cursorPosLine, cursorPosCol;
        getCursorPosition(event->pt, &cursorPosLine, &cursorPosCol);

        _selCursorPosLine = cursorPosLine;
        _selCursorPosCol = cursorPosCol;

        if (_mouseSelectionDidBeginFn) _mouseSelectionDidBeginFn(this);

        if (event->modifierFlags & MODIFIER_FLAG_SHIFT) {
            setSelection(_selBeginPosCol, _selBeginPosLine, cursorPosCol, cursorPosLine);
        } else {
            _cursorPosCol = cursorPosCol;
            _cursorPosLine = cursorPosLine;
            _isCursorVisible = true;
            setDirty();

            // clear the selection
            clearSelection();

            if (_cursorPositionDidChangeFn) _cursorPositionDidChangeFn(this, _cursorPosCol, _cursorPosLine);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::handleCopy() {
    if (isTextSelected()) {
        UString text = selectedText();
        Pasteboard::sharedInstance()->setContent(text.string());
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::handlePaste() {
    if (Pasteboard::sharedInstance()->isContentAvailable() &&
        Pasteboard::sharedInstance()->content().is<std::string>()) {
        if (_undoManager) _undoManager->beginGroup();
        if (isTextSelected()) clearSelectedText();
        auto nbCharactersAdded = addCharacters(_cursorPosCol, _cursorPosLine, Pasteboard::sharedInstance()->content());
        moveCursorForward(nbCharactersAdded);
        if (_undoManager) _undoManager->endGroup();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::handleMouseUp(const MDStudio::UIEvent* event) {
    // If we are selecting, set the cursor position
    if (_isSelecting) {
        // Get the cursor position
        unsigned int cursorPosLine, cursorPosCol;
        getCursorPosition(event->pt, &cursorPosLine, &cursorPosCol);

        _cursorPosLine = cursorPosLine;
        _cursorPosCol = cursorPosCol;

        if (_cursorPositionDidChangeFn) _cursorPositionDidChangeFn(this, _cursorPosCol, _cursorPosLine);
    }

    _isSelecting = false;
    _isCaptured = false;

    if (_mouseSelectionDidEndFn) _mouseSelectionDidEndFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::handleMouseMoved(const MDStudio::UIEvent* event) {
    if (_isSelecting) {
        // Hide the cursor
        _isCursorVisible = false;

        // Get the cursor position
        getCursorPosition(event->pt, &_selCursorPosLine, &_selCursorPosCol);

        setSelection(_selBeginPosCol, _selBeginPosLine, _selCursorPosCol, _selCursorPosLine);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::handleKeyBackspace() {
    if (isTextSelected()) {
        clearSelectedText();
        if (_textChangingFn) _textChangingFn(this, text());
    } else {
        if (!isCursorAtBeginning()) {
            if (_undoManager) _undoManager->beginGroup();
            moveCursorBackward(1);
            deleteCharacters(_cursorPosCol, _cursorPosLine,
                             isCursorAtEndOfLine() ? "\n" : _lines[_cursorPosLine][_cursorPosCol]);
            if (_undoManager) _undoManager->endGroup();
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::handleKeyDelete() {
    if (isTextSelected()) {
        clearSelectedText();
        if (_textChangingFn) _textChangingFn(this, text());
    } else {
        if (!isCursorAtEnd()) {
            if (_undoManager) _undoManager->beginGroup();
            deleteCharacters(_cursorPosCol, _cursorPosLine,
                             isCursorAtEndOfLine() ? "\n" : _lines[_cursorPosLine][_cursorPosCol]);
            moveCursorBackward(0);
            if (_undoManager) _undoManager->endGroup();
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::handleKeyLeft(const MDStudio::UIEvent* event) {
    if ((Platform::sharedInstance()->operatingSystem() == "osx") && (event->modifierFlags & MODIFIER_FLAG_COMMAND)) {
        int cursorPosLine = _cursorPosLine, cursorPosCol = _cursorPosCol;
        cursorPosCol = 0;
        setCursorPos(cursorPosCol, cursorPosLine);

        // If the user is holding the shift key, update the selection
        if (event->modifierFlags & MODIFIER_FLAG_SHIFT) {
            setSelection(_selBeginPosCol, _selBeginPosLine, cursorPosCol, cursorPosLine);
        } else {
            clearSelection();
        }
    } else {
        if (_cursorPosCol > 0) {
            int cursorPosLine = _cursorPosLine, cursorPosCol = _cursorPosCol;
            cursorPosCol--;
            setCursorPos(cursorPosCol, cursorPosLine);
            // If the user is holding the shift key, update the selection
            if (event->modifierFlags & MODIFIER_FLAG_SHIFT) {
                setSelection(_selBeginPosCol, _selBeginPosLine, cursorPosCol, cursorPosLine);
            } else {
                clearSelection();
            }
        } else if (_cursorPosLine > 0) {
            int cursorPosLine = _cursorPosLine, cursorPosCol = _cursorPosCol;
            cursorPosLine--;
            cursorPosCol = (int)_lines[cursorPosLine].length();
            setCursorPos(cursorPosCol, cursorPosLine);
            // If the user is holding the shift key, update the selection
            if (event->modifierFlags & MODIFIER_FLAG_SHIFT) {
                setSelection(_selBeginPosCol, _selBeginPosLine, cursorPosCol, cursorPosLine);
            } else {
                clearSelection();
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::handleKeyRight(const MDStudio::UIEvent* event) {
    if ((Platform::sharedInstance()->operatingSystem() == "osx") && (event->modifierFlags & MODIFIER_FLAG_COMMAND)) {
        int cursorPosLine = _cursorPosLine, cursorPosCol = _cursorPosCol;
        cursorPosCol = (unsigned int)_lines[cursorPosLine].length();
        setCursorPos(cursorPosCol, cursorPosLine);
        // If the user is holding the shift key, update the selection
        if (event->modifierFlags & MODIFIER_FLAG_SHIFT) {
            setSelection(_selBeginPosCol, _selBeginPosLine, cursorPosCol, cursorPosLine);
        } else {
            clearSelection();
        }
    } else {
        if (_cursorPosCol < _lines[_cursorPosLine].length()) {
            int cursorPosLine = _cursorPosLine, cursorPosCol = _cursorPosCol;
            cursorPosCol++;
            setCursorPos(cursorPosCol, cursorPosLine);
            // If the user is holding the shift key, update the selection
            if (event->modifierFlags & MODIFIER_FLAG_SHIFT) {
                setSelection(_selBeginPosCol, _selBeginPosLine, cursorPosCol, cursorPosLine);
            } else {
                clearSelection();
            }

        } else if (_cursorPosLine < (_lines.size() - 1)) {
            int cursorPosLine = _cursorPosLine, cursorPosCol = _cursorPosCol;
            ++cursorPosLine;
            cursorPosCol = 0;
            setCursorPos(cursorPosCol, cursorPosLine);
            // If the user is holding the shift key, update the selection
            if (event->modifierFlags & MODIFIER_FLAG_SHIFT) {
                setSelection(_selBeginPosCol, _selBeginPosLine, cursorPosCol, cursorPosLine);
            } else {
                clearSelection();
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::handleKeyUp(const MDStudio::UIEvent* event) {
    if ((Platform::sharedInstance()->operatingSystem() == "osx") && (event->modifierFlags & MODIFIER_FLAG_COMMAND)) {
        int cursorPosLine = _cursorPosLine, cursorPosCol = _cursorPosCol;

        cursorPosLine = 0;
        cursorPosCol = 0;

        setCursorPos(cursorPosCol, cursorPosLine);

        // If the user is holding the shift key, update the selection
        if (event->modifierFlags & MODIFIER_FLAG_SHIFT) {
            setSelection(_selBeginPosCol, _selBeginPosLine, cursorPosCol, cursorPosLine);
        } else {
            clearSelection();
        }

    } else {
        if (_cursorPosLine > 0) {
            int cursorPosLine = _cursorPosLine, cursorPosCol = _cursorPosCol;

            --cursorPosLine;
            if (cursorPosCol > _lines[cursorPosLine].length()) cursorPosCol = (int)_lines[cursorPosLine].length();

            setCursorPos(cursorPosCol, cursorPosLine);

            // If the user is holding the shift key, update the selection
            if (event->modifierFlags & MODIFIER_FLAG_SHIFT) {
                setSelection(_selBeginPosCol, _selBeginPosLine, cursorPosCol, cursorPosLine);
            } else {
                clearSelection();
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::handleKeyDown(const MDStudio::UIEvent* event) {
    if ((Platform::sharedInstance()->operatingSystem() == "osx") && (event->modifierFlags & MODIFIER_FLAG_COMMAND)) {
        int cursorPosLine = _cursorPosLine, cursorPosCol = _cursorPosCol;

        cursorPosLine = (unsigned int)_lines.size() - 1;
        cursorPosCol = (unsigned int)_lines[cursorPosLine].length();

        setCursorPos(cursorPosCol, cursorPosLine);

        // If the user is holding the shift key, update the selection
        if (event->modifierFlags & MODIFIER_FLAG_SHIFT) {
            setSelection(_selBeginPosCol, _selBeginPosLine, cursorPosCol, cursorPosLine);
        } else {
            clearSelection();
        }
    } else {
        if (_cursorPosLine < (_lines.size() - 1)) {
            int cursorPosLine = _cursorPosLine, cursorPosCol = _cursorPosCol;

            ++cursorPosLine;
            if (cursorPosCol > _lines[cursorPosLine].length()) cursorPosCol = (int)_lines[cursorPosLine].length();

            setCursorPos(cursorPosCol, cursorPosLine);

            // If the user is holding the shift key, update the selection
            if (event->modifierFlags & MODIFIER_FLAG_SHIFT) {
                setSelection(_selBeginPosCol, _selBeginPosLine, cursorPosCol, cursorPosLine);
            } else {
                clearSelection();
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::handleKeyEnter() {
    if (_isSingleLine) {
        responderChain()->makeFirstResponder(nullptr);
    } else {
        if (_undoManager) _undoManager->beginGroup();
        if (isTextSelected()) clearSelectedText();
        addCharacters(_cursorPosCol, _cursorPosLine, "\n");
        moveCursorForward(1);
        if (_undoManager) _undoManager->endGroup();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::handleKeyHome(const MDStudio::UIEvent* event) {
    if (Platform::sharedInstance()->operatingSystem() == "osx") {
        _cursorPosLine = 0;
        _cursorPosCol = 0;
    } else {
        _cursorPosCol = 0;
    }

    // If the user is holding the shift key, update the selection
    if (event->modifierFlags & MODIFIER_FLAG_SHIFT) {
        setSelection(_selBeginPosCol, _selBeginPosLine, _cursorPosCol, _cursorPosLine);
    } else {
        clearSelection();
    }

    if (_cursorPositionDidChangeFn) _cursorPositionDidChangeFn(this, _cursorPosCol, _cursorPosLine);
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::handleKeyEnd(const MDStudio::UIEvent* event) {
    if (Platform::sharedInstance()->operatingSystem() == "osx") {
        _cursorPosLine = (unsigned int)_lines.size() - 1;
        _cursorPosCol = (unsigned int)_lines[_cursorPosLine].length();
    } else {
        _cursorPosCol = (unsigned int)_lines[_cursorPosLine].length();
    }

    // If the user is holding the shift key, update the selection
    if (event->modifierFlags & MODIFIER_FLAG_SHIFT) {
        setSelection(_selBeginPosCol, _selBeginPosLine, _cursorPosCol, _cursorPosLine);
    } else {
        clearSelection();
    }

    if (_cursorPositionDidChangeFn) _cursorPositionDidChangeFn(this, _cursorPosCol, _cursorPosLine);
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::handleKeyTab(const MDStudio::UIEvent* event) {
    if (event->modifierFlags == 0) {
        if (_undoManager) _undoManager->beginGroup();
        if (isTextSelected()) clearSelectedText();
        addCharacters(_cursorPosCol, _cursorPosLine, "    ");
        moveCursorForward(4);
        if (_undoManager) _undoManager->endGroup();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool TextView::handleEvent(const UIEvent* event) {
    if (View::handleEvent(event)) return true;

    Rect rect = resolvedClippedRect();

    // Update the mouse cursor
    if (isMouseEvent(event)) {
        if (_isEnabled && isPointInRect(event->pt, rect) && _hasFocus) {
            responderChain()->setCursorInRect(this, Platform::IBeamCursor, rect);
        } else if (_hasFocus) {
            responderChain()->releaseCursor();
        }
    }

    if (((!_hasFocus && event->type == MOUSE_UP_UIEVENT) || (_hasFocus && event->type == MOUSE_DOWN_UIEVENT)) &&
        isPointInRect(event->pt, rect)) {
        handleMouse(event, rect);
        return true;
    } else if (_hasFocus) {
        if (event->type == CUT_UIEVENT) {
            handleCut();
        } else if (event->type == COPY_UIEVENT) {
            handleCopy();
        } else if (event->type == PASTE_UIEVENT) {
            handlePaste();
        } else if (event->type == MOUSE_DOWN_UIEVENT && !isPointInRect(event->pt, rect)) {
            responderChain()->makeFirstResponder(nullptr);
            responderChain()->sendEvent(event);
        } else if (event->type == MOUSE_UP_UIEVENT && (_isCaptured || isPointInRect(event->pt, rect))) {
            handleMouseUp(event);
        } else if (event->type == MOUSE_MOVED_UIEVENT && (_isCaptured || isPointInRect(event->pt, rect))) {
            handleMouseMoved(event);
        } else if (event->type == KEY_UIEVENT) {
            if (event->key == KEY_BACKSPACE) {
                handleKeyBackspace();  // else of if text is selected
            } else if (event->key == KEY_DELETE) {
                handleKeyDelete();  // else of if text is selected
            } else if (event->key == KEY_ESCAPE) {
                responderChain()->makeFirstResponder(nullptr);
            } else if (event->key == KEY_LEFT) {
                handleKeyLeft(event);
            } else if (event->key == KEY_RIGHT) {
                handleKeyRight(event);
            } else if (event->key == KEY_UP) {
                handleKeyUp(event);
            } else if (event->key == KEY_DOWN) {
                handleKeyDown(event);
            } else if (event->key == KEY_ENTER) {
                handleKeyEnter();
            } else if (event->key == KEY_HOME) {
                handleKeyHome(event);
            } else if (event->key == KEY_END) {
                handleKeyEnd(event);
            } else if (event->key == KEY_TAB) {
                handleKeyTab(event);
            } else if (event->characters.length() > 0) {
                if (_undoManager) _undoManager->beginGroup();
                if (isTextSelected()) clearSelectedText();
                addCharacters(_cursorPosCol, _cursorPosLine, event->characters);
                moveCursorForward(1);
                if (_undoManager) _undoManager->endGroup();
            }

            Platform::sharedInstance()->cancelDelayedInvokes(this);

            // If we still have the focus, we cancel the blinking cursor
            if (_hasFocus) {
                _isCursorVisible = false;
                blinkCursor();
            }
        } else {
            // Unhandled event
            return false;
        }
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
int TextView::addCharacters(int cursorPosCol, int cursorPosLine, const std::string& characters) {
    int nbCharactersAdded = 0;

    if (_undoManager) {
        _undoManager->pushFn([=]() { deleteCharacters(cursorPosCol, cursorPosLine, characters); });
    }

    UString strToInsert(characters);

    for (auto c : *strToInsert.str16()) {
        if (c == u'\n') {
            if (cursorPosCol == 0) {
                _lines.insert(_lines.begin() + cursorPosLine, UString());
                cursorPosLine++;
                cursorPosCol = 0;
            } else {
                UString after =
                    _lines[cursorPosLine].substr(cursorPosCol, _lines[cursorPosLine].length() - cursorPosCol);
                _lines[cursorPosLine] = _lines[cursorPosLine].substr(0, cursorPosCol);
                _lines.insert(_lines.begin() + cursorPosLine + 1, after);
                cursorPosLine++;
                cursorPosCol = 0;
            }
        } else {
            UString before = _lines[cursorPosLine].substr(0, cursorPosCol);
            UString after = _lines[cursorPosLine].substr(cursorPosCol, _lines[cursorPosLine].length() - cursorPosCol);
            _lines[cursorPosLine] = before;
            _lines[cursorPosLine] = _lines[cursorPosLine].append(c);
            _lines[cursorPosLine] = _lines[cursorPosLine].append(after);
            cursorPosCol++;
        }
        nbCharactersAdded++;
    }

    clearSelection();

    if (_textChangingFn) _textChangingFn(this, text());

    return nbCharactersAdded;
}

// ---------------------------------------------------------------------------------------------------------------------
int TextView::deleteCharacters(int cursorPosCol, int cursorPosLine, const std::string& characters) {
    int nbCharactersDeleted = 0;

    if (_undoManager) {
        _undoManager->pushFn([=]() { addCharacters(cursorPosCol, cursorPosLine, characters); });
    }

    UString strToRemove(characters);
    auto str16 = strToRemove.str16();

    for (auto it = str16->begin(); it != str16->end(); ++it) {
        if (_lines[cursorPosLine].isEmpty() && (_lines.size() > 1) && (cursorPosLine < (_lines.size() - 1))) {
            _lines.erase(_lines.begin() + cursorPosLine);

        } else if (cursorPosCol < _lines[cursorPosLine].length()) {
            _lines[cursorPosLine].erase(cursorPosCol, 1);
        } else {
            // We are at the end of the line
            if ((_lines.size() > 1) && (cursorPosLine < (_lines.size() - 1))) {
                _lines[cursorPosLine] = _lines[cursorPosLine].append(_lines[cursorPosLine + 1]);
                _lines.erase(_lines.begin() + cursorPosLine + 1);
            }
        }
        nbCharactersDeleted++;
    }

    setDirty();

    if (_textChangingFn) _textChangingFn(this, text());

    return nbCharactersDeleted;
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::resignFirstResponder() {
    responderChain()->releaseResponder(this);
    _hasFocus = false;
    _isCaptured = false;
    setDirty();
    Platform::sharedInstance()->cancelDelayedInvokes(this);
    if (_textDidChangeFn) _textDidChangeFn(this, text());

    if (_isSingleLine) {
        _cursorPosCol = 0;
        if (_cursorPositionDidChangeFn) _cursorPositionDidChangeFn(this, _cursorPosCol, _cursorPosLine);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::setTextInternal(std::string text) {
    _lines.clear();

    if (_isSingleLine) {
        _lines.push_back(text);

    } else {
        std::string line;
        for (auto c : text) {
            if (c == '\n') {
                _lines.push_back(line);
                line.clear();
            } else {
                line += c;
            }
        }
        _lines.push_back(line);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::setText(std::string text, bool isDelegateNotified) {
    stopEdition();

    setTextInternal(text);

    _cursorPosCol = 0;
    _cursorPosLine = 0;

    clearSelection();
    setDirty();

    if (isDelegateNotified) {
        if (_textChangingFn) _textChangingFn(this, text);

        if (_textDidChangeFn) _textDidChangeFn(this, text);

        if (_cursorPositionDidChangeFn) _cursorPositionDidChangeFn(this, _cursorPosCol, _cursorPosLine);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
std::string TextView::text() {
    std::string ret;

    unsigned int lineIndex = 0;
    for (auto line : _lines) {
        ret += line.string();
        // If not last line
        if (lineIndex < (_lines.size() - 1)) ret += '\n';
        ++lineIndex;
    }

    return ret;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<unsigned int> TextView::lineLengths() {
    std::vector<unsigned int> ret;
    for (auto line : _lines) {
        ret.push_back(static_cast<unsigned int>(line.length()));
    }
    return ret;
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::setCursorPos(unsigned int posCol, unsigned int posLine, bool isDelegateNotified, bool isUndoable) {
    if (isUndoable && _undoManager) {
        unsigned int cursorPosCol = _cursorPosCol, cursorPosLine = _cursorPosLine;
        _undoManager->pushFn([=]() { setCursorPos(cursorPosCol, cursorPosLine, isDelegateNotified, true); });
    }

    _cursorPosCol = posCol;
    _cursorPosLine = posLine;

    setDirty();

    if (isDelegateNotified && _cursorPositionDidChangeFn)
        _cursorPositionDidChangeFn(this, _cursorPosCol, _cursorPosLine);
}

// ---------------------------------------------------------------------------------------------------------------------
Size TextView::contentSize() {
    float maxWidth = 0.0f;
    for (auto line : _lines) {
        float width = getTextWidth(_font, line.string());
        if (width > maxWidth) maxWidth = width;
    }
    return makeSize(maxWidth + 1.0f, _lines.size() * fontHeight(_font));
}

// ---------------------------------------------------------------------------------------------------------------------
Rect TextView::cursorRect() {
    UString line = _lines[_cursorPosLine];
    std::string substr = line.substr(0, _cursorPosCol).string();
    float x = rect().origin.x + getTextWidth(_font, substr);
    float y = rect().origin.y + bounds().size.height - fontHeight(_font) * (_cursorPosLine + 1);
    Rect r = makeRect(x, y, 1.0f, fontHeight(_font));
    return r;
}
// ---------------------------------------------------------------------------------------------------------------------
Rect TextView::selectionCursorRect() {
    UString line = _lines[_selCursorPosLine];
    std::string substr = line.substr(0, _selCursorPosCol).string();
    float x = rect().origin.x + getTextWidth(_font, substr);
    float y = rect().origin.y + bounds().size.height - fontHeight(_font) * (_selCursorPosLine + 1);
    Rect r = makeRect(x, y, 1.0f, fontHeight(_font));
    return r;
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::startEdition() {
    if (_isEnabled && !_hasFocus) {
        if (_undoManager) _undoManager->clear();

        _hasFocus = true;
        responderChain()->makeFirstResponder(this);
        responderChain()->captureResponder(this);
        if (_isSingleLine) {
            _cursorPosCol = (unsigned int)_lines[0].length();
            // Make sure that the selection is at the cursor if no text is selected
            if (!isTextSelected()) _selBeginPosCol = _selEndPosCol = _cursorPosCol;
            if (_cursorPositionDidChangeFn) _cursorPositionDidChangeFn(this, _cursorPosCol, _cursorPosLine);
        }
        _isCursorVisible = true;
        setDirty();
        Platform::sharedInstance()->invokeDelayed(
            this, [=] { blinkCursor(); }, TEXT_VIEW_CURSOR_BLINK_PERIOD);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::stopEdition()
{
    if (_hasFocus) responderChain()->makeFirstResponder(nullptr);
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::clampSelection() {
    if (_lines.size() == 0) {
        _selBeginPosCol = _selEndPosCol = 0;
        _selBeginPosLine = _selEndPosLine = 0;
        return;
    }

    if (_selBeginPosLine > (unsigned int)_lines.size()) _selBeginPosLine = (unsigned int)_lines.size() - 1;

    if (_selEndPosLine > (unsigned int)_lines.size()) _selEndPosLine = (unsigned int)_lines.size() - 1;

    if (_selBeginPosCol > (unsigned int)_lines[_selBeginPosLine].length())
        _selBeginPosCol = (unsigned int)_lines[_selBeginPosLine].length();

    if (_selEndPosCol > (unsigned int)_lines[_selEndPosLine].length())
        _selEndPosCol = (unsigned int)_lines[_selEndPosLine].length();
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::setSelection(unsigned int selBeginPosCol, unsigned int selBeginPosLine, unsigned int selEndPosCol,
                            unsigned int selEndPosLine) {
    _selBeginPosCol = selBeginPosCol;
    _selBeginPosLine = selBeginPosLine;
    _selEndPosCol = selEndPosCol;
    _selEndPosLine = selEndPosLine;

    clampSelection();

    setDirty();
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::clearSelection() {
    _selBeginPosCol = _cursorPosCol;
    _selBeginPosLine = _cursorPosLine;
    _selEndPosCol = _cursorPosCol;
    _selEndPosLine = _cursorPosLine;

    setDirty();
}

// ---------------------------------------------------------------------------------------------------------------------
bool TextView::isTextSelected() { return (_selBeginPosCol != _selEndPosCol) || (_selEndPosLine != _selBeginPosLine); }

// ---------------------------------------------------------------------------------------------------------------------
UString TextView::selectedText() {
    unsigned int selBeginPosCol, selBeginPosLine;
    unsigned int selEndPosCol, selEndPosLine;

    getNormalizedSelectionPos(&selBeginPosCol, &selBeginPosLine, &selEndPosCol, &selEndPosLine);

    UString text;
    unsigned int lineCounter = 0;
    for (auto& line : _lines) {
        if (lineCounter >= selBeginPosLine && lineCounter <= selEndPosLine) {
            auto selBeginPosCol2 = (lineCounter == selBeginPosLine) ? selBeginPosCol : 0;
            auto selEndPosCol2 = (lineCounter == selEndPosLine) ? selEndPosCol : line.length();
            text = text.append(line.substr(selBeginPosCol2, selEndPosCol2 - selBeginPosCol2));
            if (lineCounter < selEndPosLine) text = text.append(UString("\n"));
        }
        ++lineCounter;
    }
    return text;
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int TextView::indexFromPosColLine(unsigned int posCol, unsigned int posLine) {
    size_t totalLength = 0;

    unsigned int lineCounter = 0;
    for (auto& line : _lines) {
        if (posLine == lineCounter) break;
        totalLength += line.length() + 1;  // consider that a CR is present
        ++lineCounter;
    }

    return (unsigned int)totalLength + posCol;
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::getNormalizedSelectionPos(unsigned int* selBeginPosCol, unsigned int* selBeginPosLine,
                                         unsigned int* selEndPosCol, unsigned int* selEndPosLine) {
    // Get the normalized selection positions
    *selBeginPosCol = _selBeginPosCol;
    *selBeginPosLine = _selBeginPosLine;
    *selEndPosCol = _selEndPosCol;
    *selEndPosLine = _selEndPosLine;

    if (*selBeginPosLine > *selEndPosLine) {
        unsigned int t = *selBeginPosLine;
        *selBeginPosLine = *selEndPosLine;
        *selEndPosLine = t;

        t = *selBeginPosCol;
        *selBeginPosCol = *selEndPosCol;
        *selEndPosCol = t;
    }

    if ((*selBeginPosCol > *selEndPosCol) && (*selBeginPosLine == *selEndPosLine)) {
        unsigned int t = *selBeginPosCol;
        *selBeginPosCol = *selEndPosCol;
        *selEndPosCol = t;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::clearSelectedText() {
    unsigned int selBeginPosCol, selBeginPosLine;
    unsigned int selEndPosCol, selEndPosLine;

    getNormalizedSelectionPos(&selBeginPosCol, &selBeginPosLine, &selEndPosCol, &selEndPosLine);
    setCursorPos(selBeginPosCol, selBeginPosLine);
    deleteCharacters(selBeginPosCol, selBeginPosLine, selectedText().string());

    clearSelection();
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::selectAllText() {
    if (_lines.size() > 0)
        setSelection(0, 0, (unsigned int)_lines[_lines.size() - 1].length(), (unsigned int)_lines.size() - 1);
}

// ---------------------------------------------------------------------------------------------------------------------
void TextView::selectAll() { selectAllText(); }
