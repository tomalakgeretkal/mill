#include <algorithm>
#include <baka/io/file_descriptor.hpp>
#include <baka/io/file_stream.hpp>
#include <cstddef>
#include <fcntl.h>
#include "interpreter.hpp"
#include "disassemble.hpp"
#include <iostream>
#include <mutex>
#include "object.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>
#include "value.hpp"
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

    disassemble(object, std::cout);

    VM vm;

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
    vm.setGlobal("std::always::infix==", make<Subroutine>([&] (VM&, std::size_t, Value** argv) {
        auto& a = dynamic_cast<Boolean&>(*argv[0]);
        auto& b = dynamic_cast<Boolean&>(*argv[1]);
        return make<Boolean>(a.value == b.value);
    }));

    vm.loadObject(object);

    vm.call(vm.global("main::MAIN").get(), 0, nullptr).get();

    return 0;
}
