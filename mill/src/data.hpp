#pragma once
#include <any_iterator.hpp>
#include <functional>
#include <iterator>
#include <unicode/unistr.h>
#include <memory>

namespace mill {
    // A handle to a Mill value.
    class handle {
    public:
        // Construct the handle with the given value.
        template<typename T>
        explicit handle(T value);

        // Extract the value from the handle. If T is not the C++ type of the
        // value, std::bad_cast is thrown.
        template<typename T>
        T& data() const;

    private:
        struct base {
            virtual ~base() = 0;
        };

        template<typename T>
        struct derived : base {
            explicit derived(T value);

            T value;
        };

        std::shared_ptr<base> ptr;
    };

    // The C++ type of the unit value.
    struct unit { };

    // The C++ type of strings.
    class string {
    public:

    private:
        icu::UnicodeString data;
    };

    // The C++ type of subroutines.
    class subroutine {
    public:
        template<typename F>
        explicit subroutine(F implementation);

        template<typename ArgumentIt>
        handle operator()(ArgumentIt arguments_begin, ArgumentIt arguments_end);

    private:
        using argument_iterator = IteratorTypeErasure::any_iterator<handle, std::random_access_iterator_tag>;
        std::function<handle(argument_iterator, argument_iterator)> implementation;
    };
}

#include "data.tpp"
