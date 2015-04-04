#include "../src/handle.hpp"
#include "../src/interpret.hpp"
#include <catch.hpp>
#include <vector>

using namespace mill;

TEST_CASE("interpreter throws interpret_eof", "[interpret]") {
    std::vector<unsigned char> code;
    std::vector<handle> arguments;

    REQUIRE_THROWS_AS(
        interpret(
            code.begin(), code.end(),
            arguments.begin(), arguments.end(),
            [] (auto) { return handle(); },
            [] (auto) { return handle(); }
        ),
        interpret_eof
    );
}
