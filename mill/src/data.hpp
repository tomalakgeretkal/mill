#pragma once
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
}

#include "data.tpp"
