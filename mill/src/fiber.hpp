#pragma once
#include <boost/coroutine/all.hpp>
#include <mutex>

namespace mill {
    // A cheap thread of execution which can be resumed and paused without
    // system calls.
    class fiber {
    public:
        enum class status {
            not_finished,
            finished,
        };

        // Construct a fiber with the given callable, which is not invoked until
        // resume() is called.
        template<typename F>
        fiber(F entry);

        // Resume the fiber until it is paused. If the fiber is not paused,
        // this function will wait until it is and then resume the fiber
        // immediately.
        //
        // Precondition: no fiber is running on the current thread.
        status resume();

        // Pause the current fiber.
        //
        // Precondition: a fiber is running on the current thread.
        static void pause();

        // Return the current fiber.
        //
        // Precondition: a fiber is running on the current thread.
        static fiber& current();

    private:
        template<typename F>
        auto body(F entry);

        std::mutex mutex;
        boost::coroutines::asymmetric_coroutine<void>::pull_type pull;
        boost::coroutines::asymmetric_coroutine<void>::push_type* push;
        status fiber_status;
    };
}

#include "fiber.tpp"
