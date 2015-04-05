#include <iostream>
#include <cassert>
#include "data.hpp"
#include <utility>
#include "utility.hpp"

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

template<typename F>
mill::subroutine::subroutine(F implementation)
    : implementation(std::move(implementation)) { }

template<typename ArgumentIt>
mill::handle mill::subroutine::operator()(ArgumentIt arguments_begin, ArgumentIt arguments_end) {
    return implementation(
        argument_iterator(std::move(arguments_begin)),
        argument_iterator(std::move(arguments_end))
    );
}

template<typename... Args, typename F>
mill::handle mill::make_subroutine(F impl) {
    return handle(subroutine(
        [impl = std::move(impl)] (auto arguments_begin, auto arguments_end) {
            auto argument_count = std::distance(arguments_begin, arguments_end);
            if (static_cast<std::size_t>(argument_count) != sizeof...(Args)) {
                throw "invalid number of arguments passed";
            }
            call<handle> call{impl, arguments_begin++->template data<Args>()...};
            return call.result;
        }
    ));
}
