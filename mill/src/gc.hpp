#pragma once
#include "value.hpp"

namespace mill {
    class GC {
    public:
        Value* alloc(Type& type);

    private:
    };
}
