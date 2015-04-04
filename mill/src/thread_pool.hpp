#pragma once
#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/thread.hpp>
#include <cstddef>

namespace mill {
    class thread_pool {
    public:
        thread_pool();

        explicit thread_pool(std::size_t threadCount);

        ~thread_pool();

        template<typename F>
        void post(F function);

    private:
        boost::thread_group threads;
        boost::asio::io_service io_service;
        boost::optional<boost::asio::io_service::work> work;
    };
}

#include "thread_pool.tpp"
