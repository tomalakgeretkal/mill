#include "../src/fiber.hpp"
#include <catch.hpp>

using namespace mill;

TEST_CASE("fiber works", "[fiber]") {
    auto a = false;
    auto b = false;
    fiber fiber([&] {
        a = true;
        fiber::pause();
        b = true;
    });
    REQUIRE(!a);
    REQUIRE(!b);
    fiber.resume();
    REQUIRE(a);
    REQUIRE(!b);
    fiber.resume();
    REQUIRE(a);
    REQUIRE(b);
}
