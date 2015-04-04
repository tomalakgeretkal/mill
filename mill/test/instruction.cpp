#include "../src/instruction.hpp"
#include <boost/variant.hpp>
#include <catch.hpp>
#include <vector>

using namespace mill;

TEST_CASE("decode_instruction throws bad_instruction", "[instruction]") {
    auto test = [] (std::vector<unsigned char> const& code) {
        auto begin = code.begin();
        REQUIRE_THROWS_AS(
            decode_instruction(begin, code.end()),
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

TEST_CASE("decode_instruction returns the decoded instruction", "[instruction]") {
    std::vector<unsigned char> code{ 0x01, 0x01, 0x02, 0x03, 0x04 };
    auto begin = code.begin();
    auto instruction = decode_instruction(begin, code.end());
    REQUIRE(boost::get<push_global_instruction>(instruction).op0 == 0x04030201);
}
