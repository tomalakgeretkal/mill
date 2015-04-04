#include "../src/data.hpp"
#include "../src/interpret.hpp"
#include "../src/tape.hpp"
#include <boost/optional.hpp>
#include <catch.hpp>
#include <iterator>
#include <vector>

using namespace mill;

namespace {
    template<typename Code, typename Arguments = std::vector<handle>>
    handle interpret(Code const& code, Arguments const& arguments = Arguments()) {
        tape<decltype(code.begin())> tape(code.begin(), code.end());
        return interpret(
            tape,
            arguments.begin(),
            arguments.end(),
            [] (auto) -> boost::optional<handle> { return boost::none; },
            [] (auto) -> boost::optional<handle> { return boost::none; }
        );
    }
}

TEST_CASE("interpret should push unit", "[interpret]") {
    std::vector<unsigned char> code{0x06, 0x05};
    REQUIRE_NOTHROW(interpret(code).data<unit>());
}

TEST_CASE("interpret should push Booleans", "[interpret]") {
    auto test = [] (std::vector<unsigned char> const& code, bool expectation) {
        REQUIRE(interpret(code).data<bool>() == expectation);
    };
    test({0x07, 0x01, 0x05}, true);
    test({0x07, 0x00, 0x05}, false);
}

TEST_CASE("interpret should push parameters", "[interpret]") {
    std::vector<unsigned char> code{
        0x08, 0x01, 0x00, 0x00, 0x00,
        0x05,
    };
    REQUIRE_NOTHROW(interpret(code, {handle(true), handle(unit())}).data<unit>());
}

TEST_CASE("interpret should pop", "[interpret]") {
    std::vector<unsigned char> code{0x06, 0x07, 0x01, 0x04, 0x05};
    REQUIRE_NOTHROW(interpret(code).data<unit>());
}

TEST_CASE("interpret should swap", "[interpret]") {
    std::vector<unsigned char> code{0x06, 0x07, 0x01, 0x09, 0x05};
    REQUIRE_NOTHROW(interpret(code).data<unit>());
}

TEST_CASE("interpret should jump conditionally", "[interpret]") {
    std::vector<unsigned char> code{
        0x07, 0x00,
        0x0A, 0x08, 0x00, 0x00, 0x00,
        0x06,
        0x07, 0x01,
        0x0A, 0x11, 0x00, 0x00, 0x00,
        0x07, 0x01,
        0x05,
    };
    REQUIRE_NOTHROW(interpret(code).data<unit>());
}

TEST_CASE("interpret should jump unconditionally", "[interpret]") {
    std::vector<unsigned char> code{
        0x06,
        0x0B, 0x08, 0x00, 0x00, 0x00,
        0x07, 0x01,
        0x05,
    };
    REQUIRE_NOTHROW(interpret(code).data<unit>());
}

TEST_CASE("interpret should push globals", "[interpret]") {
    std::vector<unsigned char> code{
        0x01, 0x04, 0x00, 0x00, 0x00,
        0x05,
    };
    tape<decltype(code.begin())> tape(code.begin(), code.end());
    std::vector<handle> arguments;
    auto result = interpret(
        tape,
        arguments.begin(),
        arguments.end(),
        [] (auto index) -> boost::optional<handle> {
            REQUIRE(index == 4);
            return handle(unit());
        },
        [] (auto) -> boost::optional<handle> { return boost::none; }
    );
    REQUIRE_NOTHROW(result.data<unit>());
}

TEST_CASE("interpret should push strings", "[interpret]") {
    std::vector<unsigned char> code{
        0x02, 0x04, 0x00, 0x00, 0x00,
        0x05,
    };
    tape<decltype(code.begin())> tape(code.begin(), code.end());
    std::vector<handle> arguments;
    auto result = interpret(
        tape,
        arguments.begin(),
        arguments.end(),
        [] (auto) -> boost::optional<handle> { return boost::none; },
        [] (auto index) -> boost::optional<handle> {
            REQUIRE(index == 4);
            return handle(string());
        }
    );
    REQUIRE_NOTHROW(result.data<string>());
}

TEST_CASE("interpret should call subroutines", "[interpret]") {
    auto called = false;
    auto callee = handle(subroutine([&] (auto arguments_begin, auto arguments_end) {
        REQUIRE(std::distance(arguments_begin, arguments_end) == 2);
        REQUIRE((arguments_begin + 0)->template data<bool>() == true);
        REQUIRE((arguments_begin + 1)->template data<bool>() == false);
        called = true;
        return handle(unit());
    }));

    std::vector<unsigned char> code{
        0x01, 0x00, 0x00, 0x00, 0x00,
        0x07, 0x01,
        0x07, 0x00,
        0x03, 0x02, 0x00, 0x00, 0x00,
        0x05,
    };
    tape<decltype(code.begin())> tape(code.begin(), code.end());
    std::vector<handle> arguments;
    auto result = interpret(
        tape,
        arguments.begin(),
        arguments.end(),
        [&] (auto) -> boost::optional<handle> { return callee; },
        [] (auto) -> boost::optional<handle> { return boost::none; }
    );
    REQUIRE_NOTHROW(result.data<unit>());
    REQUIRE(called);
}
