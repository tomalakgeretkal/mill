#include "interpret.hpp"
#include <stdexcept>

mill::interpret_eof::interpret_eof()
    : std::runtime_error("eof") { }
