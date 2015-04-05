#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include "data.hpp"
#include "fiber.hpp"
#include <iostream>
#include <memory>
#include "object.hpp"
#include <stdexcept>
#include <string>
#include "thread_pool.hpp"
#include <unordered_map>
#include <utility>
#include <vector>

using namespace mill;

int main(int argc, char const** argv) {
    assert(argc == 2);

    thread_pool* tp;

    std::unordered_map<std::string, handle> globals;
    globals.emplace("std::io::writeln", handle(subroutine([] (auto arguments_begin, auto) {
        std::cout << arguments_begin->template data<string>().data() << '\n';
        return handle(unit());
    })));
    globals.emplace("std::always::infix==", handle(subroutine([] (auto arguments_begin, auto) {
        auto a = (arguments_begin + 0)->template data<bool>();
        auto b = (arguments_begin + 1)->template data<bool>();
        return handle(a == b);
    })));
    globals.emplace("std::always::infix~", handle(subroutine([] (auto arguments_begin, auto) {
        auto const& a = (arguments_begin + 0)->template data<string>().data();
        auto const& b = (arguments_begin + 1)->template data<string>().data();
        return handle(string(a + b));
    })));
    globals.emplace("std::conc::spawn", handle(subroutine([&] (auto arguments_begin, auto) {
        auto entry = arguments_begin->template data<subroutine>();
        auto fiber = std::make_shared<mill::fiber>([entry = std::move(entry)] () mutable {
            std::vector<handle> arguments;
            entry(arguments.begin(), arguments.end());
        });
        tp->post([fiber = std::move(fiber)] {
            fiber->resume();
        });
        return handle(unit());
    })));

    search_path search_path;
    auto search_path_string = std::getenv("MILLPATH");
    if (search_path_string) {
        boost::algorithm::split(search_path, search_path_string, boost::is_any_of(":"));
    }

    auto get_global = [&] (std::string const& name) -> boost::optional<handle> {
        try {
            return globals.at(name);
        } catch (std::out_of_range const&) {
            return boost::none;
        }
    };
    auto set_global = [&] (std::string const& name, handle value) {
        globals.emplace(name, std::move(value));
    };
    if (std::strchr(argv[1], '/') || std::strchr(argv[1], '.')) {
        load_object(argv[1], get_global, set_global);
    } else {
        load_object(argv[1], search_path, get_global, set_global);
    }

    fiber fiber([&] {
        std::vector<handle> arguments;
        globals.at("main::MAIN").data<subroutine>()(arguments.begin(), arguments.end());
    });

    thread_pool tp_;
    tp = &tp_;
    tp->post([&] { fiber.resume(); });

    return 0;
}
