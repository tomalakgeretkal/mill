#pragma once
#include <boost/coroutine/all.hpp>
#include <memory>
#include <mutex>

namespace mill {
    // A cheap thread of execution which can be resumed and paused without
    // system calls.
    class fiber {
    public:
        // Resume the fiber until it is paused. If the fiber is not paused,
        // this function will wait until it is and then resume the fiber
        // immediately.
        //
        // Precondition: no fiber is running on the current thread.
        void resume();

        // Pause the current fiber.
        //
        // Precondition: a fiber is running on the current thread.
        static void pause();

        // Return the current fiber.
        //
        // Precondition: a fiber is running on the current thread.
        static std::shared_ptr<fiber> current();

    private:
        template<typename F>
        fiber(F entry);

        template<typename F>
        auto body(F entry);

        std::mutex mutex;
        boost::coroutines::asymmetric_coroutine<void>::pull_type pull;
        boost::coroutines::asymmetric_coroutine<void>::push_type* push;

        std::shared_ptr<fiber> self;

        template<typename F>
        friend std::shared_ptr<fiber> make_fiber(F entry);
    };

    // Construct a fiber with the given callable, which is not invoked until
    // resume() is called.
    template<typename F>
    std::shared_ptr<fiber> make_fiber(F entry);
}

#include "fiber.tpp"
