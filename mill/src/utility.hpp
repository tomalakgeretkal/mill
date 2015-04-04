#pragma once

namespace mill {
    // A byte order.
    enum class byte_order {
        big_endian,
        little_endian,
        network = big_endian,
        native = __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ ? little_endian : big_endian,
    };

    // Cast the input to R using std::memcpy. R and I must be of the
    // same size.
    template<typename R, typename I>
    R memcpy_cast(I input);

    // Read an integer from the given iterator assuming the given byte order.
    template<typename T, typename It>
    T read_integer(It&& begin, byte_order bo);
}

#include "utility.tpp"
