#include "../src/interpreter.hpp"
#include "../src/object.hpp"
#include "../src/vm.hpp"
#include <algorithm>
#include <baka/io/memory_stream.hpp>
#include <boost/intrusive_ptr.hpp>
#include <cstddef>
#include <catch.hpp>
#include <iterator>
#include <vector>

using namespace mill;

TEST_CASE("interpreter works", "[Interpreter]") {
    auto interpret = [] (auto& vm, auto& object, std::vector<char> const& data) {
        baka::io::memory_stream source;
        source.write(data.data(), data.data() + data.size());
        source.seek_begin(0);
        return mill::interpret(vm, object, source);
    };

    Object object;
    object.strings = { "foo", "bar" };

    {
        VM vm;
        vm.loadObject(object);
        REQUIRE(dynamic_cast<Unit*>(interpret(vm, object, {
            0x06,
            0x07, 0x00,
            0x07, 0x01,
            0x04,
            0x04,
            0x05,
        }).get()));
    }

    {
        VM vm;
        vm.loadObject(object);
        REQUIRE(dynamic_cast<Boolean&>(*interpret(vm, object, { 0x07, 0x01, 0x05 })).value);
        REQUIRE(!dynamic_cast<Boolean&>(*interpret(vm, object, { 0x07, 0x00, 0x05 })).value);
    }

    {
        VM vm;
        vm.loadObject(object);
        REQUIRE(dynamic_cast<String&>(*interpret(vm, object, { 0x02, 0x00, 0x00, 0x00, 0x00, 0x05 })).value == "foo");
        REQUIRE(dynamic_cast<String&>(*interpret(vm, object, { 0x02, 0x01, 0x00, 0x00, 0x00, 0x05 })).value == "bar");
    }

    {
        VM vm;
        std::size_t argc;
        std::vector<boost::intrusive_ptr<Value>> argv;
        vm.setGlobal("foo", make<Subroutine>([&] (VM&, std::size_t argc_, boost::intrusive_ptr<Value>* argv_) {
            argc = argc_;
            std::copy(argv_, argv_ + argc_, std::back_inserter(argv));
            return make<Unit>();
        }));
        vm.loadObject(object);
        REQUIRE(dynamic_cast<Unit*>(interpret(vm, object, {
            0x01, 0x00, 0x00, 0x00, 0x00,
            0x06,
            0x07, 0x01,
            0x03, 0x02, 0x00, 0x00, 0x00,
            0x05,
        }).get()));
        REQUIRE(argc == 2);
        REQUIRE(dynamic_cast<Unit*>(argv[0].get()));
        REQUIRE(dynamic_cast<Boolean&>(*argv[1]).value);
    }
}
