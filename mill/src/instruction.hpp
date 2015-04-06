#pragma once
#include <boost/variant.hpp>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <stdexcept>

namespace mill {
#include "../build/instruction_type.ipp"

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
    instruction decode_instruction(CodeIt& begin, CodeIt const end);
}

#include "instruction.tpp"
