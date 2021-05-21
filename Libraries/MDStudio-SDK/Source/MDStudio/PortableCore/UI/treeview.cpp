//
//  treeview.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2015-10-01.
//  Copyright (c) 2015-2020 Daniel Cliche. All rights reserved.
//

#include "treeview.h"

#include "draw.h"
#include "responderchain.h"

#define TREE_VIEW_INDENTATION_WIDTH 20.0f

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
TreeView::TreeView(const std::string& name, void* owner, Size itemSize) : _itemSize(itemSize), Control(name, owner) {
    _nbRowsFn = nullptr;
    _viewForIndexPathFn = nullptr;
    _didPerformLayoutFn = nullptr;
    _didChangeRowExpandedStateFn = nullptr;
    _didSelectRowFn = nullptr;
    _didDeselectRowFn = nullptr;
    _didHoverRowFn = nullptr;
    _didConfirmRowSelectionFn = nullptr;
    _didSetFocusStateFn = nullptr;
    _hasFocus = false;
    _isWaitingMouseUp = false;
    _isPassThrough = false;
}

// ---------------------------------------------------------------------------------------------------------------------
TreeView::~TreeView() { removeAllSubviews(); }

// ---------------------------------------------------------------------------------------------------------------------
void TreeView::addBranch(std::vector<int> indexPath) {
    unsigned int nbRows = _nbRowsFn(this, indexPath);

    indexPath.push_back(0);
    for (unsigned int row = 0; row < nbRows; row++) {
        using namespace std::placeholders;

        // Add a collapse button
        auto collapseButton = std::make_shared<Button>("collapseButton", this, "");
        collapseButton->setType(Button::DisclosureButtonType);
        collapseButton->setStateDidChangeFn(std::bind(&TreeView::collapseButtonStateDidChange, this, _1, _2));

        addSubview(collapseButton);

        _collapseButtons.push_back(std::make_pair(indexPath, collapseButton));

        // Add the view
        bool isExpanded = false;
        std::shared_ptr<View> rowView = _viewForIndexPathFn(this, indexPath, &isExpanded);
        _items.push_back(std::make_pair(indexPath, rowView));
        rowView->setIsVisible(false);
        addSubview(rowView);
        collapseButton->setState(isExpanded, false);

        // Add a branch
        addBranch(indexPath);

        // Increase the row index
        std::vector<int>::iterator it = indexPath.end() - 1;
        (*it)++;
    }

    indexPath.pop_back();
}

