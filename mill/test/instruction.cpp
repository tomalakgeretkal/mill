#include "../src/instruction.hpp"
#include <catch.hpp>
#include <vector>

using namespace mill;

TEST_CASE("instruction throws bad_instruction", "[interpret]") {
    auto test = [] (std::vector<unsigned char> const& code) {
        REQUIRE_THROWS_AS(
            decode_instruction(code.begin(), code.end()),
            bad_instruction
        );
    };
    test({ });
    test({ 0x00 });
    test({ 0xFF });
    test({ 0x01 });
    test({ 0x01, 0x00 });
    test({ 0x01, 0x00, 0x00 });
    test({ 0x01, 0x00, 0x00, 0x00 });
}
