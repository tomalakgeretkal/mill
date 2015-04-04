#pragma once
#include <algorithm>
#include <array>
#include <cstring>
#include <type_traits>

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
    R memcpy_cast(I input) {
        static_assert(sizeof(R) == sizeof(I), "R and I must have the same size!");
        R result;
        std::memcpy(&result, &input, sizeof(result));
        return result;
    }

    // Read an integer from the given iterator assuming the given byte order.
    template<typename T, typename It>
    T read_integer(It&& begin, byte_order bo) {
        static_assert(std::is_integral<T>(), "read_integer can only read integers");
        std::array<char, sizeof(T)> data;
        for (auto& byte : data) {
            byte = *begin++;
        }
        if (bo != byte_order::native) {
            std::reverse(data.begin(), data.end());
        }
        return memcpy_cast<T>(data);
    }
}