// ---------------------------------------------------------------------------------------------------------------------
void TreeView::collapseButtonStateDidChange(Button* sender, bool state) {
    layoutTree();

    if (_didChangeRowExpandedStateFn) {
        // Find the related index path
        for (auto collapseButton : _collapseButtons) {
            if (collapseButton.second.get() == sender)
                _didChangeRowExpandedStateFn(this, collapseButton.first, sender->state());
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TreeView::reload() {
    setSelectedRow({});

    removeAllSubviews();
    _collapseButtons.clear();

    _items.clear();
    _nbRows.clear();

    std::vector<int> indexPath;

    addBranch(indexPath);

    layoutTree();
}

// ---------------------------------------------------------------------------------------------------------------------
void TreeView::layoutBranch(float& y, unsigned int& viewIndex, std::vector<int> indexPath, bool isVisible) {
    unsigned int nbRows = _nbRowsFn(this, indexPath);

    float x = static_cast<float>(indexPath.size()) * TREE_VIEW_INDENTATION_WIDTH;
    indexPath.push_back(0);
    for (unsigned int row = 0; row < nbRows; ++row) {
        std::shared_ptr<Button> collapseButton = std::dynamic_pointer_cast<Button>(subviews()[2 * viewIndex]);
        std::shared_ptr<View> rowView = subviews()[2 * viewIndex + 1];
        viewIndex++;

        bool isLeaf = false;
        if (_nbRowsFn(this, indexPath) == 0) isLeaf = true;

        collapseButton->setIsVisible(isVisible && !isLeaf);
        rowView->setIsVisible(isVisible);
        rowView->setDirty();
        if (isVisible) {
            Rect r = makeRect(x, y, 12.0f, _itemSize.height);
            collapseButton->setFrame(makeCenteredRectInRect(r, 12.0f, 12.0f));
            rowView->setFrame(makeRect(x + 17.0f, y, _itemSize.width, _itemSize.height));
            y -= rowView->rect().size.height;
            _contentSize.width = std::max(x + _itemSize.width, _contentSize.width);
            _contentSize.height += rowView->rect().size.height;

            // Layout branch
            layoutBranch(y, viewIndex, indexPath, collapseButton->state());
        } else {
            // Layout branch
            layoutBranch(y, viewIndex, indexPath, false);
        }

        // Increase the row index
        std::vector<int>::iterator it = indexPath.end() - 1;
        (*it)++;
    }

    indexPath.pop_back();
}

// ---------------------------------------------------------------------------------------------------------------------
void TreeView::layoutTree(bool isDelegateNotified) {
    if (subviews().empty()) return;

    _contentSize = makeZeroSize();

    std::vector<int> indexPath;
    float y = rect().size.height - _itemSize.height;
    unsigned int viewIndex = 0;
    layoutBranch(y, viewIndex, indexPath, true);

    if (isDelegateNotified && _didPerformLayoutFn) _didPerformLayoutFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<View> TreeView::viewAtIndexPath(std::vector<int> indexPath) {
    for (auto item : _items) {
        if (item.first == indexPath) return item.second;
    }
    return nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------
void TreeView::setSelectedRow(std::vector<int> row, bool isDelegateNotified) {
    if (_selectedRow.size() > 0) {
        if (isDelegateNotified && _didDeselectRowFn) _didDeselectRowFn(this, _selectedRow);
    }

    _selectedRow = row;

    if ((row.size() > 0) && isDelegateNotified && _didSelectRowFn) _didSelectRowFn(this, row);
}

// ---------------------------------------------------------------------------------------------------------------------
void TreeView::exposeRow(std::vector<int> row, bool isDelegateNotified) {
    for (auto collapseButton : _collapseButtons) {
        // Check if there is a match between the provided index path and the collapse button index path
        bool isFound = true;
        for (size_t i = 0; (i < collapseButton.first.size()) && (i < row.size()); ++i) {
            if (collapseButton.first[i] != row[i]) {
                isFound = false;
                break;
            }
        }
        if (isFound && (collapseButton.first.size() < row.size()))
            if (!collapseButton.second->state()) collapseButton.second->setState(true, isDelegateNotified);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool TreeView::handleEvent(const UIEvent* event) {
    if (!isVisible()) return false;

    bool hadFocus = _hasFocus;

    if ((_hasFocus && (event->type == KEY_UIEVENT)) ||
        ((event->type == MOUSE_DOWN_UIEVENT || event->type == MOUSE_UP_UIEVENT || event->type == MOUSE_MOVED_UIEVENT) &&
         (isPointInRect(event->pt, clippedRect())))) {
        if (event->type == MOUSE_DOWN_UIEVENT && !_hasFocus) {
            _hasFocus = responderChain()->captureResponder(this);
            _isWaitingMouseUp = _hasFocus;
            if (_didSetFocusStateFn) _didSetFocusStateFn(this, _hasFocus);
        }

        if (event->type == KEY_UIEVENT) {
            if (event->key == KEY_UP) {
                std::pair<std::vector<int>, std::shared_ptr<View>> lastVisibleItem;
                for (auto item : _items) {
                    if (item.first == _selectedRow) break;
                    if (item.second->isVisible()) lastVisibleItem = item;
                }
                if (lastVisibleItem.second) setSelectedRow(lastVisibleItem.first);

            } else if (event->key == KEY_DOWN) {
                std::pair<std::vector<int>, std::shared_ptr<View>> lastVisibleItem;
                for (auto it = _items.rbegin(); it != _items.rend(); ++it) {
                    auto item = *it;
                    if (item.first == _selectedRow) break;
                    if (item.second->isVisible()) lastVisibleItem = item;
                }
                if (lastVisibleItem.second) setSelectedRow(lastVisibleItem.first);

            } else if (event->key == KEY_LEFT) {
                // Collapse the selected item
                for (auto collapseButton : _collapseButtons) {
                    if ((collapseButton.first == _selectedRow) && collapseButton.second->state())
                        collapseButton.second->setState(false);
                }
            } else if (event->key == KEY_RIGHT) {
                // Collapse the selected item
                for (auto collapseButton : _collapseButtons) {
                    if ((collapseButton.first == _selectedRow) && !collapseButton.second->state())
                        collapseButton.second->setState(true);
                }
            } else if (_didConfirmRowSelectionFn && (event->key == KEY_ENTER)) {
                _didConfirmRowSelectionFn(this, _selectedRow);
            } else {
                return false;
            }
        } else {
            Point pt = makePoint(event->pt.x - offset().x, event->pt.y - offset().y);

            std::vector<int> indexPath;
            // Find the index path for the event point
            for (auto item : _items) {
                std::shared_ptr<View> view = item.second;
                if (view->isVisible() && isPointInRect(pt, view->rect())) {
                    indexPath = item.first;
                }
            }

            if (indexPath.size() > 0) {
                if ((event->type == MOUSE_DOWN_UIEVENT) || (event->type == MOUSE_UP_UIEVENT)) {
                    if (event->type == MOUSE_UP_UIEVENT && _selectedRow != indexPath) {
                        setSelectedRow(indexPath);

                        if (_didConfirmRowSelectionFn) _didConfirmRowSelectionFn(this, indexPath);
                    } else {
                        // If we have the focus, we do not intercept the event if we allow pass through
                        if (_isPassThrough && hadFocus && _hasFocus && _selectedRow == indexPath) {
                            if (!(_isWaitingMouseUp && (event->type == MOUSE_UP_UIEVENT))) {
                                return false;
                            }
                        }
                    }

                } else {
                    if (_didHoverRowFn != nullptr) _didHoverRowFn(this, indexPath);
                }
            } else {
                return false;
            }
        }

        if (event->type == MOUSE_UP_UIEVENT) {
            _isWaitingMouseUp = false;
        }

        return true;
    } else if (_hasFocus && (event->type == MOUSE_DOWN_UIEVENT) && !isPointInRect(event->pt, clippedRect())) {
        responderChain()->releaseResponder(this);
        _hasFocus = false;
        _isWaitingMouseUp = false;
        if (_didSetFocusStateFn) _didSetFocusStateFn(this, _hasFocus);
        responderChain()->sendEvent(event);
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
void TreeView::setFrame(Rect aRect) {
    View::setFrame(aRect);

    layoutTree(false);
}

// ---------------------------------------------------------------------------------------------------------------------
void TreeView::captureFocus() {
    if (!_hasFocus) {
        _hasFocus = responderChain()->captureResponder(this);
        if (_didSetFocusStateFn) _didSetFocusStateFn(this, _hasFocus);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void TreeView::releaseFocus() {
    if (_hasFocus) {
        responderChain()->releaseResponder(this);
        _hasFocus = false;
        if (_didSetFocusStateFn) _didSetFocusStateFn(this, _hasFocus);
    }
}
