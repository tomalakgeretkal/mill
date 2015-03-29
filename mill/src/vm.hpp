#pragma once
#include <boost/intrusive_ptr.hpp>
#include <boost/utility.hpp>
#include <future>
#include "object.hpp"
#include <string>
#include "thread_pool.hpp"
#include <unordered_map>
#include "value.hpp"
#include <vector>

namespace mill {
    class VM : boost::noncopyable {
    public:
        void loadObject(Object const& object);

        void setGlobal(std::string const&, boost::intrusive_ptr<Value>);
        boost::intrusive_ptr<Value> global(std::string const&) const;
        boost::intrusive_ptr<Value> global(Object const&, std::size_t) const;

        boost::intrusive_ptr<Value> string(Object const&, std::size_t) const;

        std::future<boost::intrusive_ptr<Value>> call(Value* value, std::size_t argc, Value** argv);

    private:
        std::unordered_map<Object const*, std::vector<boost::intrusive_ptr<Value>>> strings;
        std::unordered_map<std::string, boost::intrusive_ptr<Value>> globals;

        ThreadPool threadPool;
    };
}
