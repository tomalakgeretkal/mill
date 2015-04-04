#include <boost/scope_exit.hpp>
#include <cassert>
#include "fiber.hpp"

namespace {
    __thread mill::fiber volatile* current_fiber = nullptr;
}

void mill::fiber::resume() {
    assert(!current_fiber);
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
