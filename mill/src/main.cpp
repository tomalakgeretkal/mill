#include <algorithm>
#include <baka/io/file_descriptor.hpp>
#include <baka/io/file_stream.hpp>
#include <cstddef>
#include <fcntl.h>
#include <future>
#include "interpreter.hpp"
#include <iostream>
#include <mutex>
#include "object.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>
#include "value.hpp"
#include <vector>
#include "vm.hpp"

using namespace mill;

int main(int argc, char const** argv) {
    if (argc != 2) {
        std::cerr << "usage: mill hello.millo\n";
        return 1;
    }

    baka::io::unique_fd objectFD(open(argv[1], O_RDONLY));
    baka::io::file_stream<baka::io::unique_fd, baka::io::operation::read> objectReader(std::move(objectFD));
    auto object = readObject(objectReader);

    VM vm;
    vm.loadObject(object);

    std::mutex coutMutex;
    vm.setGlobal("std::io::writeln", make<Subroutine>([&] (VM&, std::size_t, Value** argv) -> boost::intrusive_ptr<Value> {
        std::lock_guard<decltype(coutMutex)> lock(coutMutex);
        std::cout << dynamic_cast<String&>(*argv[0]).value << '\n';
        return &Unit::instance();
    }));
    vm.setGlobal("std::always::infix~", make<Subroutine>([&] (VM&, std::size_t, Value** argv) {
        auto& a = dynamic_cast<String&>(*argv[0]);
        auto& b = dynamic_cast<String&>(*argv[1]);
        return make<String>(a.value + b.value);
    }));
    vm.setGlobal("std::conc::yield", make<Subroutine>([] (VM& vm, std::size_t, Value**) {
        auto& fiber = Fiber::current();
        vm.schedule(fiber);
        Fiber::yield();
        return &Unit::instance();
    }));

    std::vector<std::future<boost::intrusive_ptr<Value>>> results;
    for (auto i = 0; i < 100; ++i) {
        results.push_back(vm.call(vm.global("main::MAIN").get(), 0, nullptr));
    }
    sleep(1);
    for (auto i = 0; i < 100; ++i) {
        results.push_back(vm.call(vm.global("main::MAIN").get(), 0, nullptr));
    }
    for (auto&& result : results) {
        result.get();
    }

    return 0;
}
