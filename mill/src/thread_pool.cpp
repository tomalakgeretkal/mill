#include <cstddef>
#include <thread>
#include "thread_pool.hpp"

mill::thread_pool::thread_pool()
    : thread_pool(std::thread::hardware_concurrency()) { }

mill::thread_pool::thread_pool(std::size_t thread_count)
    : work(io_service) {
    if (thread_count == 0) {
        thread_count = 1;
    }
    for (decltype(thread_count) i = 0; i < thread_count; ++i) {
        threads.create_thread([=] {
            io_service.run();
        });
    }
}

mill::thread_pool::~thread_pool() {
    work.reset();
    threads.join_all();
}

void mill::thread_pool::resume(fiber& fiber) {
    assert(fibers.count(&fiber));
    io_service.post([&] {
        if (fiber.resume() == fiber::status::finished) {
            fibers.erase(&fiber);
            // FIXME: Shitty std::unique_ptr doesn't work as map key so fix this memory leak.
            delete &fiber;
        }
    });
}
