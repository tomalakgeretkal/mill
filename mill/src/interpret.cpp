#include <cstddef>
#include "interpret.hpp"
#include <stdexcept>
#include <string>

mill::interpret_eof::interpret_eof()
    : std::runtime_error("eof") { }

mill::bad_instruction::bad_instruction(std::size_t offset)
    : std::runtime_error("invalid instruction at offset " + std::to_string(offset)) { }
