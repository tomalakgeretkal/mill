#pragma once
#include <boost/variant.hpp>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace mill {
    struct push_global_instruction { std::uint32_t name_index; };

    // An instruction that can be executed by the interpreter.
    using instruction = boost::variant<
        push_global_instruction
    >;

    // Thrown when the decoder encounters an invalid instruction.
    class bad_instruction : public std::runtime_error {
    public:
        // Construct the exception with the offset of the invalid instruction.
        explicit bad_instruction(std::size_t offset);
    };

    // Decode an instruction. Throw bad_instruction if an instruction
    // could not be decoded.
    //
    // - begin and end must be random access iterators to unsigned chars.
    template<typename CodeIt>
    instruction decode_instruction(CodeIt&& begin, CodeIt end);
}

#include "instruction.tpp"
