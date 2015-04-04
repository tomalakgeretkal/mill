#include "fiber.hpp"
#include <utility>

template<typename F>
mill::fiber::fiber(F entry)
    : pull(body(entry)) { }

template<typename F>
auto mill::fiber::body(F entry) {
    return [this, entry = std::move(entry)] (decltype(*push)& sink) mutable {
        push = &sink;
        sink();
        entry();
    };
}
