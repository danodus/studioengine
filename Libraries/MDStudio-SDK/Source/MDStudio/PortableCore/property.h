//
//  property.h
//  MDStudio
//
//  Created by Daniel Cliche on 2020-03-09.
//  Copyright (c) 2020-2021 Daniel Cliche. All rights reserved.
//

#ifndef PROPERTY_H
#define PROPERTY_H

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "any.h"

namespace MDStudio {

class Property {
   public:
    typedef std::function<void(Property* sender)> ValueWillChangeFnType;
    typedef std::function<void(Property* sender)> ValueDidChangeFnType;

   private:
    void* _owner = nullptr;

    std::string _name;
    Any _value;
    Property* _boundProperty = nullptr;

    std::vector<ValueWillChangeFnType> _valueWillChangeFns;
    std::vector<ValueDidChangeFnType> _valueDidChangeFns;

   public:
    Property(std::string name) : _name(name) {}

    Property(std::string name, Any value) : _name(name), _value(value) {}

    void* owner() { return _owner; }
    void setOwner(void* owner) { _owner = owner; }

    void notifyValueWillChange(bool isForwarded = true) {
        for (auto valueWillChangeFn : _valueWillChangeFns) valueWillChangeFn(this);
        if (_boundProperty && isForwarded) _boundProperty->notifyValueWillChange();
    }

    void notifyValueDidChange(bool isForwarded = true) {
        for (auto valueDidChangeFn : _valueDidChangeFns) valueDidChangeFn(this);
        if (_boundProperty && isForwarded) _boundProperty->notifyValueDidChange();
    }

    Any value() { return _value; }
    template <typename T>
    T value() {
        return _value.as<T>();
    }

    void setValue(Any value, bool isDelegateNotified = true, bool isForwarded = true) {
        if (isDelegateNotified) notifyValueWillChange(false);
        _value = value;
        if (_boundProperty && isForwarded) _boundProperty->setValue(_value, true, false);
        if (isDelegateNotified) notifyValueDidChange(false);
    }

    void bindForward(Property* property) {
        if (!_boundProperty) {
            _boundProperty = property;
            if (_value.not_null()) _boundProperty->setValue(_value, true, false);
        }
    }

    static void bind(Property* property1, Property* property2) {
        property1->bindForward(property2);
        property2->bindForward(property1);
    }

    void operator=(const Any value) { setValue(value); }

    void setValueWillChangeFn(ValueWillChangeFnType valueWillChangeFn) {
        _valueWillChangeFns.push_back(valueWillChangeFn);
    }
    void setValueDidChangeFn(ValueDidChangeFnType valueDidChangeFn) { _valueDidChangeFns.push_back(valueDidChangeFn); }
};

}  // namespace MDStudio

#endif  // PROPERTY_H
