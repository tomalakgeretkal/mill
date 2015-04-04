#include <cassert>
#include "data.hpp"
#include <utility>

template<typename T>
mill::handle::handle(T value)
    : ptr(std::make_shared<derived<T>>(std::move(value))) { }

template<typename T>
T& mill::handle::data() const {
    assert(ptr);
    return dynamic_cast<derived<T>&>(*ptr).value;
}

template<typename T>
mill::handle::derived<T>::derived(T value) : value(std::move(value)) { }
