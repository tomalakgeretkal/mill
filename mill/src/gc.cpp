#include "gc.hpp"
#include "value.hpp"

mill::Value* mill::GC::alloc(Type& type) {
    auto value = static_cast<Value*>(operator new(type.size()));
    value->type = &type;
    return value;
}
