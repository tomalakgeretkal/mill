#include <cstddef>
#include <thread>
#include "thread_pool.hpp"

mill::ThreadPool::ThreadPool()
    : ThreadPool(std::thread::hardware_concurrency()) { }

mill::ThreadPool::ThreadPool(std::size_t threadCount)
    : work(ioService) {
    if (threadCount == 0) {
        threadCount = 1;
    }
    for (decltype(threadCount) i = 0; i < threadCount; ++i) {
        threads.create_thread([=] {
            ioService.run();
        });
    }
}

mill::ThreadPool::~ThreadPool() {
    ioService.stop();
    threads.join_all();
}
