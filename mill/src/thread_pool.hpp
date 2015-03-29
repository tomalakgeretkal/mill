#pragma once
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <cstddef>
#include <memory>
#include <utility>

namespace mill {
    class ThreadPool {
    public:
        ThreadPool();

        explicit ThreadPool(std::size_t threadCount);

        ~ThreadPool();

        template<typename F>
        void post(F function) {
            // FIXME: ioService.post creates a fucking copy of the
            // function we pass in.
            auto fuck = std::make_shared<F>(std::move(function));
            ioService.post([=] { return (*fuck)(); });
        }

    private:
        boost::thread_group threads;
        boost::asio::io_service ioService;
        boost::asio::io_service::work work;
    };
}
