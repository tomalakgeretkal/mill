#include "../src/instruction.hpp"
#include "../src/tape.hpp"
#include <catch.hpp>
#include <vector>

using namespace mill;

TEST_CASE("tape::read throws bad_instruction", "[tape]") {
    auto test = [] (std::vector<unsigned char> const& code) {
        tape<decltype(code.begin())> tape(code.begin(), code.end());
        REQUIRE_THROWS_AS(
            tape.read(),
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

TEST_CASE("tape::read reads subsequent instructions", "[tape]") {
    std::vector<unsigned char> code{0x04, 0x09};
    tape<decltype(code.begin())> tape(code.begin(), code.end());
    REQUIRE(tape.read() == instruction(pop_instruction()));
    REQUIRE(tape.read() == instruction(swap_instruction()));
}
