#include "data.hpp"
#include <string>
#include <utility>

mill::handle::base::~base() = default;

mill::string::string(std::string data)
    : data_(std::move(data)) { }

std::string const& mill::string::data() const {
    return data_;
}
