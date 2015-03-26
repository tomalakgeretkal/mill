#pragma once
#include <atomic>
#include <boost/intrusive_ptr.hpp>
#include <boost/utility.hpp>
#include <cstddef>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>

namespace mill {
    class VM;

    class Value {
    public:
        virtual ~Value() = 0;

    private:
        mutable std::atomic<long> referenceCount;

        friend void retain(Value const&);
        friend void release(Value const&);
    };

    class Unit : public Value { };

    template<typename T>
    class CXXValue : public Value {
    public:
        explicit CXXValue(T value) : value(std::move(value)) { }

        T value;
    };

    using Boolean = CXXValue<bool const>;
    using String = CXXValue<std::string const>;
    using Subroutine = CXXValue<std::function<boost::intrusive_ptr<Value>(VM&, std::size_t, Value**)>>;

    void retain(Value const&);
    void release(Value const&);

    void intrusive_ptr_add_ref(Value const*);
    void intrusive_ptr_release(Value const*);

    template<typename T, typename... Args>
    boost::intrusive_ptr<T> make(Args&&... args) {
        return new T(std::forward<Args>(args)...);
    }
}
