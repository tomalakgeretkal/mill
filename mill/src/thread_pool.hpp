#pragma once
#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/thread.hpp>
#include <cstddef>

namespace mill {
    // A pool of threads to which tasks can be submitted.
    class thread_pool {
    public:
        // Construct the thread pool with an optimal number of threads.
        thread_pool();

        // Construct the thread pool with a custom number of threads.
        explicit thread_pool(std::size_t threadCount);

        // Wait until all tasks have finished and stop the thread pool.
        ~thread_pool();

        // Enqueue a task.
        template<typename F>
        void post(F function);

    private:
        boost::thread_group threads;
        boost::asio::io_service io_service;
        boost::optional<boost::asio::io_service::work> work;
    };
}

#include "thread_pool.tpp"
