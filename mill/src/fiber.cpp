#include <boost/scope_exit.hpp>
#include <cassert>
#include "fiber.hpp"
#include <mutex>

namespace {
    __thread mill::fiber* volatile current_fiber = nullptr;
}

mill::fiber::status mill::fiber::resume() {
    assert(!current_fiber);
    std::lock_guard<decltype(mutex)> lock(mutex);
    current_fiber = this;
    BOOST_SCOPE_EXIT() {
        current_fiber = nullptr;
    };
    pull();
    return fiber_status;
}

void mill::fiber::pause() {
    assert(current_fiber);
    (*current_fiber->push)();
}

mill::fiber& mill::fiber::current() {
    assert(current_fiber);
    return *current_fiber;
}
