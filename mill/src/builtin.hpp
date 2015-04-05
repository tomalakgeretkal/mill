#pragma once
#include "thread_pool.hpp"

namespace mill {
    // Load built-in entities.
    //
    // - get_global is a callable that takes a string and returns an optional
    //   handle.
    // - set_global is a callable that takes a string and a handle and returns
    //   void.
    template<typename GlobalGetter, typename GlobalSetter>
    void load_builtins(
        thread_pool& thread_pool,
        GlobalGetter&& get_global,
        GlobalSetter&& set_global
    );
}

#include "builtin.tpp"
