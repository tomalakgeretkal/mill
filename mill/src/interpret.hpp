#pragma once
#include "data.hpp"
#include "tape.hpp"

namespace mill {
    // Interpret Mill instructions from a tape.
    //
    // - arguments_begin and arguments_end must be a random access
    //   iterators to handles.
    // - get_global must be a callable which when given an index in the
    //   string pool as an std::size_t, returns the global associated with that
    //   name as a boost::optional<handle>.
    // - get_string must be a callable which when given an index in the
    //   string pool as an std::size_t, returns that string as a
    //   boost::optional<handle>.
    template<
        typename CodeIt,
        typename ArgumentIt,
        typename GlobalGetter,
        typename StringGetter
    >
    handle interpret(
        tape<CodeIt> tape,
        ArgumentIt arguments_begin,
        ArgumentIt arguments_end,
        GlobalGetter&& get_global,
        StringGetter&& get_string
    );
}

#include "interpret.tpp"
