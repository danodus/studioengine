//
//  listview.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-26.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#include "listview.h"

#include <algorithm>

#include "draw.h"
#include "responderchain.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
ListView::ListView(const std::string& name, void* owner, float rowHeight, bool isMultipleSelectionsAllowed)
    : _rowHeight(rowHeight), _isMultipleSelectionsAllowed(isMultipleSelectionsAllowed), Control(name, owner) {
    _nbRowsFn = nullptr;
    _viewForRowFn = nullptr;
    _didSelectRowFn = nullptr;
    _didDeselectRowFn = nullptr;
    _didHoverRowFn = nullptr;
    _didConfirmRowSelectionFn = nullptr;
    _didSetFocusStateFn = nullptr;
    _didPressUnhandledKeyFn = nullptr;
    _selectedRow = -1;
    _hasFocus = false;
    _isWaitingMouseUp = false;
    _isDeselectingRow = false;
    _nbRows = 0;
    _isPassThrough = false;
}

// ---------------------------------------------------------------------------------------------------------------------
ListView::~ListView() { removeAllSubviews(); }

// ---------------------------------------------------------------------------------------------------------------------
void ListView::reload() {
    setSelectedRow(-1);

    removeAllSubviews();
    _items.clear();

    _nbRows = _nbRowsFn(this);

    for (unsigned int row = 0; row < _nbRows; row++) {
        std::shared_ptr<View> rowView = _viewForRowFn(this, row);
        addSubview(rowView);
        _items[row] = rowView;
    }

    layoutList();
}

