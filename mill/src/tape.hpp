#pragma once
#include <cstddef>
#include "instruction.hpp"

namespace mill {
    // A tape of instructions which can be seeked and read from.
    //
    // - CodeIt must be the type of a random access iterator to
    //   unsigned chars.
    template<typename CodeIt>
    class tape {
    public:
        // Construct a tape given its begin and end.
        tape(CodeIt begin, CodeIt end);

        // Read one instruction off the tape and advance the tape. May throw
        // bad_instruction as per decode_instruction.
        instruction read();

        // Advance or rewind the tape to the given absolute offset in bytes.
        //
        // Precondition: the offset is in bounds.
        void seek(std::size_t offset);

    private:
        CodeIt it, begin, end;
    };
}

#include "tape.tpp"
