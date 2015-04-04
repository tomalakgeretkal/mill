#include "handle.hpp"
#include "interpret.hpp"

template<
    typename CodeIt,
    typename ArgumentsIt,
    typename GetGlobal,
    typename GetString
>
mill::handle mill::interpret(
    CodeIt code_begin, CodeIt code_end,
    ArgumentsIt arguments_begin, ArgumentsIt arguments_end,
    GetGlobal&& get_global,
    GetString&& get_string
) {
    if (code_begin == code_end) {
        throw interpret_eof();
    }
    throw bad_instruction(0);
}
