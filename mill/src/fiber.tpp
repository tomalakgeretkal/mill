#include "fiber.hpp"
#include <memory>
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
        self.reset();
    };
}

template<typename F>
std::shared_ptr<mill::fiber> mill::make_fiber(F entry) {
    std::shared_ptr<fiber> shared_ptr(new fiber(std::move(entry)));
    shared_ptr->self = shared_ptr;
    return shared_ptr;
}
