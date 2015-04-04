#include <algorithm>
#include <array>
#include <cstring>
#include <type_traits>
#include "utility.hpp"

template<typename R, typename I>
R mill::memcpy_cast(I input) {
    static_assert(sizeof(R) == sizeof(I), "R and I must have the same size!");
    R result;
    std::memcpy(&result, &input, sizeof(result));
    return result;
}

template<typename T, typename It>
T mill::read_integer(It&& begin, byte_order bo) {
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
