#include "builtin.hpp"
#include "data.hpp"
#include "fiber.hpp"
#include <iostream>
#include "thread_pool.hpp"
#include <utility>
#include "utility.hpp"
#include <vector>

template<typename GlobalGetter, typename GlobalSetter>
void mill::load_builtins(
    thread_pool& thread_pool,
    GlobalGetter&& get_global,
    GlobalSetter&& set_global
) {
    (void)get_global;

    set_global("std::io::writeln", make_subroutine<string>([&thread_pool] (string const& string) {
        std::cout << string.data() << '\n';
        auto current = fiber::current();
        thread_pool.post([current = std::move(current)] { current->resume(); });
        fiber::pause();
        return handle(unit());
    }));

    set_global("std::always::infix==", make_subroutine<bool, bool>([] (bool a, bool b) {
        return handle(a == b);
    }));

    set_global("std::always::infix~", make_subroutine<string, string>([] (string const& a, string const& b) {
        return handle(string(a.data() + b.data()));
    }));

    set_global("std::conc::spawn", make_subroutine<subroutine>([&thread_pool] (subroutine const& entry) {
        auto fiber = make_fiber([entry = std::move(entry)] () mutable {
            std::vector<handle> arguments;
            entry(arguments.begin(), arguments.end());
        });
        thread_pool.post([fiber = std::move(fiber)] {
            fiber->resume();
        });
        return handle(unit());
    }));
}
