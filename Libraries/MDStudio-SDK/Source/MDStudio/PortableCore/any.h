//
//  any.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-08-23.
//  Copyright (c) 2014 Daniel Cliche. All rights reserved.
//

//
//  Based on code available here: http://codereview.stackexchange.com/questions/20058/a-c11-any-class
//

#ifndef ANY_H
#define ANY_H

#include <cassert>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>

using namespace std;

template <class T>
using StorageType = typename decay<T>::type;

struct Any {
    bool is_null() const { return !ptr; }
    bool not_null() const { return ptr; }

    template <typename U>
    Any(U&& value) : ptr(new Derived<StorageType<U>>(forward<U>(value))) {}

    template <class U>
    bool is() const {
        typedef StorageType<U> T;

        auto derived = dynamic_cast<Derived<T>*>(ptr);

        return derived;
    }

    template <class U>
    StorageType<U>& as() const {
        typedef StorageType<U> T;

        auto derived = dynamic_cast<Derived<T>*>(ptr);

        if (!derived) throw bad_cast();

        return derived->value;
    }

    template <class U>
    operator U() {
        return as<StorageType<U>>();
    }

    Any() : ptr(nullptr) {}

    Any(Any& that) : ptr(that.clone()) {}

    Any(Any&& that) : ptr(that.ptr) { that.ptr = nullptr; }

    Any(const Any& that) : ptr(that.clone()) {}

    Any(const Any&& that) : ptr(that.clone()) {}

    Any& operator=(const Any& a) {
        if (ptr == a.ptr) return *this;

        auto old_ptr = ptr;

        ptr = a.clone();

        if (old_ptr) delete old_ptr;

        return *this;
    }

    Any& operator=(Any&& a) {
        if (ptr == a.ptr) return *this;

        swap(ptr, a.ptr);

        return *this;
    }

    ~Any() {
        if (ptr) delete ptr;
    }

   private:
    struct Base {
        virtual ~Base() {}

        virtual Base* clone() const = 0;
    };

    template <typename T>
    struct Derived : Base {
        template <typename U>
        Derived(U&& value) : value(forward<U>(value)) {}

        T value;

        Base* clone() const { return new Derived<T>(value); }
    };

    Base* clone() const {
        if (ptr)
            return ptr->clone();
        else
            return nullptr;
    }

    Base* ptr;
};

#endif
