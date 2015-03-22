#include <algorithm>
#include <baka/io/file_descriptor.hpp>
#include <baka/io/file_stream.hpp>
#include <cstddef>
#include <fcntl.h>
#include "gc.hpp"
#include "interpreter.hpp"
#include <iostream>
#include "object.hpp"
#include <sys/types.h>
#include <sys/stat.h>
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
    baka::io::file_stream<decltype(objectFD), baka::io::operation::read> objectReader(std::move(objectFD));
    auto object = readObject(objectReader);

    VM vm;
    vm.loadObject(object);
    vm.setGlobal("std::io::writeln", vm.subroutine([&] (VM&, std::size_t, GCPtr* argv) {
        std::cout << vm.unstring(argv[0]) << '\n';
        return vm.unit();
    }));

    PrimitiveType<VM::CXXSubroutine>::instance().get(vm.global("main::MAIN"))
    .implementation
    ->operator()(vm, 0, nullptr);

    return 0;
}
