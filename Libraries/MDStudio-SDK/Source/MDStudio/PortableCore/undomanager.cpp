//
//  undomanager.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-11-15.
//  Copyright (c) 2014-2018 Daniel Cliche. All rights reserved.
//

#include "undomanager.h"

#include <cassert>

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
UndoManager::UndoManager() {
    _statesDidChangeFn = nullptr;
    _isUndoing = false;
    _isRedoing = false;
    _isRegistrationEnabled = true;
    _isGrouping = false;
}

// ---------------------------------------------------------------------------------------------------------------------
void UndoManager::clearCurrentGroup(size_t nbItemsToKeep) {
    assert(_isGrouping);

    while (_fnGroupStack.size() > nbItemsToKeep) _fnGroupStack.pop();
}

// ---------------------------------------------------------------------------------------------------------------------
void UndoManager::beginGroup() {
    _isGrouping = true;

    // Clear the current stack
    clearCurrentGroup();
}

// ---------------------------------------------------------------------------------------------------------------------
void UndoManager::endGroup() {
    if (!_isRegistrationEnabled) {
        _isGrouping = false;
        return;
    }

    // Clear the redo stack if we are not undoing and redoing (new action)
    if (!_isUndoing && !_isRedoing) {
        while (_redoFnStack.size() > 0) _redoFnStack.pop();
    }

    if (_isUndoing) {
        if (!_fnGroupStack.empty()) _redoFnStack.push(_fnGroupStack);
    } else {
        if (!_fnGroupStack.empty()) _undoFnStack.push(_fnGroupStack);
    }

    if (_statesDidChangeFn) _statesDidChangeFn(this);

    _isGrouping = false;
}

// ---------------------------------------------------------------------------------------------------------------------
void UndoManager::pushFn(std::function<void()> fn) {
    bool isGrouping = _isGrouping;

    if (!isGrouping) beginGroup();
    _fnGroupStack.push(fn);
    if (!isGrouping) endGroup();
}

// ---------------------------------------------------------------------------------------------------------------------
void UndoManager::undo() {
    if (_undoFnStack.size() > 0) {
        std::stack<std::function<void()>>& stack = _undoFnStack.top();
        _isUndoing = true;
        beginGroup();
        while (stack.size() > 0) {
            std::function<void()> fn = stack.top();
            stack.pop();
            fn();
        }
        endGroup();
        _isUndoing = false;
        _undoFnStack.pop();
    }

    if (_statesDidChangeFn) _statesDidChangeFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void UndoManager::redo() {
    if (_redoFnStack.size() > 0) {
        std::stack<std::function<void()>>& stack = _redoFnStack.top();
        _isRedoing = true;
        beginGroup();
        while (stack.size() > 0) {
            std::function<void()> fn = stack.top();
            stack.pop();
            fn();
        }
        endGroup();
        _isRedoing = false;
        _redoFnStack.pop();
    }

    if (_statesDidChangeFn) _statesDidChangeFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void UndoManager::clear() {
    assert(!_isGrouping);

    while (_undoFnStack.size() > 0) _undoFnStack.pop();

    while (_redoFnStack.size() > 0) _redoFnStack.pop();

    if (_statesDidChangeFn) _statesDidChangeFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
bool UndoManager::hasRemainingOperationsInGroup() {
    if (!_isUndoing && !_isRedoing) return false;

    if (_isUndoing && _undoFnStack.empty()) return false;
    if (_isRedoing && _redoFnStack.empty()) return false;

    std::stack<std::function<void()>> stack = _isUndoing ? _undoFnStack.top() : _redoFnStack.top();
    return !stack.empty();
}
