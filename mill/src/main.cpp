#include <boost/optional.hpp>
#include <cassert>
#include "data.hpp"
#include "fiber.hpp"
#include <fstream>
#include "interpret.hpp"
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include "tape.hpp"
#include "thread_pool.hpp"
#include <unordered_map>
#include <vector>
#include "object.hpp"

using namespace mill;

int main(int argc, char const** argv) {
    assert(argc == 2);

    thread_pool* tp;

    std::ifstream object_file_stream(argv[1], std::ios::binary);
    std::vector<unsigned char> object_file(
        (std::istreambuf_iterator<char>(object_file_stream)),
        std::istreambuf_iterator<char>()
    );
    auto object = read_object(object_file.begin(), object_file.end());

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

    for (auto&& subroutine : object.subroutines) {
        globals.emplace(
            object.strings[object.name] + "::" + object.strings[subroutine.name],
            handle(mill::subroutine([&] (auto arguments_begin, auto arguments_end) {
                auto& code = subroutine.body;
                tape<decltype(code.begin())> tape(code.begin(), code.end());
                return interpret(
                    tape,
                    arguments_begin,
                    arguments_end,
                    [&] (auto index) -> boost::optional<handle> {
                        try {
                            return globals.at(object.strings.at(index));
                        } catch (std::out_of_range const&) {
                            return boost::none;
                        }
                    },
                    [&] (auto index) -> boost::optional<handle> {
                        try {
                            return handle(string(object.strings.at(index)));
                        } catch (std::out_of_range const&) {
                            return boost::none;
                        }
                    }
                );
            }))
        );
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
