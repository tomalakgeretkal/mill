#pragma once
#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/thread.hpp>
#include <cstddef>
#include "fiber.hpp"
#include <memory>
#include <tbb/concurrent_hash_map.h>

namespace mill {
    // A pool of threads on which fibers can be resumed.
    class thread_pool {
    public:
        // Construct the thread pool with an optimal number of threads.
        thread_pool();

        // Construct the thread pool with a custom number of threads.
        explicit thread_pool(std::size_t threadCount);

        // Wait until all fibers have finished and stop the thread pool.
        ~thread_pool();

        // Create a fiber owned by this thread pool.
        template<typename F>
        fiber& spawn(F entry);

        // Enqueue a fiber for resuming.
        void resume(fiber& fiber);

    private:
        boost::asio::io_service io_service;
        boost::optional<boost::asio::io_service::work> work;
        tbb::concurrent_hash_map<
            fiber*,
            boost::asio::io_service::work
        > fibers;
        boost::thread_group threads;
    };
}

#include "thread_pool.tpp"
