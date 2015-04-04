#include <cassert>
#include <cstddef>
#include "instruction.hpp"
#include <iterator>
#include "tape.hpp"

template<typename CodeIt>
mill::tape<CodeIt>::tape(CodeIt begin, CodeIt end)
    : it(begin), begin(std::move(begin)), end(std::move(end)) { }

template<typename CodeIt>
mill::instruction mill::tape<CodeIt>::read() {
    return decode_instruction(it, end);
}

template<typename CodeIt>
void mill::tape<CodeIt>::seek(std::size_t offset) {
    assert(offset < static_cast<std::size_t>(std::distance(begin, end)));
    it = begin + offset;
}
