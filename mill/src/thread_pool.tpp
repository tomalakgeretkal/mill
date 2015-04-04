#include "thread_pool.hpp"
#include <utility>

template<typename F>
void mill::thread_pool::post(F function) {
    io_service.post(std::move(function));
}
