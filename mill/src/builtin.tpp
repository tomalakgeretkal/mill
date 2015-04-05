#include "builtin.hpp"
#include "data.hpp"
#include "fiber.hpp"
#include <iostream>
#include <memory>
#include "thread_pool.hpp"
#include <utility>
#include <vector>

template<typename GlobalGetter, typename GlobalSetter>
void mill::load_builtins(
    thread_pool& thread_pool,
    GlobalGetter&& get_global,
    GlobalSetter&& set_global
) {
    (void)get_global;

    set_global("std::io::writeln", handle(subroutine([] (auto arguments_begin, auto) {
        std::cout << arguments_begin->template data<string>().data() << '\n';
        return handle(unit());
    })));

    set_global("std::always::infix==", handle(subroutine([] (auto arguments_begin, auto) {
        auto a = (arguments_begin + 0)->template data<bool>();
        auto b = (arguments_begin + 1)->template data<bool>();
        return handle(a == b);
    })));

    set_global("std::always::infix~", handle(subroutine([] (auto arguments_begin, auto) {
        auto const& a = (arguments_begin + 0)->template data<string>().data();
        auto const& b = (arguments_begin + 1)->template data<string>().data();
        return handle(string(a + b));
    })));

    set_global("std::conc::spawn", handle(subroutine([&thread_pool] (auto arguments_begin, auto) {
        auto entry = arguments_begin->template data<subroutine>();
        auto fiber = std::make_shared<mill::fiber>([entry = std::move(entry)] () mutable {
            std::vector<handle> arguments;
            entry(arguments.begin(), arguments.end());
        });
        thread_pool.post([fiber = std::move(fiber)] {
            fiber->resume();
        });
        return handle(unit());
    })));
}
