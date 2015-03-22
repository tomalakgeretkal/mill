#pragma once
#include <algorithm>
#include <cstddef>
#include "gc.hpp"
#include "object.hpp"
#include <unordered_map>
#include "value.hpp"
#include <vector>

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

            stringType = &PrimitiveType<String>::instance();
        }

        void loadObject(Object const& object) {
            for (auto&& string : object.strings) {
                strings[&object].push_back(this->string(string));
            }
        }

        Value* unit() { return unit_; }

        Value* true_() { return true__; }
        Value* false_() { return false__; }

        Value* string(std::string const& value) {
            auto result = gc.alloc(*stringType);

            String data;
            data.data = new char[value.size()];
            std::copy(value.begin(), value.end(), data.data);
            data.size = value.size();

            stringType->set(result, data);
            return result;
        }

        Value* string(Object const& object, std::size_t index) {
            return strings.at(&object)[index];
        }

        std::string unstring(Value* value) {
            auto data = stringType->get(value);
            return std::string(data.data, data.data + data.size);
        }

        GC gc;

    private:
        struct Unit { };
        PrimitiveType<Unit>* unitType;
        Value* unit_;

        PrimitiveType<bool>* booleanType;
        Value* true__;
        Value* false__;

        struct String { char* data; std::size_t size; };
        PrimitiveType<String>* stringType;
        std::unordered_map<Object const*, std::vector<Value*>> strings;
    };
}
