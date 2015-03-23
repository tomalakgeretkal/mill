#pragma once
#include <boost/intrusive_ptr.hpp>
#include <boost/utility.hpp>
#include "object.hpp"
#include <string>
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

    private:
        std::unordered_map<Object const*, std::vector<boost::intrusive_ptr<Value>>> strings;
        std::unordered_map<std::string, boost::intrusive_ptr<Value>> globals;
    };
}
