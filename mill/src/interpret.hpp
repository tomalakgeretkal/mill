#pragma once
#include <cstddef>
#include "handle.hpp"
#include <stdexcept>

namespace mill {
    // Thrown when the interpreter encounters EOF.
    class interpret_eof : public std::runtime_error {
    public:
        interpret_eof();
    };

    // Thrown when the interpreter encounters an invalid instruction.
    class bad_instruction : public std::runtime_error {
    public:
        // Construct the exception with the offset of the invalid instruction.
        explicit bad_instruction(std::size_t offset);
    };

    // Interpret bytecode given a range of arguments, a function to retrieve a
    // global given the index of its name in the constant pool and a function
    // to retrieve a string given its index in the constant pool.
    //
    // - code_begin and code_end must be random access iterators.
    // - arguments_begin and arguments_end must be random access iterators.
    // - get_global must take an std::size_t and return a handle or throw a
    //   no_such_global exception.
    // - get_string must take an std::size_t and return a handle.
    template<
        typename CodeIt,
        typename ArgumentsIt,
        typename GetGlobal,
        typename GetString
    >
    handle interpret(
        CodeIt code_begin, CodeIt code_end,
        ArgumentsIt arguments_begin, ArgumentsIt arguments_end,
        GetGlobal&& get_global,
        GetString&& get_string
    );
}

#include "interpret.tpp"
