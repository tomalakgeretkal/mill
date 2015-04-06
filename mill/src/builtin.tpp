#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "builtin.hpp"
#include <cstddef>
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

#include "builtin/always.inc"
#include "builtin/conc.inc"
#include "builtin/io.inc"
}
