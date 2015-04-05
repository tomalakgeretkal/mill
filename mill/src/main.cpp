#include <boost/algorithm/string.hpp>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include "data.hpp"
#include "builtin.hpp"
#include "fiber.hpp"
#include "object.hpp"
#include <stdexcept>
#include <string>
#include <tbb/concurrent_unordered_map.h>
#include "thread_pool.hpp"
#include <utility>
#include <vector>

using namespace mill;

int main(int argc, char const** argv) {
    assert(argc == 2);

    tbb::concurrent_unordered_map<std::string, handle> globals;

    auto get_global = [&] (std::string const& name) -> boost::optional<handle> {
        try {
            return globals.at(name);
        } catch (std::out_of_range const&) {
            return boost::none;
        }
    };

    auto set_global = [&] (std::string const& name, handle value) {
        globals.insert({name, std::move(value)});
    };

    search_path search_path;
    auto search_path_string = std::getenv("MILLPATH");
    if (search_path_string) {
        boost::algorithm::split(search_path, search_path_string, boost::is_any_of(":"));
    }

    if (std::strchr(argv[1], '/') || std::strchr(argv[1], '.')) {
        load_object(argv[1], get_global, set_global);
    } else {
        load_object(argv[1], search_path, get_global, set_global);
    }

    auto main_fiber = make_fiber([&] {
        std::vector<handle> arguments;
        globals.at("main::MAIN").data<subroutine>()(arguments.begin(), arguments.end());
    });

    thread_pool thread_pool;
    load_builtins(thread_pool, get_global, set_global);
    thread_pool.post([main_fiber = std::move(main_fiber)] { main_fiber->resume(); });

    return 0;
}
