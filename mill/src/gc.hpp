#pragma once
#include <cstddef>

namespace mill {
    class Type;
    struct Value;

    class GCPtr {
    public:
        GCPtr() : value(nullptr) { }

        GCPtr(std::nullptr_t) : value(nullptr) { }

        Value* get() const {
            return value;
        }

        Value* operator->() const {
            return value;
        }

        Value& operator*() const {
            return *value;
        }

        explicit operator bool() const {
            return value;
        }

        bool operator!() const {
            return !value;
        }

    private:
        explicit GCPtr(Value* value) : value(value) { }

        Value* value;

        friend class GC;

        friend bool operator==(GCPtr a, GCPtr b) {
            return a.value == b.value;
        }
    };

    class GC {
    public:
        GCPtr alloc(Type& type);

        GCPtr root(Value* value);
    };
}
