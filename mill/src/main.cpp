#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/optional.hpp>
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
#include <thread>
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

    boost::asio::io_service io_service;
    boost::optional<boost::asio::io_service::work> work(io_service);

    std::thread io_thread([&] {
        io_service.run();
    });

    {
        thread_pool thread_pool;

        load_builtins(thread_pool, io_service, get_global, set_global);

        auto& main_fiber = thread_pool.spawn([&] {
            std::vector<handle> arguments;
            globals.at("main::MAIN").data<subroutine>()(arguments.begin(), arguments.end());
        });

        thread_pool.resume(main_fiber);
    }

    work.reset();
    io_thread.join();

    return 0;
}
