#pragma once
#include <atomic>
#include <boost/intrusive_ptr.hpp>
#include <boost/coroutine/all.hpp>
#include <boost/utility.hpp>
#include <future>
#include <memory>
#include "object.hpp"
#include <string>
#include "thread_pool.hpp"
#include <unordered_map>
#include <utility>
#include "value.hpp"
#include <vector>

namespace mill {
    class Fiber : boost::noncopyable {
    public:
        template<typename F>
        explicit Fiber(F f)
            : pull([this, f = std::move(f)] (decltype(*push)& push) mutable {
                this->push = &push;
                push();
                f();
            }) { }

        void resume() noexcept;

        static Fiber& current();
        static void yield();

    private:
        std::atomic<bool> running;
        boost::coroutines::asymmetric_coroutine<void>::pull_type pull;
        boost::coroutines::asymmetric_coroutine<void>::push_type* push;
    };

    class VM : boost::noncopyable {
    public:
        void loadObject(Object const& object);

        void setGlobal(std::string const&, boost::intrusive_ptr<Value>);
        boost::intrusive_ptr<Value> global(std::string const&) const;
        boost::intrusive_ptr<Value> global(Object const&, std::size_t) const;

        boost::intrusive_ptr<Value> string(Object const&, std::size_t) const;

        std::future<boost::intrusive_ptr<Value>> call(Value* value, std::size_t argc, Value** argv);

        void schedule(Fiber& fiber);

    private:
        template<typename F>
        void spawn(F f) {
            auto fiber = std::make_unique<Fiber>(std::move(f));
            auto fiberPtr = fiber.get();
            fibers.push_back(std::move(fiber));
            schedule(*fiberPtr);
        }

        std::unordered_map<Object const*, std::vector<boost::intrusive_ptr<Value>>> strings;
        std::unordered_map<std::string, boost::intrusive_ptr<Value>> globals;

        std::vector<std::unique_ptr<Fiber>> fibers;

        ThreadPool threadPool;
    };
}
