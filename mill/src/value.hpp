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
        Value() : referenceCount(0) { }

        virtual ~Value() = 0;

    private:
        mutable std::atomic<long> referenceCount;

        friend void retain(Value const&);
        friend void release(Value const&);
    };

    class Unit : public Value {
    public:
        static Unit& instance() {
            static Unit unit;
            static auto x = (retain(unit), 0);
            (void)x;
            return unit;
        }

    private:
        Unit() = default;
    };

    template<typename T>
    class CXXValue : public Value {
    public:
        explicit CXXValue(T value) : value(std::move(value)) { }

        T value;
    };

    using Boolean = CXXValue<bool const>;
    using String = CXXValue<std::string const>;

    class Subroutine : public Value {
    public:
        template<typename F>
        explicit Subroutine(F slow)
            : slow(std::move(slow)), fast(), fastAvailable(false) { }

        Subroutine(Subroutine const&) = delete;
        Subroutine& operator=(Subroutine const&) = delete;

        boost::intrusive_ptr<Value> operator()(VM& vm, std::size_t argc, Value** argv) {
            if (fastAvailable) {
                return fast(vm, argc, argv);
            } else {
                return slow(vm, argc, argv);
            }
        }

    private:
        std::function<boost::intrusive_ptr<Value>(VM&, std::size_t, Value**)> slow;
        std::function<boost::intrusive_ptr<Value>(VM&, std::size_t, Value**)> fast;
        std::atomic<bool> fastAvailable;

        friend class VM;
    };

    void retain(Value const&);
    void release(Value const&);

    void intrusive_ptr_add_ref(Value const*);
    void intrusive_ptr_release(Value const*);

    template<typename T, typename... Args>
    boost::intrusive_ptr<T> make(Args&&... args) {
        return new T(std::forward<Args>(args)...);
    }
}
