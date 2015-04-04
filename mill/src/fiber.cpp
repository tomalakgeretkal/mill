#include <boost/scope_exit.hpp>
#include <cassert>
#include "fiber.hpp"
#include <mutex>

namespace {
    __thread mill::fiber* volatile current_fiber = nullptr;
}

void mill::fiber::resume() {
    std::lock_guard<decltype(mutex)> lock(mutex);
    current_fiber = this;
    BOOST_SCOPE_EXIT() {
        current_fiber = nullptr;
    };
    pull();
}

void mill::fiber::pause() {
    assert(current_fiber);
    (*current_fiber->push)();
}

mill::fiber& mill::fiber::current() {
    assert(current_fiber);
    return *current_fiber;
}
