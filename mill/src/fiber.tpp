#include "fiber.hpp"
#include <memory>
#include <utility>

template<typename F>
mill::fiber::fiber(F entry)
    : pull(body(entry))
    , fiber_status(status::not_finished) { }

template<typename F>
auto mill::fiber::body(F entry) {
    return [this, entry = std::move(entry)] (decltype(*push)& sink) mutable {
        push = &sink;
        sink();
        entry();
        fiber_status = status::finished;
    };
}
