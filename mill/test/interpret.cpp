#include "../src/data.hpp"
#include "../src/interpret.hpp"
#include "../src/tape.hpp"
#include <boost/optional.hpp>
#include <catch.hpp>
#include <vector>

using namespace mill;

TEST_CASE("interpret should push Booleans", "[interpret]") {
    auto test = [] (std::vector<unsigned char> const& code, bool expectation) {
        tape<decltype(code.begin())> tape(code.begin(), code.end());
        std::vector<handle> arguments;
        auto result = interpret(
            tape,
            arguments.begin(),
            arguments.end(),
            [] (auto) { return boost::none; },
            [] (auto) { return boost::none; }
        );
        REQUIRE(result.data<bool>() == expectation);
    };
    test({0x07, 0x01, 0x05}, true);
    test({0x07, 0x00, 0x05}, false);
}
