#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "builtin.hpp"
#include "data.hpp"
#include "fiber.hpp"
#include "thread_pool.hpp"
#include "utility.hpp"
#include <iostream>
#include <vector>
#include <utility>
#include <cstddef>

template<typename GlobalGetter, typename GlobalSetter>
void mill::load_builtins(
    thread_pool& thread_pool,
    boost::asio::io_service& io_service,
    GlobalGetter&& get_global,
    GlobalSetter&& set_global
) {
    (void)get_global;

#include "builtin/always.ipp"
#include "builtin/conc.ipp"
#include "builtin/io.ipp"
}
