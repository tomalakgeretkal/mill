#include "../src/interpreter.hpp"
#include "../src/vm.hpp"
#include <baka/io/memory_stream.hpp>
#include <catch.hpp>

TEST_CASE("interpreter works", "[Interpreter]") {
    std::vector<char> data{
        0x06,
        0x07, 0x00,
        0x07, 0x01,
        0x04,
        0x04,
        0x05,
    };

    baka::io::memory_stream source;
    source.write(data.data(), data.data() + data.size());
    source.seek_begin(0);

    mill::VM vm;
    REQUIRE(mill::interpret(vm, source) == vm.unit());
}
