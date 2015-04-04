#include "../src/data.hpp"
#include "../src/interpret.hpp"
#include "../src/tape.hpp"
#include <boost/optional.hpp>
#include <catch.hpp>
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
            [] (auto) { return boost::none; },
            [] (auto) { return boost::none; }
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

TEST_CASE("interpret should jump unconditionally", "[interpret]") {
    std::vector<unsigned char> code{
        0x06,
        0x0B, 0x08, 0x00, 0x00, 0x00,
        0x07, 0x01,
        0x05,
    };
    REQUIRE_NOTHROW(interpret(code).data<unit>());
}
