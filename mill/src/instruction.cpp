#include <cstddef>
#include "instruction.hpp"
#include <stdexcept>
#include <string>

mill::bad_instruction::bad_instruction(std::size_t offset)
    : std::runtime_error("invalid instruction at offset " + std::to_string(offset)) { }
