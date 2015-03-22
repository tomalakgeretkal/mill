#include "gc.hpp"
#include "value.hpp"

mill::GCPtr mill::GC::alloc(Type& type) {
    auto value = static_cast<Value*>(operator new(type.size()));
    value->type = &type;
    return root(value);
}

mill::GCPtr mill::GC::root(Value* value) {
    return GCPtr(value);
}