// ---------------------------------------------------------------------------------------------------------------------
void ListView::layoutList() {
    for (auto item : _items) {
        auto row = item.first;
        auto rowView = item.second;
        auto y = rect().size.height - (_rowHeight) * (row + 1);
        rowView->setFrame(makeRect(0.0f, y, rect().size.width, _rowHeight));
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ListView::reloadWindow(bool isDelayed, bool isRefreshOnly) {
    if (isDelayed) {
        _isReloadWindowPending = true;
    } else {
        float endPosY = resolvedClippedRect().size.height - resolvedOffset().y;
        float startPosY = endPosY - resolvedClippedRect().size.height - _rowHeight;

        if (!isRefreshOnly) {
            removeAllSubviews();
            _items.clear();
        }

        _nbRows = _nbRowsFn(this);

        float y = rect().size.height - _rowHeight;

        unsigned int minRow = 0, maxRow = 0;
        bool isMinRowSet = false;
        for (unsigned int row = 0; row < _nbRows; row++) {
            if (y >= startPosY && y <= endPosY) {
                if (!isMinRowSet) {
                    minRow = row;
                    isMinRowSet = true;
                }
                maxRow = row;

                bool isFetchRequired = true;
                if (isRefreshOnly) isFetchRequired = _items.find(row) == _items.end();

                if (isFetchRequired) {
                    std::shared_ptr<View> rowView = _viewForRowFn(this, row);
                    addSubview(rowView);
                    _items[row] = rowView;
                }
            }
            if (y < startPosY) break;
            y -= _rowHeight;
        }

        if (isRefreshOnly) {
            // Remove subviews no longer visible
            for (auto it = _items.begin(); it != _items.end();)
                if (it->first < minRow || it->first > maxRow) {
                    removeSubview(it->second);
                    it = _items.erase(it);
                } else {
                    ++it;
                }
        }

        layoutList();
    }

    setDirty();
}

// ---------------------------------------------------------------------------------------------------------------------
float ListView::contentHeight() { return _nbRowsFn(this) * _rowHeight; }

// ---------------------------------------------------------------------------------------------------------------------
Rect ListView::viewRectAtRow(int row) {
    auto r = rect();
    auto y = r.size.height - (_rowHeight) * (row + 1);
    return makeRect(r.origin.x, r.origin.y + y, r.size.width, _rowHeight);
}

// ---------------------------------------------------------------------------------------------------------------------
void ListView::setFrame(Rect aRect) {
    View::setFrame(aRect);

    layoutList();
}

// ---------------------------------------------------------------------------------------------------------------------
void ListView::didResolveClippedRect() {
    if (_isReloadWindowPending) {
        _isReloadWindowPending = false;
        Platform::sharedInstance()->invoke([=] { reloadWindow(false, false); });
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ListView::setSelectedRow(int row, bool isDelegateNotified, bool isExclusive) {
    if (!_isMultipleSelectionsAllowed) {
        // Deselect all rows except the selected one
        std::vector<int> selectedRows = _selectedRows;
        _selectedRows.clear();
        for (auto r : selectedRows) {
            if ((r != row) && isDelegateNotified && _didDeselectRowFn != nullptr) _didDeselectRowFn(this, r);
        }
        if (row >= 0) _selectedRows.push_back(row);
    } else {
        // Multiple selections are allowed
        if (isExclusive) {
            std::vector<int> selectedRows = _selectedRows;
            _selectedRows.clear();
            if (isDelegateNotified && _didDeselectRowFn != nullptr) {
                for (auto row : selectedRows) _didDeselectRowFn(this, row);
            }
        }
        if (row >= 0 && std::find(_selectedRows.begin(), _selectedRows.end(), row) == _selectedRows.end())
            _selectedRows.push_back(row);
    }

    _selectedRow = row;

    if ((row >= 0) && isDelegateNotified && _didSelectRowFn != nullptr) _didSelectRowFn(this, row);
}

// ---------------------------------------------------------------------------------------------------------------------
void ListView::setSelectedRangeRows(int firstRow, int lastRow) {
    for (int row = firstRow; row <= lastRow; ++row) {
        // If the row is not already selected
        if (std::find(_selectedRows.begin(), _selectedRows.end(), row) == _selectedRows.end())
            setSelectedRow(row, true, false);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool ListView::isRowSelected(int row) {
    if (row < 0) return false;

    if (!_isMultipleSelectionsAllowed) {
        return _selectedRow == row;
    } else {
        return std::find(_selectedRows.begin(), _selectedRows.end(), row) != _selectedRows.end();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ListView::deselectRow(int row) {
    if (_isMultipleSelectionsAllowed) {
        _selectedRows.erase(std::find(_selectedRows.begin(), _selectedRows.end(), row));
    }

    if (row == _selectedRow) _selectedRow = -1;

    if (_didSelectRowFn != nullptr) _didDeselectRowFn(this, row);
}

// ---------------------------------------------------------------------------------------------------------------------
bool ListView::handleEvent(const UIEvent* event) {
    if (_isStatic) return false;

    if (View::handleEvent(event)) return true;

    if (!isVisible()) return false;

    bool hadFocus = _hasFocus;

    if ((_hasFocus && (event->type == KEY_UIEVENT)) ||
        ((event->type == MOUSE_DOWN_UIEVENT || event->type == MOUSE_UP_UIEVENT || event->type == MOUSE_MOVED_UIEVENT) &&
         (isPointInRect(event->pt, resolvedClippedRect())))) {
        if (event->type == MOUSE_DOWN_UIEVENT && !_hasFocus) {
            responderChain()->makeFirstResponder(this);
            if (_isCapturing) responderChain()->captureResponder(this);
            _hasFocus = true;
            _isWaitingMouseUp = _hasFocus;
            if (_didSetFocusStateFn) _didSetFocusStateFn(this, _hasFocus);
        }

        if (event->type == KEY_UIEVENT) {
            if (event->key == KEY_UP) {
                if (_selectedRow > 0) {
                    if (event->modifierFlags & MODIFIER_FLAG_SHIFT) {
                        int row = _selectedRow - 1;
                        if (_selectedRow > _selectedRowRef) deselectRow(_selectedRow);
                        setSelectedRow(row, true, false);
                    } else {
                        setSelectedRow(_selectedRow - 1);
                        _selectedRowRef = _selectedRow;
                    }
                }
            } else if (event->key == KEY_DOWN) {
                if (_selectedRow < _nbRows - 1) {
                    if (event->modifierFlags & MODIFIER_FLAG_SHIFT) {
                        int row = _selectedRow + 1;
                        if (_selectedRow < _selectedRowRef) deselectRow(_selectedRow);
                        setSelectedRow(row, true, false);
                    } else {
                        setSelectedRow(_selectedRow + 1);
                        _selectedRowRef = _selectedRow;
                    }
                }

            } else if (_didConfirmRowSelectionFn && (event->key == KEY_ENTER)) {
                _didConfirmRowSelectionFn(this, _selectedRow);
            } else {
                if (_didPressUnhandledKeyFn) {
                    return _didPressUnhandledKeyFn(this, event->key);
                } else {
                    return false;
                }
            }
        } else {
            int row = (rect().origin.y + rect().size.height + resolvedOffset().y - event->pt.y) / _rowHeight;

            if (row >= _nbRows) return false;

            if (event->type == MOUSE_DOWN_UIEVENT || event->type == MOUSE_UP_UIEVENT) {
                if (event->modifierFlags & MODIFIER_FLAG_SHIFT) {
                    if (_selectedRow >= 0) {
                        int firstRow = _selectedRow;
                        int lastRow = row;
                        if (firstRow > lastRow) {
                            int r = firstRow;
                            firstRow = lastRow;
                            lastRow = r;
                        }
                        setSelectedRangeRows(firstRow, lastRow);
                    }
                } else if (!isRowSelected(row)) {
                    if (!_isDeselectingRow && (event->type == MOUSE_UP_UIEVENT)) {
                        setSelectedRow(row, true, !(event->modifierFlags & MODIFIER_FLAG_COMMAND));
                        _selectedRowRef = _selectedRow;
                    }
                } else {
                    // The row is already selected
                    if (event->modifierFlags & MODIFIER_FLAG_COMMAND) {
                        if (event->type == MOUSE_DOWN_UIEVENT) {
                            deselectRow(row);
                            _isDeselectingRow = true;
                        }
                    } else {
                        if (event->type == MOUSE_UP_UIEVENT && _selectedRows.size() > 1) {
                            setSelectedRow(row);
                            _selectedRowRef = _selectedRow;
                        } else {
                            // If we have the focus, we do not intercept the event if we allow pass through
                            if (_isPassThrough && hadFocus && _hasFocus) {
                                if (!(_isWaitingMouseUp && (event->type == MOUSE_UP_UIEVENT))) {
                                    _isWaitingMouseUp = false;
                                    return false;
                                }
                            }
                        }
                    }
                }

                if (event->type == MOUSE_UP_UIEVENT) {
                    _isWaitingMouseUp = false;
                    _isDeselectingRow = false;
                }

                if ((event->type == MOUSE_UP_UIEVENT) && _didConfirmRowSelectionFn)
                    _didConfirmRowSelectionFn(this, row);

            } else {
                // Mouse move
                if (_didHoverRowFn != nullptr) _didHoverRowFn(this, row);

                if (_isPassThrough && _hasFocus) return false;
            }
        }
        return true;
    } else if (_hasFocus && (event->type == MOUSE_DOWN_UIEVENT) && !isPointInRect(event->pt, resolvedClippedRect())) {
        responderChain()->makeFirstResponder(nullptr);
        _isWaitingMouseUp = false;
        responderChain()->sendEvent(event);
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
void ListView::captureFocus() {
    if (!_hasFocus) {
        _hasFocus = true;
        responderChain()->makeFirstResponder(this);
        if (_isCapturing) responderChain()->captureResponder(this);
        if (_didSetFocusStateFn) _didSetFocusStateFn(this, _hasFocus);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ListView::releaseFocus() {
    if (_hasFocus) {
        responderChain()->makeFirstResponder(nullptr);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ListView::resignFirstResponder() {
    if (_hasFocus) {
        if (_isCapturing) responderChain()->releaseResponder(this);
        _hasFocus = false;
        if (_didSetFocusStateFn) _didSetFocusStateFn(this, _hasFocus);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<View> ListView::viewAtRow(int row) {
    // If the view is available, return it directly
    if (_items.find(row) != _items.end()) return _items[row];

    // The view is not available
    return nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------
void ListView::selectAll() {
    if (_isMultipleSelectionsAllowed) setSelectedRangeRows(0, _nbRows - 1);
}
