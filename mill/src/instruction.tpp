#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstring>
#include "instruction.hpp"
#include <iterator>

template<typename CodeIt>
mill::instruction mill::decode_instruction(CodeIt&& it, CodeIt const end) {
    auto const begin = it;

    auto check = [&] (auto condition) {
        if (!condition) {
            throw bad_instruction(std::distance(begin, it));
        }
    };

    auto read_int = [&] (auto& out) {
        check(static_cast<std::size_t>(std::distance(it, end)) >= sizeof(out));
        std::array<unsigned char, sizeof(out)> data;
        std::copy(it, it + sizeof(out), data.begin());
        it += sizeof(out);
        std::memcpy(&out, data.data(), sizeof(out));
    };

    check(begin != end);

    switch (*it++) {
#include "../build/instruction_decode.inc"

        default:
            check(false);
            assert(false);
    }
}
