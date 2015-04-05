#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <type_traits>
#include <utility>
#include "utility.hpp"

template<typename R, typename I>
R mill::memcpy_cast(I input) {
    static_assert(sizeof(R) == sizeof(I), "R and I must have the same size!");
    R result;
    std::memcpy(&result, &input, sizeof(result));
    return result;
}

template<typename It, typename Out>
void mill::read_exactly(It&& begin, It&& end, std::size_t n, Out out) {
    if (static_cast<std::size_t>(std::distance(begin, end)) < n) {
        throw "fuck";
    }
    for (std::size_t i = 0; i < n; ++i) {
        *out++ = *begin++;
    }
}

template<typename T, typename It>
T mill::read_integer(It&& begin, It&& end, byte_order bo) {
    static_assert(std::is_integral<T>(), "read_integer can only read integers");
    std::array<unsigned char, sizeof(T)> data;
    read_exactly(begin, end, data.size(), data.begin());
    if (bo != byte_order::native) {
        std::reverse(data.begin(), data.end());
    }
    return memcpy_cast<T>(data);
}

template<typename R>
template<typename F, typename... Args>
mill::call<R>::call(F&& f, Args&&... args)
    : result(std::forward<F>(f)(std::forward<Args>(args)...)) { }
