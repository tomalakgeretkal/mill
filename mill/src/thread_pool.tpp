#include <boost/asio.hpp>
#include <memory>
#include "thread_pool.hpp"
#include <utility>

template<typename F>
mill::fiber& mill::thread_pool::spawn(F entry) {
    // FIXME: Shitty std::unique_ptr doesn't work as map key so fix this memory leak.
    auto fiber = new mill::fiber(std::move(entry));
    fibers.insert({fiber, boost::asio::io_service::work(io_service)});
    return *fiber;
}
