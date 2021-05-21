//
//  undomanager.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-11-15.
//  Copyright (c) 2014-2018 Daniel Cliche. All rights reserved.
//

#ifndef UNDOMANAGER_H
#define UNDOMANAGER_H

#include <functional>
#include <stack>

namespace MDStudio {

class UndoManager {
   public:
    typedef std::function<void(UndoManager* sender)> StatesDidChangeFnType;

   private:
    std::stack<std::stack<std::function<void()>>> _undoFnStack;
    std::stack<std::stack<std::function<void()>>> _redoFnStack;

    std::stack<std::function<void()>> _fnGroupStack;

    bool _isUndoing;
    bool _isRedoing;

    bool _isRegistrationEnabled;

    bool _isGrouping;

    StatesDidChangeFnType _statesDidChangeFn;

   public:
    UndoManager();

    void pushFn(std::function<void()> fn);

    void undo();
    void redo();

    void beginGroup();
    void endGroup();
    void clearCurrentGroup(size_t nbItemsToKeep = 0);

    const size_t nbOperationsInGroup() { return _fnGroupStack.size(); }

    void enableRegistration() { _isRegistrationEnabled = true; }
    void disableRegistration() { _isRegistrationEnabled = false; }

    void clear();

    bool canUndo() { return _undoFnStack.size() > 0; }
    bool canRedo() { return _redoFnStack.size() > 0; }

    bool hasRemainingOperationsInGroup();

    void setStatesDidChangeFn(StatesDidChangeFnType statesDidChangeFn) { _statesDidChangeFn = statesDidChangeFn; }
};

}  // namespace MDStudio

#endif  // UNDOMANAGER_H
