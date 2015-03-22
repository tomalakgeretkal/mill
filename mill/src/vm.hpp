#pragma once
#include "gc.hpp"
#include "value.hpp"

namespace mill {
    class VM {
    public:
        VM() {
            unitType = &PrimitiveType<Unit>::instance();
            unit_ = gc.alloc(*unitType);

            booleanType = &PrimitiveType<bool>::instance();
            true__ = gc.alloc(*booleanType);
            booleanType->set(true__, true);
            false__ = gc.alloc(*booleanType);
            booleanType->set(false__, false);
        }

        Value* unit() { return unit_; }
        Value* true_() { return true__; }
        Value* false_() { return false__; }

        GC gc;

    private:
        struct Unit { };
        PrimitiveType<Unit>* unitType;
        Value* unit_;

        PrimitiveType<bool>* booleanType;
        Value* true__;
        Value* false__;
    };
}
