#pragma once
#include <algorithm>
#include <boost/utility.hpp>
#include <cstddef>
#include <functional>
#include "gc.hpp"
#include "object.hpp"
#include <unordered_map>
#include <utility>
#include "value.hpp"
#include <vector>

namespace mill {
    class VM : boost::noncopyable {
    public:
        struct Unit { };
        struct String { char* data; std::size_t size; };
        struct CXXSubroutine { std::function<GCPtr(VM&, std::size_t, GCPtr*)>* implementation; };

        VM() {
            unitType = &PrimitiveType<Unit>::instance();
            unit_ = gc.alloc(*unitType);

            booleanType = &PrimitiveType<bool>::instance();
            true__ = gc.alloc(*booleanType);
            booleanType->set(true__, true);
            false__ = gc.alloc(*booleanType);
            booleanType->set(false__, false);

            stringType = &PrimitiveType<String>::instance();

            cxxSubroutineType = &PrimitiveType<CXXSubroutine>::instance();
        }

        void loadObject(Object const& object) {
            for (auto&& string : object.strings) {
                strings[&object].push_back(this->string(string));
            }
        }

        GCPtr unit() { return unit_; }

        GCPtr true_() { return true__; }
        GCPtr false_() { return false__; }

        GCPtr string(std::string const& value) {
            auto result = gc.alloc(*stringType);

            String data;
            data.data = new char[value.size()];
            std::copy(value.begin(), value.end(), data.data);
            data.size = value.size();

            stringType->set(result, data);
            return result;
        }

        GCPtr string(Object const& object, std::size_t index) {
            return strings.at(&object)[index];
        }

        std::string unstring(GCPtr value) {
            auto data = stringType->get(value);
            return std::string(data.data, data.data + data.size);
        }

        void setGlobal(std::string name, GCPtr value) {
            globals[name] = value;
        }

        GCPtr global(Object const& object, std::size_t nameIndex) {
            auto const& name = strings.at(&object)[nameIndex];
            return globals.at(unstring(name));
        }

        template<typename F>
        GCPtr subroutine(F f) {
            auto result = gc.alloc(*cxxSubroutineType);
            CXXSubroutine data;
            data.implementation = new std::function<GCPtr(VM&, std::size_t, GCPtr*)>(std::move(f));
            cxxSubroutineType->set(result, data);
            return result;
        }

        GC gc;

    private:
        PrimitiveType<Unit>* unitType;
        GCPtr unit_;

        PrimitiveType<bool>* booleanType;
        GCPtr true__;
        GCPtr false__;

        PrimitiveType<String>* stringType;
        std::unordered_map<Object const*, std::vector<GCPtr>> strings;

        PrimitiveType<CXXSubroutine>* cxxSubroutineType;

        std::unordered_map<std::string, GCPtr> globals;
    };
}
