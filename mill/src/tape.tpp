#include <cstddef>
#include "instruction.hpp"
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
    (void)offset;
    throw 2;
}
