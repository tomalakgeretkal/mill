#include "../src/interpreter.hpp"
#include "../src/object.hpp"
#include "../src/vm.hpp"
#include <baka/io/memory_stream.hpp>
#include <catch.hpp>

TEST_CASE("interpreter works", "[Interpreter]") {
    auto interpret = [] (auto& vm, auto& object, std::vector<char> const& data) {
        baka::io::memory_stream source;
        source.write(data.data(), data.data() + data.size());
        source.seek_begin(0);
        return mill::interpret(vm, object, source);
    };

    mill::Object object;
    object.strings = { "foo", "bar" };

    {
        mill::VM vm;
        vm.loadObject(object);
        REQUIRE(vm.unit() == interpret(vm, object, {
            0x06,
            0x07, 0x00,
            0x07, 0x01,
            0x04,
            0x04,
            0x05,
        }));
    }

    {
        mill::VM vm;
        vm.loadObject(object);
        REQUIRE(vm.true_() == interpret(vm, object, { 0x07, 0x01, 0x05 }));
        REQUIRE(vm.false_() == interpret(vm, object, { 0x07, 0x00, 0x05 }));
    }

    {
        mill::VM vm;
        vm.loadObject(object);
        REQUIRE(vm.unstring(interpret(vm, object, { 0x02, 0x00, 0x00, 0x00, 0x00, 0x05 })) == "foo");
        REQUIRE(vm.unstring(interpret(vm, object, { 0x02, 0x01, 0x00, 0x00, 0x00, 0x05 })) == "bar");
    }
}
