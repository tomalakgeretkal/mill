#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
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
    boost::asio::io_service& io_service,
    GlobalGetter&& get_global,
    GlobalSetter&& set_global
) {
    (void)get_global;

    set_global("std::io::writeln", make_subroutine<string>([&thread_pool] (string const& string) {
        std::cout << string.data() << '\n';
        return handle(unit());
    }));

    set_global("std::conc::sleep", make_subroutine([&thread_pool, &io_service] {
        boost::asio::deadline_timer timer(io_service);
        timer.expires_from_now(boost::posix_time::milliseconds(1000));

        auto& current_fiber = fiber::current();
        timer.async_wait([&] (boost::system::error_code ec) {
            (void)ec; // TODO: Handle error!
            thread_pool.resume(current_fiber);
        });
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
        auto& fiber = thread_pool.spawn([entry = std::move(entry)] () mutable {
            std::vector<handle> arguments;
            entry(arguments.begin(), arguments.end());
        });
        thread_pool.resume(fiber);
        return handle(unit());
    }));
}
